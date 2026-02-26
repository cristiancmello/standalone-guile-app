# ============================================================================
# CONFIGURAÇÃO GERAL
# ============================================================================

SHELL := /bin/bash
CC := gcc
CFLAGS := -Wall -Wextra -O2 -g $(shell pkg-config --cflags guile-3.0 2>/dev/null || echo "-I/usr/include/guile/3.0")
LDFLAGS := $(shell pkg-config --libs guile-3.0 2>/dev/null || echo "-lguile-3.0") -ldl -lpthread

# Paths do Guile-GI
GUILE_GI_ROOT := $(CURDIR)/guile-gi
GUILE_GI_BUILD := $(GUILE_GI_ROOT)/build
GUILE_GI_MODULE_SRC := $(GUILE_GI_ROOT)/module
GUILE_GI_MODULE_BUILD := $(GUILE_GI_BUILD)/module
GUILE_GI_MODULE_GI := $(GUILE_GI_MODULE_BUILD)/gi
GUILE_GI_LIBS := $(GUILE_GI_BUILD)/.libs

# Variáveis de ambiente para o Guile
export GUILE_LOAD_PATH := $(GUILE_GI_MODULE_SRC):$(GUILE_LOAD_PATH)
export GUILE_LOAD_COMPILED_PATH := $(GUILE_GI_MODULE_BUILD):$(GUILE_LOAD_COMPILED_PATH)
export LD_LIBRARY_PATH := $(GUILE_GI_LIBS):$(LD_LIBRARY_PATH)
export GI_TYPELIB_PATH := /usr/lib/x86_64-linux-gnu/girepository-1.0:$(GI_TYPELIB_PATH)

# Ferramentas
GUILD := guild-3.0
GUILD_COMPILE := $(GUILD) compile
OBJCOPY := objcopy

# ============================================================================
# ARQUIVOS EMBUTIDOS
# ============================================================================

HELLO_GO := hello.go
HELLO_GO_OBJ := hello.go.o

# Módulos no subdiretório gi/
GI_SUBMODULES := compat config core-generics girepository logging oop types util
GI_SUBMODULE_OBJS := $(foreach mod,$(GI_SUBMODULES),gi_$(mod).go.o)

# Módulos na raiz de module/
GI_ROOT_MODULES := gi repository
GI_ROOT_MODULE_OBJS := $(foreach mod,$(GI_ROOT_MODULES),$(mod).go.o)

GI_GO_OBJS := $(GI_ROOT_MODULE_OBJS) $(GI_SUBMODULE_OBJS)

GI_SO_FILES := libguile-gi.so libguile-girepository.so
GI_SO_OBJS := $(foreach lib,$(GI_SO_FILES),$(lib).o)

TYPELIBS_TAR := typelibs.tar
TYPELIBS_TAR_OBJ := typelibs.tar.o

EMBEDDED_OBJS := $(HELLO_GO_OBJ) $(GI_GO_OBJS) $(GI_SO_OBJS) $(TYPELIBS_TAR_OBJ)

# ============================================================================
# ALVOS PRINCIPAIS
# ============================================================================

.PHONY: all clean distclean hello run debug info

all: hello

hello: main.c $(EMBEDDED_OBJS)
	@echo "=== Linkando hello binário ==="
	$(CC) $(CFLAGS) -o $@ main.c $(EMBEDDED_OBJS) $(LDFLAGS)
	@echo "✓ Binário 'hello' gerado com sucesso!"

run: hello
	./hello

debug: hello
	./hello 2>&1 | grep "⏱️"

# ============================================================================
# COMPILAÇÃO GUILLE-GI
# ============================================================================

.PHONY: build-gi

build-gi: $(GUILE_GI_BUILD)/libguile-gi.so

$(GUILE_GI_ROOT)/.cloned:
	@echo "=== Clonando guile-gi ==="
	@if [ -d "$(GUILE_GI_ROOT)" ]; then \
		echo "Diretório guile-gi já existe, atualizando..."; \
		cd $(GUILE_GI_ROOT) && git pull --rebase; \
	else \
		git clone https://github.com/spk121/guile-gi.git $(GUILE_GI_ROOT); \
	fi
	touch $@

