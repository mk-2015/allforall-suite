CXX      := g++
INCLUDE  := include
CXXFLAGS := -Wall -Wextra -std=c++23 -I$(INCLUDE) -DCPPHTTPLIB_OPENSSL_SUPPORT
ifeq ($(OS),Windows_NT)
    CXXFLAGS += -lcrypt32
else
    UNAME_S := $(shell uname -s)
    ifeq ($(UNAME_S),Linux)
        CXXFLAGS += -lcrypto -lssl
    endif
endif
SRC_DIR  := src
OBJ_DIR  := obj
BIN_DIR  := bin
TARGET   := $(BIN_DIR)/snipe

FLAGS := 

SRCS := $(wildcard $(SRC_DIR)/*.cpp)
OBJS := $(patsubst $(SRC_DIR)/%.cpp, $(OBJ_DIR)/%.o, $(SRCS))

all: $(TARGET)

$(TARGET): $(OBJS)
	@mkdir $(BIN_DIR) 2>&1 | true || true
	@echo " [LINK] $(TARGET)"
	@$(CXX) $(CXXFLAGS) $(OBJS) -o $(TARGET) -lstdc++exp

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir $(OBJ_DIR) 2>&1 | true || true
	@echo " [CC] $<"
	@$(CXX) $(CXXFLAGS) $(FLAGS) -c $< -o $@

clean:
	rm -rf $(OBJ_DIR) $(BIN_DIR)
	@echo " [CLEAN] Cleaned"

deps:
	@echo " [cURL] githubusercontent @yhirose/cpp-httplib/master/httplib.h"
	@curl -L https://raw.githubusercontent.com/yhirose/cpp-httplib/master/httplib.h -o $(INCLUDE)/httplib.h

.PHONY: all clean deps