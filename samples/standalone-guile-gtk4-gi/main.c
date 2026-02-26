#define _GNU_SOURCE
#include <libguile.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <dlfcn.h>
#include <unistd.h>
#include <time.h>
#include <errno.h>
#include <fcntl.h>
#include <dirent.h>

// ============================================================================
// UTILITÁRIOS DE TIMING (RELATIVO)
// ============================================================================

static double g_start_time_ms = 0;

static double get_time_ms(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (double)ts.tv_sec * 1000.0 + (double)ts.tv_nsec / 1000000.0;
}

static double get_elapsed_ms(void) {
    return get_time_ms() - g_start_time_ms;
}

#define TIME_POINT(msg) \
    fprintf(stderr, "[⏱️  %7.3f ms] %s\n", get_elapsed_ms(), msg)

// ============================================================================
// SÍMBOLOS EMBUTIDOS
// ============================================================================

extern char _binary_hello_go_start[];
extern char _binary_hello_go_end[];

extern char _binary_gi_go_start[];
extern char _binary_gi_go_end[];

extern char _binary_repository_go_start[];
extern char _binary_repository_go_end[];

extern char _binary_gi_compat_go_start[];
extern char _binary_gi_compat_go_end[];

extern char _binary_gi_config_go_start[];
extern char _binary_gi_config_go_end[];

extern char _binary_gi_core_generics_go_start[];
extern char _binary_gi_core_generics_go_end[];

extern char _binary_gi_girepository_go_start[];
extern char _binary_gi_girepository_go_end[];

extern char _binary_gi_logging_go_start[];
extern char _binary_gi_logging_go_end[];

extern char _binary_gi_oop_go_start[];
extern char _binary_gi_oop_go_end[];

extern char _binary_gi_types_go_start[];
extern char _binary_gi_types_go_end[];

extern char _binary_gi_util_go_start[];
extern char _binary_gi_util_go_end[];

extern char _binary_libguile_gi_so_start[];
extern char _binary_libguile_gi_so_end[];

extern char _binary_libguile_girepository_so_start[];
extern char _binary_libguile_girepository_so_end[];

extern char _binary_typelibs_tar_start[];
extern char _binary_typelibs_tar_end[];

static char tmpdir[512];

// ============================================================================
// FUNÇÕES AUXILIARES
// ============================================================================

static void write_file(const char *path, char *start, char *end) {
    FILE *f = fopen(path, "wb");
    if (!f) { 
        perror(path); 
        exit(1); 
    }
    fwrite(start, 1, end - start, f);
    fclose(f);
}

static void extract_files(void) {
    TIME_POINT(">>> extract_files");
    char path[512];
    
    snprintf(path, sizeof(path), "%s/gi.go", tmpdir);
    write_file(path, _binary_gi_go_start, _binary_gi_go_end);
    TIME_POINT("  gi.go extraído");
    
    snprintf(path, sizeof(path), "%s/gi", tmpdir);
    mkdir(path, 0700);
    
    #define EXTRACT_GI_MODULE(name, start_sym, end_sym) do { \
        snprintf(path, sizeof(path), "%s/gi/" #name ".go", tmpdir); \
        write_file(path, start_sym, end_sym); \
    } while(0)

    EXTRACT_GI_MODULE(repository, _binary_repository_go_start, _binary_repository_go_end);
    TIME_POINT("  repository.go extraído");
    
    EXTRACT_GI_MODULE(compat, _binary_gi_compat_go_start, _binary_gi_compat_go_end);
    EXTRACT_GI_MODULE(config, _binary_gi_config_go_start, _binary_gi_config_go_end);
    EXTRACT_GI_MODULE(core-generics, _binary_gi_core_generics_go_start, _binary_gi_core_generics_go_end);
    EXTRACT_GI_MODULE(girepository, _binary_gi_girepository_go_start, _binary_gi_girepository_go_end);
    EXTRACT_GI_MODULE(logging, _binary_gi_logging_go_start, _binary_gi_logging_go_end);
    EXTRACT_GI_MODULE(oop, _binary_gi_oop_go_start, _binary_gi_oop_go_end);
    EXTRACT_GI_MODULE(types, _binary_gi_types_go_start, _binary_gi_types_go_end);
    EXTRACT_GI_MODULE(util, _binary_gi_util_go_start, _binary_gi_util_go_end);
    TIME_POINT("  módulos gi/*.go extraídos");
    
    snprintf(path, sizeof(path), "%s/libguile-gi.so", tmpdir);
    write_file(path, _binary_libguile_gi_so_start, _binary_libguile_gi_so_end);
    
    snprintf(path, sizeof(path), "%s/libguile-girepository.so", tmpdir);
    write_file(path, _binary_libguile_girepository_so_start, _binary_libguile_girepository_so_end);
    TIME_POINT("  bibliotecas .so extraídas");
    
    snprintf(path, sizeof(path), "%s/typelib", tmpdir);
    mkdir(path, 0700);

    char tarpath[512];
    snprintf(tarpath, sizeof(tarpath), "%s/typelibs.tar", tmpdir);
    write_file(tarpath, _binary_typelibs_tar_start, _binary_typelibs_tar_end);

    snprintf(path, sizeof(tarpath), "tar -xf '%s' -C '%s/typelib' 2>/dev/null", tarpath, tmpdir);
    system(path);
    remove(tarpath);
    TIME_POINT("  typelibs extraídos");
    
    TIME_POINT("<<< extract_files completo");
}