$(GUILE_GI_BUILD)/libguile-gi.so: $(GUILE_GI_ROOT)/.cloned
	@echo "=== Preparando build do guile-gi ==="
	
	mkdir -p $(GUILE_GI_BUILD)
	mkdir -p $(GUILE_GI_BUILD)/test
	
	cd $(GUILE_GI_ROOT) && libtoolize --force --copy 2>/dev/null || true
	cd $(GUILE_GI_ROOT) && aclocal -I m4 --install 2>/dev/null || true
	cd $(GUILE_GI_ROOT) && autoreconf -fi 2>/dev/null || true
	
	@echo "=== Configurando guile-gi ==="
	cd $(GUILE_GI_BUILD) && \
		env MAKEINFO=true \
		../configure \
			--prefix=$(CURDIR)/guile-gi-install \
			--disable-dependency-tracking \
			--quiet || (echo "Configure falhou, veja logs acima" && exit 1)
	
	@echo "=== Compilando guile-gi ==="
	$(MAKE) -C $(GUILE_GI_BUILD) MAKEINFO=true || (echo "Build falhou, veja logs acima" && exit 1)
	
	@echo "✓ guile-gi compilado com sucesso!"

# ============================================================================
# COMPILAÇÃO DO CÓDIGO SCHEME
# ============================================================================

$(HELLO_GO): hello.scm | build-gi
	@echo "=== Compilando hello.go ==="
	$(GUILD_COMPILE) -o $@ $<

# ============================================================================
# EMBEDDING VIA OBJCOPY
# ============================================================================

# hello.go.o (arquivo local)
hello.go.o: hello.go
	@echo "=== Embedding hello.go ==="
	$(OBJCOPY) --input-target=binary \
	           --output-target=elf64-x86-64 \
	           --rename-section .data=.rodata,alloc,load,readonly,data,contents \
	           $< $@

# gi.go (raiz de module/)
gi.go.o: $(GUILE_GI_MODULE_BUILD)/gi.go | build-gi
	@echo "=== Embedding gi.go ==="
	cp $(GUILE_GI_MODULE_BUILD)/gi.go gi.go
	$(OBJCOPY) --input-target=binary --output-target=elf64-x86-64 \
	           --rename-section .data=.rodata,alloc,load,readonly,data,contents \
	           gi.go $@
	rm -f gi.go

# repository.go (gi/repository.go)
repository.go.o: $(GUILE_GI_MODULE_GI)/repository.go | build-gi
	@echo "=== Embedding repository.go ==="
	cp $(GUILE_GI_MODULE_GI)/repository.go repository.go
	$(OBJCOPY) --input-target=binary --output-target=elf64-x86-64 \
	           --rename-section .data=.rodata,alloc,load,readonly,data,contents \
	           repository.go $@
	rm -f repository.go

# Módulos no subdiretório gi/
gi_compat.go.o: $(GUILE_GI_MODULE_GI)/compat.go | build-gi
	@echo "=== Embedding gi/compat.go ==="
	cp $(GUILE_GI_MODULE_GI)/compat.go gi_compat.go
	$(OBJCOPY) --input-target=binary --output-target=elf64-x86-64 \
	           --rename-section .data=.rodata,alloc,load,readonly,data,contents \
	           gi_compat.go $@
	rm -f gi_compat.go

gi_config.go.o: $(GUILE_GI_MODULE_GI)/config.go | build-gi
	cp $(GUILE_GI_MODULE_GI)/config.go gi_config.go
	$(OBJCOPY) --input-target=binary --output-target=elf64-x86-64 \
	           --rename-section .data=.rodata,alloc,load,readonly,data,contents \
	           gi_config.go $@
	rm -f gi_config.go

gi_core-generics.go.o: $(GUILE_GI_MODULE_GI)/core-generics.go | build-gi
	cp $(GUILE_GI_MODULE_GI)/core-generics.go gi_core-generics.go
	$(OBJCOPY) --input-target=binary --output-target=elf64-x86-64 \
	           --rename-section .data=.rodata,alloc,load,readonly,data,contents \
	           gi_core-generics.go $@
	rm -f gi_core-generics.go

