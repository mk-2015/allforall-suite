INCLUDE := include
SRC     := src
LIB     := lib
OBJ     := obj

LIBNAME := libm-e
TARGET  := $(LIB)/$(LIBNAME)

STATIC_WINDOWS_LIB := $(TARGET).a
STATIC_LINUX_LIB := $(TARGET)-linux.a
DYNAMIC_WINDOWS_LIB := $(TARGET).dll
DYNAMIC_LINUX_LIB := $(TARGET)-linux.so

CC      := gcc
AR	    := ar
CFLAGS  := -I$(INCLUDE) -fPIC -Wall -Wextra -O3 -std=c11

OBJ_STATIC  := $(OBJ)/static
OBJ_DYNAMIC := $(OBJ)/dynamic

ifeq ($(OS),Windows_NT)
	TARGET_DYLIB  := $(DYNAMIC_WINDOWS_LIB)
	TARGET_STATIC := $(STATIC_WINDOWS_LIB)
	LDFLAGS       := -shared
	CLEAN_CMD     := rm -rf $(OBJ) $(LIB)
	MKDIR_CMD     := mkdir -p $(INCLUDE) $(SRC) $(LIB) $(OBJ) $(OBJ_STATIC) $(OBJ_DYNAMIC)
else
	TARGET_DYLIB  := $(DYNAMIC_LINUX_LIB)
	TARGET_STATIC := $(STATIC_LINUX_LIB)
	LDFLAGS       := -shared
	CLEAN_CMD     := rm -rf $(OBJ) $(LIB)
	MKDIR_CMD     := mkdir -p $(INCLUDE) $(SRC) $(LIB) $(OBJ) $(OBJ_STATIC) $(OBJ_DYNAMIC)
endif

SRCS := $(shell find $(SRC) -name "*.c")

OBJS_STATIC  := $(patsubst $(SRC)/%,$(OBJ_STATIC)/%,$(SRCS:.c=.o))
OBJS_DYNAMIC := $(patsubst $(SRC)/%,$(OBJ_DYNAMIC)/%,$(SRCS:.c=.o))

.PHONY: all clean directories static dynamic deps test

deps:
	# None

all: directories static dynamic deps

test: static
	$(CC) $(CFLAGS) test.c -L$(LIB) -l:$(notdir $(TARGET_STATIC)) -o test_runner -lpthread
	./test_runner

directories:
	@$(MKDIR_CMD)

$(OBJ_STATIC)/%.o: $(SRC)/%.c
	@mkdir -p $(dir $@)
	@$(CC) $(CFLAGS) -c $< -o $@
	@echo " [CC] $<"

$(OBJ_DYNAMIC)/%.o: $(SRC)/%.c
	@mkdir -p $(dir $@)
	@$(CC) $(CFLAGS) -c $< -o $@ -DBUILDING_DLL
	@echo " [CC] $<"

dynamic: $(TARGET_DYLIB)

$(TARGET_DYLIB): $(OBJS_DYNAMIC)
	@$(CC) $(LDFLAGS) -o $@ $^
	@echo " [LD] $@"

static: $(TARGET_STATIC)

$(TARGET_STATIC): $(OBJS_STATIC)
	@$(AR) rcs $@ $^
	@echo " [AR] $@"

clean:
	@$(CLEAN_CMD)
