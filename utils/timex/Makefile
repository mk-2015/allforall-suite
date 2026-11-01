CXX = g++
AR  = ar

CXXFLAGS = -std=c++20 -Wall -Wextra -Wpedantic -O2 -fPIC
CPPFLAGS = -Iinclude

SRC = src
OBJ = obj
LIB = lib

STATIC = $(LIB)/libtimex.a
SHARED = $(LIB)/libtimex.so

SOURCES = $(wildcard $(SRC)/*.cpp)
OBJECTS = $(patsubst $(SRC)/%.cpp,$(OBJ)/%.o,$(SOURCES))

.PHONY: all clean

all: $(STATIC) $(SHARED)

$(OBJ)/%.o: $(SRC)/%.cpp
	@mkdir -p $(OBJ)
	@echo " [CXX] $<"
	@$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

$(STATIC): $(OBJECTS)
	@mkdir -p $(LIB)
	@echo " [AR] $@"
	@$(AR) rcs $@ $^

$(SHARED): $(OBJECTS)
	@mkdir -p $(LIB)
	@echo " [LINK] $@"
	@$(CXX) -shared -o $@ $^

clean:
	@rm -rf $(OBJ) $(LIB)
	@echo " [CLEAN]"
