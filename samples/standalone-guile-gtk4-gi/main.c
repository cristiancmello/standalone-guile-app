#include <libguile.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <dlfcn.h>
#include <unistd.h>

// Símbolos baseados na saída do make
extern char _binary_hello_go_start[];
extern char _binary_hello_go_end[];

extern char _binary_gi_tmp_start[];
extern char _binary_gi_tmp_end[];
extern char _binary_repository_tmp_start[];
extern char _binary_repository_tmp_end[];
extern char _binary_compat_tmp_start[];
extern char _binary_compat_tmp_end[];
extern char _binary_config_tmp_start[];
extern char _binary_config_tmp_end[];
extern char _binary_core_generics_tmp_start[];
extern char _binary_core_generics_tmp_end[];
extern char _binary_logging_tmp_start[];
extern char _binary_logging_tmp_end[];
extern char _binary_oop_tmp_start[];
extern char _binary_oop_tmp_end[];
extern char _binary_types_tmp_start[];
extern char _binary_types_tmp_end[];
extern char _binary_util_tmp_start[];
extern char _binary_util_tmp_end[];
extern char _binary_girepository_tmp_start[];
extern char _binary_girepository_tmp_end[];

extern char _binary_libguile_gi_tmp_start[];
extern char _binary_libguile_gi_tmp_end[];
extern char _binary_libguile_girepository_tmp_start[];
extern char _binary_libguile_girepository_tmp_end[];

extern char _binary_typelibs_tmp_start[];
extern char _binary_typelibs_tmp_end[];

static char tmpdir[256];

static void write_file(const char *path, char *start, char *end) {
    FILE *f = fopen(path, "wb");
    if (!f) { perror(path); exit(1); }
    fwrite(start, 1, end - start, f);
    fclose(f);
}

static void extract_files(void) {
    char path[512];

    // módulos guile-gi
    snprintf(path, sizeof(path), "%s/gi", tmpdir);
    mkdir(path, 0700);

    snprintf(path, sizeof(path), "%s/gi.go", tmpdir);
    write_file(path, _binary_gi_tmp_start, _binary_gi_tmp_end);
    
    snprintf(path, sizeof(path), "%s/gi/repository.go", tmpdir);
    write_file(path, _binary_repository_tmp_start, _binary_repository_tmp_end);
    
    snprintf(path, sizeof(path), "%s/gi/compat.go", tmpdir);
    write_file(path, _binary_compat_tmp_start, _binary_compat_tmp_end);
    
    snprintf(path, sizeof(path), "%s/gi/config.go", tmpdir);
    write_file(path, _binary_config_tmp_start, _binary_config_tmp_end);
    
    snprintf(path, sizeof(path), "%s/gi/core-generics.go", tmpdir);
    write_file(path, _binary_core_generics_tmp_start, _binary_core_generics_tmp_end);
    
    snprintf(path, sizeof(path), "%s/gi/logging.go", tmpdir);
    write_file(path, _binary_logging_tmp_start, _binary_logging_tmp_end);
    
    snprintf(path, sizeof(path), "%s/gi/oop.go", tmpdir);
    write_file(path, _binary_oop_tmp_start, _binary_oop_tmp_end);
    
    snprintf(path, sizeof(path), "%s/gi/types.go", tmpdir);
    write_file(path, _binary_types_tmp_start, _binary_types_tmp_end);
    
    snprintf(path, sizeof(path), "%s/gi/util.go", tmpdir);
    write_file(path, _binary_util_tmp_start, _binary_util_tmp_end);
    
    snprintf(path, sizeof(path), "%s/gi/girepository.go", tmpdir);
    write_file(path, _binary_girepository_tmp_start, _binary_girepository_tmp_end);

    // .so
    snprintf(path, sizeof(path), "%s/libguile-gi.so", tmpdir);
    write_file(path, _binary_libguile_gi_tmp_start, _binary_libguile_gi_tmp_end);
    
    snprintf(path, sizeof(path), "%s/libguile-girepository.so", tmpdir);
    write_file(path, _binary_libguile_girepository_tmp_start, _binary_libguile_girepository_tmp_end);

    // typelibs
    snprintf(path, sizeof(path), "%s/typelib", tmpdir);
    mkdir(path, 0700);

    char tarpath[512];
    snprintf(tarpath, sizeof(tarpath), "%s/typelibs.tar", tmpdir);
    write_file(tarpath, _binary_typelibs_tmp_start, _binary_typelibs_tmp_end);

    char cmd[512];
    snprintf(cmd, sizeof(cmd), "tar -xf %s -C %s/typelib", tarpath, tmpdir);
    system(cmd);
    remove(tarpath);
}

static void inner_main(void *data, int argc, char **argv) {
    size_t len = _binary_hello_go_end - _binary_hello_go_start;
    SCM bv = scm_c_make_bytevector(len);
    memcpy(SCM_BYTEVECTOR_CONTENTS(bv), _binary_hello_go_start, len);

    SCM thunk = scm_call_1(
        scm_c_public_ref("system vm loader", "load-thunk-from-memory"),
        bv
    );
    scm_call_0(thunk);
}

int main(int argc, char **argv) {
    char *existing = getenv("HELLO_TMPDIR");
    if (existing) {
        strncpy(tmpdir, existing, sizeof(tmpdir) - 1);
        tmpdir[sizeof(tmpdir) - 1] = '\0';
        char tlpath[512];
        snprintf(tlpath, sizeof(tlpath), "%s/typelib", tmpdir);
        setenv("GI_TYPELIB_PATH", tlpath, 1);
    } else {
        snprintf(tmpdir, sizeof(tmpdir), "/tmp/hello-XXXXXX");
        mkdtemp(tmpdir);
        extract_files();

        char tlpath[512];
        snprintf(tlpath, sizeof(tlpath), "%s/typelib", tmpdir);

        setenv("HELLO_TMPDIR", tmpdir, 1);
        setenv("GUILE_LOAD_COMPILED_PATH", tmpdir, 1);
        setenv("GUILE_LOAD_PATH", tmpdir, 1);
        setenv("LD_LIBRARY_PATH", tmpdir, 1);
        setenv("GI_TYPELIB_PATH", tlpath, 1);

        execv("/proc/self/exe", argv);
        perror("execv");
        exit(1);
    }

    scm_boot_guile(argc, argv, inner_main, NULL);
    return 0;
}