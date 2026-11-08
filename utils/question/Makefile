CXX = g++
AR  = ar

ICU_CFLAGS := $(shell pkg-config --cflags icu-i18n)
ICU_LIBS   := $(shell pkg-config --libs icu-i18n icu-uc)

CXXFLAGS = -std=c++20 -Wall -Wextra -Wpedantic -O2 -fPIC -Isrc/
CPPFLAGS = -Iinclude $(ICU_CFLAGS)

SRC = src
OBJ = obj
LIB = lib

STATIC = $(LIB)/libque.a
SHARED = $(LIB)/libque.so

SOURCES = $(shell find $(SRC) -type f -name '*.cpp')
OBJECTS = $(patsubst $(SRC)/%.cpp,$(OBJ)/%.o,$(SOURCES))

.PHONY: all clean deps

all: $(STATIC) $(SHARED)

$(OBJ)/%.o: $(SRC)/%.cpp
	@mkdir -p $(dir $@)
	@echo " [CXX] $<"
	@$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

$(STATIC): $(OBJECTS)
	@mkdir -p $(LIB)
	@echo " [AR] $@"
	@$(AR) rcs $@ $^

$(SHARED): $(OBJECTS)
	@mkdir -p $(LIB)
	@echo " [LINK] $@"
	@$(CXX) -shared -o $@ $^ $(ICU_CFLAGS) $(ICU_LIBS)

clean:
	@rm -rf $(OBJ) $(LIB)
	@echo " [CLEAN]"

deps:
	@echo "NONE"
