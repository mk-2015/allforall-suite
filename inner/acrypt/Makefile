CC       = gcc
CFLAGS   = -Wall -Wextra -Iinclude -O3 -fPIC

SRC      = src
OBJ      = obj
LIBDIR   = lib

SOURCES  := $(shell find $(SRC) -name '*.c')
OBJECTS  := $(patsubst $(SRC)/%.c, $(OBJ)/%.o, $(SOURCES))

LIB_STATIC = $(LIBDIR)/libacrypt.a

ifeq ($(OS),Windows_NT)
    LIB_SHARED   = $(LIBDIR)/libacrypt.dll
    LIB_IMPLIB   = $(LIBDIR)/libacrypt_impl.a
    SHARED_FLAGS = -shared -Wl,--out-implib,$(LIB_IMPLIB)
else
    LIB_SHARED   = $(LIBDIR)/libacrypt.so
    SHARED_FLAGS = -shared
endif

all: deps $(LIB_STATIC) $(LIB_SHARED)

deps:
	@mkdir -p $(LIBDIR)
	@mkdir -p $(OBJ)

$(OBJ)/%.o: $(SRC)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

$(LIB_STATIC): $(OBJECTS)
	@mkdir -p $(LIBDIR)
	ar rcs $@ $^
	@echo "===> Built static library: $@"

$(LIB_SHARED): $(OBJECTS)
	@mkdir -p $(LIBDIR)
	$(CC) $(SHARED_FLAGS) -o $@ $^
	@echo "===> Built shared library / DLL: $@"

clean:
	rm -rf $(OBJ) $(LIBDIR)
	@echo "===> Cleaned build artifacts."

.PHONY: all deps clean