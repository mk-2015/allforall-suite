INCLUDE := include
SRC     := src
LIB     := lib
OBJ     := obj

LIBNAME := libutil
TARGET  := $(LIB)/$(LIBNAME)

STATIC_WINDOWS_LIB := $(TARGET).a
STATIC_LINUX_LIB := $(TARGET).a
DYNAMIC_WINDOWS_LIB := $(TARGET).dll
DYNAMIC_LINUX_LIB := $(TARGET).so

CXX      := gcc
CXXFLAGS := -I$(INCLUDE) -fPIC -Wall -Wextra -O3

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

SRCS := $(wildcard $(SRC)/*.c)

OBJS_STATIC  := $(patsubst $(SRC)/%,$(OBJ_STATIC)/%,$(SRCS:.c=.o))
OBJS_DYNAMIC := $(patsubst $(SRC)/%,$(OBJ_DYNAMIC)/%,$(SRCS:.c=.o))

.PHONY: all clean directories static dynamic deps

deps:
    # None

all: directories static dynamic deps

directories:
	@$(MKDIR_CMD)

$(OBJ_STATIC)/%.o: $(SRC)/%.c
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJ_DYNAMIC)/%.o: $(SRC)/%.c
	$(CXX) $(CXXFLAGS) -c $< -o $@ -DBUILDING_DLL

dynamic: $(TARGET_DYLIB)

$(TARGET_DYLIB): $(OBJS_DYNAMIC)
	$(CXX) $(LDFLAGS) -o $@ $^

static: $(TARGET_STATIC)

$(TARGET_STATIC): $(OBJS_STATIC)
	$(AR) rcs $@ $^
	
clean:
	@$(CLEAN_CMD)
