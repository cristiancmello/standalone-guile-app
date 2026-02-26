# ====================================================================
# Makefile para projeto Guile com suporte automático a guile-gi
# ====================================================================

# Configurações do projeto
TARGET   = hello
SCM      = hello.scm
GO       = hello.go
MAIN     = main.c

# Configurações do guile-gi
GI_REPO  = https://github.com/spk121/guile-gi.git
GI_DIR   = guile-gi
GI_BUILD_DIR = $(GI_DIR)/build
GI_INSTALL_PREFIX = $(shell pwd)/gi-install

# Diretórios locais para instalação
GI_LIB_DIR = $(GI_INSTALL_PREFIX)/lib
GI_INCLUDE_DIR = $(GI_INSTALL_PREFIX)/include
GI_GO_DIR = $(GI_INSTALL_PREFIX)/lib/guile/3.0/site-ccache

# Diretórios de origem
GI_SO_DIR = $(GI_BUILD_DIR)/.libs
GI_GO_SRC_DIR = $(GI_BUILD_DIR)/module

# Diretório de typelibs
TYPELIB_DIR = /usr/lib/x86_64-linux-gnu/girepository-1.0

# Lista de objetos
OBJS = hello.o \
       typelibs.o \
       gi.o \
       gi_repository.o \
       gi_compat.o \
       gi_config.o \
       gi_core_generics.o \
       gi_logging.o \
       gi_oop.o \
       gi_types.o \
       gi_util.o \
       gi_girepository.o \
       libguile_gi.o \
       libguile_girepository.o

# Ferramentas
CC       = gcc
CFLAGS   = -O2 -Wall
GUILE_FLAGS = $(shell pkg-config --cflags --libs guile-3.0)
GUILD    = guild

OBJCOPY_FLAGS = --input-target binary --output-target elf64-x86-64 \
                --binary-architecture i386:x86-64 \
                --add-section .note.GNU-stack=/dev/null

# ====================================================================
# Regras principais
# ====================================================================

.PHONY: all clean distclean clone-gi build-gi install-gi symbols

all: clone-gi build-gi install-gi $(TARGET)

# Clona o repositório
clone-gi:
	@if [ ! -d "$(GI_DIR)" ]; then \
		echo "Clonando guile-gi..."; \
		git clone --depth 1 $(GI_REPO) $(GI_DIR); \
	else \
		echo "guile-gi já existe."; \
	fi

# Compila o guile-gi
build-gi: clone-gi
	@echo "Compilando guile-gi..."
	cd $(GI_DIR) && if [ ! -f configure ]; then autoreconf -i -f; fi
	mkdir -p $(GI_BUILD_DIR)
	cd $(GI_BUILD_DIR) && ../configure --prefix=/usr
	$(MAKE) -C $(GI_BUILD_DIR)
	@echo "✅ Build concluído."

