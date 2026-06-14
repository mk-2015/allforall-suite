INCLUDE := include
SRC     := src
LIB     := lib
OBJ     := obj

LIBNAME := libdot3
TARGET  := $(LIB)/$(LIBNAME)

STATIC_WINDOWS_LIB := $(TARGET).a
STATIC_LINUX_LIB := $(TARGET)-linux.a
DYNAMIC_WINDOWS_LIB := $(TARGET).dll
DYNAMIC_LINUX_LIB := $(TARGET)-linux.so

CXX      := g++
CXXFLAGS := -I$(INCLUDE) -fPIC -Wall -Wextra -O3 -std=c++17

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

SRCS := $(wildcard $(SRC)/*.cpp)

OBJS_STATIC  := $(patsubst $(SRC)/%,$(OBJ_STATIC)/%,$(SRCS:.cpp=.o))
OBJS_DYNAMIC := $(patsubst $(SRC)/%,$(OBJ_DYNAMIC)/%,$(SRCS:.cpp=.o))

.PHONY: all clean directories static dynamic deps

deps:
    # None

all: directories static dynamic deps

directories:
	@$(MKDIR_CMD)

$(OBJ_STATIC)/%.o: $(SRC)/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJ_DYNAMIC)/%.o: $(SRC)/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@ -DBUILDING_DLL

dynamic: $(TARGET_DYLIB)

$(TARGET_DYLIB): $(OBJS_DYNAMIC)
	$(CXX) $(LDFLAGS) -o $@ $^

static: $(TARGET_STATIC)

$(TARGET_STATIC): $(OBJS_STATIC)
	$(AR) rcs $@ $^
	
clean:
	@$(CLEAN_CMD)
