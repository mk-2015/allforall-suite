INCLUDE := include
SRC     := src
LIB     := lib
OBJ     := obj

LIBNAME := libgpop
TARGET  := $(LIB)/$(LIBNAME)

STATIC_WINDOWS_LIB := $(TARGET).a
STATIC_LINUX_LIB := $(TARGET)-linux.a
DYNAMIC_WINDOWS_LIB := $(TARGET).dll
DYNAMIC_LINUX_LIB := $(TARGET)-linux.so

CC      := g++
AR	    := ar
CFLAGS  := -I$(INCLUDE) -fPIC -Wall -Wextra -O3 -std=c++17

OBJ_STATIC  := $(OBJ)/static
OBJ_DYNAMIC := $(OBJ)/dynamic

ENABLE_ARCHNATIV ?= 0

ifeq ($(ENABLE_ARCHNATIV), 1)
    CFLAGS += -march=native -flto
endif

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

SRCS := $(shell find $(SRC) -name "*.cpp")

OBJS_STATIC  := $(patsubst $(SRC)/%.cpp,$(OBJ_STATIC)/%.o,$(SRCS))
OBJS_DYNAMIC := $(patsubst $(SRC)/%.cpp,$(OBJ_DYNAMIC)/%.o,$(SRCS))

.PHONY: all clean directories static dynamic deps

deps:
	# None

all: directories static dynamic

directories:
	@$(MKDIR_CMD)

$(OBJ_STATIC)/%.o: $(SRC)/%.cpp
	@mkdir -p $(dir $@)
	@$(CC) $(CFLAGS) -c $< -o $@
	@echo " [CC] $<"

$(OBJ_DYNAMIC)/%.o: $(SRC)/%.cpp
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
