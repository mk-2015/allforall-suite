SRC       := src
BIN       := bin
OBJ       := obj
INC       := include

CC        := g++
CCFLAGS   := -Wall -Wextra -std=c++17 -I$(INC)
LINKFLAGS := 

SOURCES   := $(wildcard $(SRC)/*.cpp)
OBJECTS   := $(patsubst $(SRC)/%.cpp, $(OBJ)/%.o, $(SOURCES))
TARGETS   := $(patsubst $(SRC)/%.cpp, $(BIN)/%, $(SOURCES))

.PHONY: all clean deps

all: $(TARGETS)

deps:
	@mkdir -p $(INC)
	curl -L https://raw.githubusercontent.com/yhirose/cpp-httplib/master/httplib.h -o $(INC)/httplib.h

$(BIN)/%: $(OBJ)/%.o
	@mkdir -p $(BIN)
	@echo " [LINK] $< -> $@"
	$(CC) $< $(LINKFLAGS) -o $@

$(OBJ)/%.o: $(SRC)/%.cpp
	@mkdir -p $(OBJ)
	@echo " [CC] $<"
	$(CC) $(CCFLAGS) -c $< -o $@

clean:
	rm -rf $(BIN) $(OBJ)