# Instala o guile-gi localmente
install-gi: build-gi
	@echo "Instalando guile-gi..."
	mkdir -p $(GI_LIB_DIR) $(GI_INCLUDE_DIR) $(GI_GO_DIR)/gi
	cp -vf $(GI_SO_DIR)/libguile-gi.so* $(GI_LIB_DIR)/ 2>/dev/null || true
	cp -vf $(GI_SO_DIR)/libguile-girepository.so* $(GI_LIB_DIR)/ 2>/dev/null || true
	cp -vf $(GI_DIR)/src/*.h $(GI_INCLUDE_DIR)/ 2>/dev/null || true
	cp -vf $(GI_GO_SRC_DIR)/gi.go $(GI_GO_DIR)/ 2>/dev/null || true
	cp -vf $(GI_GO_SRC_DIR)/gi/*.go $(GI_GO_DIR)/gi/ 2>/dev/null || true
	@echo "✅ Instalação concluída."

# ====================================================================
# Regras para os objetos
# ====================================================================

# hello.o
hello.o: $(GO)
	objcopy $(OBJCOPY_FLAGS) $< $@

$(GO): $(SCM)
	$(GUILD) compile -o $@ $<

# Regras para arquivos .go na raiz do GI_GO_DIR
gi.o: $(GI_GO_DIR)/gi.go
	cp $< gi.tmp
	objcopy $(OBJCOPY_FLAGS) gi.tmp $@
	rm -f gi.tmp

# Regras para arquivos .go no subdiretório gi/
gi_repository.o: $(GI_GO_DIR)/gi/repository.go
	cp $< repository.tmp
	objcopy $(OBJCOPY_FLAGS) repository.tmp $@
	rm -f repository.tmp

gi_compat.o: $(GI_GO_DIR)/gi/compat.go
	cp $< compat.tmp
	objcopy $(OBJCOPY_FLAGS) compat.tmp $@
	rm -f compat.tmp

gi_config.o: $(GI_GO_DIR)/gi/config.go
	cp $< config.tmp
	objcopy $(OBJCOPY_FLAGS) config.tmp $@
	rm -f config.tmp

gi_core_generics.o: $(GI_GO_DIR)/gi/core-generics.go
	cp $< core_generics.tmp
	objcopy $(OBJCOPY_FLAGS) core_generics.tmp $@
	rm -f core_generics.tmp

gi_logging.o: $(GI_GO_DIR)/gi/logging.go
	cp $< logging.tmp
	objcopy $(OBJCOPY_FLAGS) logging.tmp $@
	rm -f logging.tmp

gi_oop.o: $(GI_GO_DIR)/gi/oop.go
	cp $< oop.tmp
	objcopy $(OBJCOPY_FLAGS) oop.tmp $@
	rm -f oop.tmp

gi_types.o: $(GI_GO_DIR)/gi/types.go
	cp $< types.tmp
	objcopy $(OBJCOPY_FLAGS) types.tmp $@
	rm -f types.tmp

gi_util.o: $(GI_GO_DIR)/gi/util.go
	cp $< util.tmp
	objcopy $(OBJCOPY_FLAGS) util.tmp $@
	rm -f util.tmp

gi_girepository.o: $(GI_GO_DIR)/gi/girepository.go
	cp $< girepository.tmp
	objcopy $(OBJCOPY_FLAGS) girepository.tmp $@
	rm -f girepository.tmp

# Regras para bibliotecas .so
libguile_gi.o: $(GI_LIB_DIR)/libguile-gi.so
	cp $< libguile-gi.tmp
	objcopy $(OBJCOPY_FLAGS) libguile-gi.tmp $@
	rm -f libguile-gi.tmp

libguile_girepository.o: $(GI_LIB_DIR)/libguile-girepository.so
	cp $< libguile-girepository.tmp
	objcopy $(OBJCOPY_FLAGS) libguile-girepository.tmp $@
	rm -f libguile-girepository.tmp

# typelibs
typelibs.tar:
	tar -cf $@ -C $(TYPELIB_DIR) .

typelibs.o: typelibs.tar
	cp $< typelibs.tmp
	objcopy $(OBJCOPY_FLAGS) typelibs.tmp $@
	rm -f typelibs.tmp

# ====================================================================
# Link do executável
# ====================================================================

$(TARGET): $(MAIN) $(OBJS)
	$(CC) $(CFLAGS) -I$(GI_INCLUDE_DIR) $(MAIN) $(OBJS) -o $@ \
		-L$(GI_LIB_DIR) -lguile-gi \
		-Wl,-rpath,$(GI_LIB_DIR) \
		$(GUILE_FLAGS)
	@echo "✅ Executável criado: ./$@"

# ====================================================================
# Utilidades
# ====================================================================

symbols:
	@echo "=== Símbolos nos arquivos .o ==="
	@for obj in $(OBJS); do \
		if [ -f "$$obj" ]; then \
			echo "\n$$obj:"; \
			nm $$obj 2>/dev/null | grep -E '_binary.*_(start|end)' | sed 's/^/  /'; \
		fi \
	done

clean:
	rm -f $(GO) $(OBJS) $(TARGET) typelibs.tar *.tmp

distclean: clean
	rm -rf $(GI_DIR) $(GI_INSTALL_PREFIX)

debug:
	@echo "GI_LIB_DIR: $(GI_LIB_DIR)"
	@echo "GI_GO_DIR: $(GI_GO_DIR)"
	@ls -la $(GI_LIB_DIR)/*.so* 2>/dev/null || true
	@ls -la $(GI_GO_DIR)/*.go 2>/dev/null || true
	@ls -la $(GI_GO_DIR)/gi/*.go 2>/dev/null || true