ifeq ($(OS),Windows_NT)
    UNAME_S := Windows
    ifneq ($(findstring MSYS,$(shell uname -s 2>/dev/null)),)
        OS_TYPE := MSYS
    else ifneq ($(findstring MINGW,$(shell uname -s 2>/dev/null)),)
        OS_TYPE := MSYS
    else
        OS_TYPE := WINDOWS
    endif
else
    UNAME_S := $(shell uname -s)
    ifeq ($(UNAME_S),Linux)
        OS_TYPE := LINUX
    else ifneq ($(findstring BSD,$(UNAME_S)),)
        OS_TYPE := BSD
    else ifeq ($(UNAME_S),Darwin)
        OS_TYPE := MACOS
    else
        OS_TYPE := UNKNOWN
    endif
endif

SRC       := src
BIN       := bin
OBJ       := obj
INC       := include

CC        := g++
CCFLAGS   := -Wall -Wextra -std=c++17 -I$(INC)
LINKFLAGS := 

ifeq ($(OS_TYPE),LINUX)
    CCFLAGS   += -pthread
    LINKFLAGS += -pthread
else ifeq ($(OS_TYPE),BSD)
    CCFLAGS   += -pthread
    LINKFLAGS += -pthread
else ifneq ($(filter $(OS_TYPE),WINDOWS MSYS),)
    LINKFLAGS += -lws2_32
endif

SOURCES   := $(wildcard $(SRC)/*.cpp)
OBJECTS   := $(patsubst $(SRC)/%.cpp, $(OBJ)/%.o, $(SOURCES))
TARGETS   := $(patsubst $(SRC)/%.cpp, $(BIN)/%, $(SOURCES))

.PHONY: all clean deps check_os

all: check_os $(TARGETS)

check_os:
ifeq ($(OS_TYPE),MACOS)
	@echo "Error: MacOS is not supported by the All For All toolkit."
	@exit 1
endif
ifeq ($(OS_TYPE),UNKNOWN)
	@echo "Error: Unsupported or unidentified OS detection profile."
	@exit 1
endif

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