gi_girepository.go.o: $(GUILE_GI_MODULE_GI)/girepository.go | build-gi
	cp $(GUILE_GI_MODULE_GI)/girepository.go gi_girepository.go
	$(OBJCOPY) --input-target=binary --output-target=elf64-x86-64 \
	           --rename-section .data=.rodata,alloc,load,readonly,data,contents \
	           gi_girepository.go $@
	rm -f gi_girepository.go

gi_logging.go.o: $(GUILE_GI_MODULE_GI)/logging.go | build-gi
	cp $(GUILE_GI_MODULE_GI)/logging.go gi_logging.go
	$(OBJCOPY) --input-target=binary --output-target=elf64-x86-64 \
	           --rename-section .data=.rodata,alloc,load,readonly,data,contents \
	           gi_logging.go $@
	rm -f gi_logging.go

gi_oop.go.o: $(GUILE_GI_MODULE_GI)/oop.go | build-gi
	cp $(GUILE_GI_MODULE_GI)/oop.go gi_oop.go
	$(OBJCOPY) --input-target=binary --output-target=elf64-x86-64 \
	           --rename-section .data=.rodata,alloc,load,readonly,data,contents \
	           gi_oop.go $@
	rm -f gi_oop.go

gi_types.go.o: $(GUILE_GI_MODULE_GI)/types.go | build-gi
	cp $(GUILE_GI_MODULE_GI)/types.go gi_types.go
	$(OBJCOPY) --input-target=binary --output-target=elf64-x86-64 \
	           --rename-section .data=.rodata,alloc,load,readonly,data,contents \
	           gi_types.go $@
	rm -f gi_types.go

gi_util.go.o: $(GUILE_GI_MODULE_GI)/util.go | build-gi
	cp $(GUILE_GI_MODULE_GI)/util.go gi_util.go
	$(OBJCOPY) --input-target=binary --output-target=elf64-x86-64 \
	           --rename-section .data=.rodata,alloc,load,readonly,data,contents \
	           gi_util.go $@
	rm -f gi_util.go

# ✅ Bibliotecas .so (REGRAS EXPLÍCITAS)
libguile-gi.so.o: $(GUILE_GI_LIBS)/libguile-gi.so | build-gi
	@echo "=== Embedding libguile-gi.so ==="
	cp $(GUILE_GI_LIBS)/libguile-gi.so libguile-gi.so
	$(OBJCOPY) --input-target=binary --output-target=elf64-x86-64 \
	           --rename-section .data=.rodata,alloc,load,readonly,data,contents \
	           libguile-gi.so $@
	rm -f libguile-gi.so

libguile-girepository.so.o: $(GUILE_GI_LIBS)/libguile-girepository.so | build-gi
	@echo "=== Embedding libguile-girepository.so ==="
	cp $(GUILE_GI_LIBS)/libguile-girepository.so libguile-girepository.so
	$(OBJCOPY) --input-target=binary --output-target=elf64-x86-64 \
	           --rename-section .data=.rodata,alloc,load,readonly,data,contents \
	           libguile-girepository.so $@
	rm -f libguile-girepository.so

# typelibs.tar.o
$(TYPELIBS_TAR_OBJ): $(TYPELIBS_TAR)
	@echo "=== Embedding $< ==="
	$(OBJCOPY) --input-target=binary \
	           --output-target=elf64-x86-64 \
	           --rename-section .data=.rodata,alloc,load,readonly,data,contents \
	           $< $@

# ============================================================================
# GERAR TYPELIBS.TAR (SEM PREFIXO ./)
# ============================================================================