// ============================================================================
// INNER MAIN: Carregamento do bytecode Guile
// ============================================================================

static void inner_main(void *data, int argc, char **argv) {
    (void)data; (void)argc; (void)argv;
    
    // === DEBUG: Verificar GI_TYPELIB_PATH ===
    const char *gi_path = getenv("GI_TYPELIB_PATH");
    fprintf(stderr, "[DEBUG] GI_TYPELIB_PATH = %s\n", gi_path ? gi_path : "(NULL)");
    
    if (gi_path) {
        char check_path[512];
        snprintf(check_path, sizeof(check_path), "%s/Gtk-4.0.typelib", gi_path);
        FILE *f = fopen(check_path, "r");
        if (f) {
            fprintf(stderr, "[DEBUG] ✓ Gtk-4.0.typelib ENCONTRADA\n");
            fclose(f);
        } else {
            fprintf(stderr, "[DEBUG] ❌ Gtk-4.0.typelib NÃO encontrada em: %s\n", check_path);
            
            DIR *dir = opendir(gi_path);
            if (dir) {
                fprintf(stderr, "[DEBUG] Conteúdo do diretório:\n");
                struct dirent *entry;
                while ((entry = readdir(dir)) != NULL) {
                    fprintf(stderr, "[DEBUG]   - %s\n", entry->d_name);
                }
                closedir(dir);
            }
        }
    }
    // === FIM DEBUG ===
    
    TIME_POINT(">>> inner_main: load bytecode");
    
    size_t len = _binary_hello_go_end - _binary_hello_go_start;
    fprintf(stderr, "[⏱️  %7.3f ms]   Tamanho do bytecode: %zu bytes\n", get_elapsed_ms(), len);
    
    TIME_POINT("  Criando bytevector");
    SCM bv = scm_c_make_bytevector(len);
    
    TIME_POINT("  Copiando dados para bytevector");
    memcpy(SCM_BYTEVECTOR_CONTENTS(bv), _binary_hello_go_start, len);

    TIME_POINT("  Obtendo referência para load-thunk-from-memory");
    SCM loader = scm_c_public_ref("system vm loader", "load-thunk-from-memory");
    
    TIME_POINT("  Criando thunk");
    SCM thunk = scm_call_1(loader, bv);
    
    TIME_POINT("  Executando thunk (carregando módulo hello)");
    scm_call_0(thunk);
    
    TIME_POINT("<<< inner_main: bytecode carregado");
}

// ============================================================================
// MAIN: Ponto de entrada
// ============================================================================

int main(int argc, char **argv) {
    g_start_time_ms = get_time_ms();
    
    fprintf(stderr, "\n[🚀 STARTUP] hello standalone app\n");
    fprintf(stderr, "[⏱️  %7.3f ms] PID: %d\n", get_elapsed_ms(), getpid());
    
    char *existing = getenv("HELLO_TMPDIR");
    
    if (existing) {
        TIME_POINT("Cache HIT - usando diretório existente");
        
        strncpy(tmpdir, existing, sizeof(tmpdir) - 1);
        tmpdir[sizeof(tmpdir) - 1] = '\0';
        
        char tlpath[512];
        snprintf(tlpath, sizeof(tlpath), "%s/typelib", tmpdir);
        setenv("GI_TYPELIB_PATH", tlpath, 1);
        
        TIME_POINT("Variáveis de ambiente configuradas");
        
        scm_boot_guile(argc, argv, inner_main, NULL);
        
        fprintf(stderr, "[⏱️  %7.3f ms] scm_boot_guile completo\n", get_elapsed_ms());
        
    } else {
        TIME_POINT("Cache MISS - extração necessária");
        
        snprintf(tmpdir, sizeof(tmpdir), "/tmp/hello-XXXXXX");
        if (mkdtemp(tmpdir) == NULL) {
            perror("mkdtemp");
            exit(1);
        }
        fprintf(stderr, "[⏱️  %7.3f ms] Diretório temporário: %s\n", get_elapsed_ms(), tmpdir);
        
        extract_files();
        
        char tlpath[512];
        snprintf(tlpath, sizeof(tlpath), "%s/typelib", tmpdir);

        setenv("HELLO_TMPDIR", tmpdir, 1);
        setenv("GUILE_LOAD_COMPILED_PATH", tmpdir, 1);
        setenv("GUILE_LOAD_PATH", tmpdir, 1);
        setenv("LD_LIBRARY_PATH", tmpdir, 1);
        setenv("GI_TYPELIB_PATH", tlpath, 1);
        
        TIME_POINT("Variáveis de ambiente definidas, executando execv...");
        
        execv("/proc/self/exe", argv);
        
        fprintf(stderr, "[❌ ERRO] execv falhou: %s\n", strerror(errno));
        exit(1);
    }

    fprintf(stderr, "[🏁 DONE] Tempo total: %.3f ms\n\n", get_elapsed_ms());
    
    return 0;
}