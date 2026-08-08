CXX := g++
CC := gcc
INCLUDE := include
CXXFLAGS := -Wall -Wextra -std=c++23 -I$(INCLUDE) -DCPPHTTPLIB_OPENSSL_SUPPORT -DCURL_STATICLIB
CFLAGS := -Wall -Wextra -I$(INCLUDE)

SRC_DIR := src
OBJ_DIR := obj
BIN_DIR := bin
TARGET := $(BIN_DIR)/hpm

SRCS_CPP := $(wildcard $(SRC_DIR)/*.cpp) $(wildcard $(SRC_DIR)/utils/*.cpp)
SRCS_C := $(wildcard $(SRC_DIR)/*.c)
OBJS := $(patsubst $(SRC_DIR)/%.cpp, $(OBJ_DIR)/%.o, $(SRCS_CPP)) \
        $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(SRCS_C))

ifeq ($(OS),Windows_NT)
    LDLIBS := -lsqlite3 -lyaml -lcurl -lssl -lcrypto -lws2_32 -lcrypt32 -lwsock32 -lwtext -lshlwapi -lssh2 -lz
    MKDIR = mkdir -p $(1)
    RMDIR = rm -rf $(1)
else
    UNAME_S := $(shell uname -s)
    ifeq ($(UNAME_S),Linux)
        LDLIBS := -lsqlite3 -lyaml -lcurl -lssl -lcrypto -lz -lpthread -ldl
    endif
    MKDIR = mkdir -p $(1)	
    RMDIR = rm -rf $(1)
endif

all: $(TARGET)

$(TARGET): $(OBJS)
	@$(call MKDIR, $(BIN_DIR))
	@echo "  [LINK] $(TARGET)"
	@$(CXX) $(CXXFLAGS) $(OBJS) -o $(TARGET) $(LDLIBS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	@$(call MKDIR, $(OBJ_DIR))
	@echo "  [CXX]  $<"
	@$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@$(call MKDIR, $(OBJ_DIR))
	@echo "  [CC]   $<"
	@$(CC) $(CFLAGS) -c $< -o $@

clean:
	@$(call RMDIR, $(OBJ_DIR))
	@$(call RMDIR, $(BIN_DIR))
	@echo "  [CLEAN] Cleaned"

deps:
	@$(call MKDIR, $(OBJ_DIR))
	@$(call MKDIR, $(BIN_DIR))
	mkdir $(OBJ_DIR)/utils
	@$(call MKDIR, $(INCLUDE))
	@$(call MKDIR, $(SRC_DIR))
	# Download headers
	curl -sSL "https://raw.githubusercontent.com/richgel999/miniz/master/miniz.h" -o $(INCLUDE)/miniz.h
	curl -sSL "https://raw.githubusercontent.com/richgel999/miniz/master/miniz_common.h" -o $(INCLUDE)/miniz_common.h
	curl -sSL "https://raw.githubusercontent.com/richgel999/miniz/master/miniz_tdef.h" -o $(INCLUDE)/miniz_tdef.h
	curl -sSL "https://raw.githubusercontent.com/richgel999/miniz/master/miniz_tinfl.h" -o $(INCLUDE)/miniz_tinfl.h
	curl -sSL "https://raw.githubusercontent.com/richgel999/miniz/master/miniz_zip.h" -o $(INCLUDE)/miniz_zip.h
	# Download source files
	curl -sSL "https://raw.githubusercontent.com/richgel999/miniz/master/miniz.c" -o $(SRC_DIR)/miniz.c
	curl -sSL "https://raw.githubusercontent.com/richgel999/miniz/master/miniz_tdef.c" -o $(SRC_DIR)/miniz_tdef.c
	curl -sSL "https://raw.githubusercontent.com/richgel999/miniz/master/miniz_tinfl.c" -o $(SRC_DIR)/miniz_tinfl.c
	curl -sSL "https://raw.githubusercontent.com/richgel999/miniz/master/miniz_zip.c" -o $(SRC_DIR)/miniz_zip.c	
.PHONY: all clean deps