$(TYPELIBS_TAR): | build-gi
	@echo "=== Coletando typelibs do sistema ==="
	@rm -rf typelib-tmp
	@mkdir -p typelib-tmp
	@# GTK4 e dependências diretas
	cp /usr/lib/x86_64-linux-gnu/girepository-1.0/Gtk-4.0.typelib typelib-tmp/ 2>/dev/null || echo "⚠️ Gtk-4.0 não encontrado"
	cp /usr/lib/x86_64-linux-gnu/girepository-1.0/Gdk-4.0.typelib typelib-tmp/ 2>/dev/null || true
	cp /usr/lib/x86_64-linux-gnu/girepository-1.0/Gsk-4.0.typelib typelib-tmp/ 2>/dev/null || true
	cp /usr/lib/x86_64-linux-gnu/girepository-1.0/Gio-2.0.typelib typelib-tmp/ 2>/dev/null || true
	cp /usr/lib/x86_64-linux-gnu/girepository-1.0/GObject-2.0.typelib typelib-tmp/ 2>/dev/null || true
	cp /usr/lib/x86_64-linux-gnu/girepository-1.0/GLib-2.0.typelib typelib-tmp/ 2>/dev/null || true
	cp /usr/lib/x86_64-linux-gnu/girepository-1.0/GdkPixbuf-2.0.typelib typelib-tmp/ 2>/dev/null || true
	cp /usr/lib/x86_64-linux-gnu/girepository-1.0/Pango-1.0.typelib typelib-tmp/ 2>/dev/null || true
	cp /usr/lib/x86_64-linux-gnu/girepository-1.0/HarfBuzz-0.0.typelib typelib-tmp/ 2>/dev/null || true
	@# ✅ NOVO: Graphene (dependência do GTK4)
	cp /usr/lib/x86_64-linux-gnu/girepository-1.0/Graphene-1.0.typelib typelib-tmp/ 2>/dev/null || echo "⚠️ Graphene-1.0 não encontrado"
	@# Criar tar SEM prefixo ./
	cd typelib-tmp && tar -cf ../$@ *
	@rm -rf typelib-tmp
	@echo "✓ typelibs.tar gerado com $(tar -tf $@ | wc -l) arquivos"

# ============================================================================
# LIMPEZA
# ============================================================================

clean:
	@echo "=== Limpando arquivos intermediários ==="
	rm -f hello hello.go $(EMBEDDED_OBJS)
	rm -f gi.go repository.go gi_*.go
	rm -f libguile-gi.so libguile-girepository.so
	rm -f $(TYPELIBS_TAR)
	rm -rf typelib-tmp
	rm -rf guile-gi-install

distclean: clean
	@echo "=== Limpando build do guile-gi ==="
	rm -rf $(GUILE_GI_BUILD)
	rm -f $(GUILE_GI_ROOT)/.cloned
	rm -rf $(GUILE_GI_ROOT)/autom4te.cache
	rm -f $(GUILE_GI_ROOT)/configure
	rm -f $(GUILE_GI_ROOT)/aclocal.m4
	rm -f $(GUILE_GI_ROOT)/Makefile.in

# ============================================================================
# DEBUG E INFORMAÇÃO
# ============================================================================

info:
	@echo "=== Estrutura do guile-gi ==="
	@echo "Módulos na raiz:"
	@ls $(GUILE_GI_MODULE_BUILD)/*.go 2>/dev/null || echo "  (nenhum)"
	@echo "Módulos em gi/:"
	@ls $(GUILE_GI_MODULE_GI)/*.go 2>/dev/null || echo "  (nenhum)"
	@echo ""
	@echo "=== Objetos para embed ==="
	@echo "EMBEDDED_OBJS: $(EMBEDDED_OBJS)"

check-symbols: hello
	@echo "=== Verificando símbolos _binary_* ==="
	nm hello | grep "_binary_" | head -30

test-load: build-gi
	@echo "=== Testando carregamento do módulo (gi) ==="
	GUILE_LOAD_PATH=$(GUILE_LOAD_PATH) \
	GUILE_LOAD_COMPILED_PATH=$(GUILE_LOAD_COMPILED_PATH) \
	LD_LIBRARY_PATH=$(LD_LIBRARY_PATH) \
	guile -c '(use-modules (gi repository)) (require "Gtk" "4.0") (display "✓ (gi) carregado!\n")'