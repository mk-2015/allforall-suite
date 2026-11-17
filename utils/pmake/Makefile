CXX      := g++
CPPFLAGS := -MMD -MP -Iinclude -Isrc
CXXFLAGS := -Wall -Wextra -Wpedantic -g -std=c++20
LDFLAGS  :=
LDLIBS   :=

SRC_DIR := src
OBJ_DIR := obj
BIN_DIR := bin
INC_DIR := include
TMP_DIR := .deps

TARGET := $(BIN_DIR)/pmake

TOML_REPO    := https://github.com/marzer/tomlplusplus
TOML_VERSION := v3.4.0
TOML_DIR     := $(INC_DIR)/toml++
TOML_HEADER  := $(TOML_DIR)/toml.hpp

CLI11_REPO    := https://github.com/CLIUtils/CLI11
CLI11_VERSION := v2.7.2
CLI11_HEADER  := $(INC_DIR)/CLI/CLI.hpp

SRCS := $(shell find $(SRC_DIR) -type f \
	\( -name '*.cpp' -o -name '*.cc' -o -name '*.cxx' \))

OBJS := $(patsubst $(SRC_DIR)/%,$(OBJ_DIR)/%.o,$(SRCS))
DEPS := $(OBJS:.o=.d)

.DEFAULT_GOAL := all
.PHONY: all deps clean distclean love

love:
	@echo "Make LOVE not WAR"

all: $(TARGET)

deps: $(TOML_HEADER) $(CLI11_HEADER)

$(TOML_HEADER):
	@mkdir -p $(TMP_DIR) $(INC_DIR)
	@echo "[GIT] toml++ $(TOML_VERSION)"
	@git clone --depth 1 --branch $(TOML_VERSION) \
		$(TOML_REPO) $(TMP_DIR)/toml
	@rm -rf $(TOML_DIR)
	@cp -R $(TMP_DIR)/toml/include/toml++ $(INC_DIR)/
	@rm -rf $(TMP_DIR)/toml
	@echo "[DEPS] toml++ installed"

$(CLI11_HEADER):
	@mkdir -p $(TMP_DIR) $(INC_DIR)
	@echo "[GIT] CLI11 $(CLI11_VERSION)"
	@git clone --depth 1 --branch $(CLI11_VERSION) \
		$(CLI11_REPO) $(TMP_DIR)/cli11
	@rm -rf $(INC_DIR)/CLI
	@cp -R $(TMP_DIR)/cli11/include/CLI $(INC_DIR)/
	@rm -rf $(TMP_DIR)/cli11
	@echo "[DEPS] CLI11 installed"

$(TARGET): $(OBJS)
	@mkdir -p $(dir $@)
	@$(CXX) $(LDFLAGS) $(OBJS) $(LDLIBS) -o $@
	@echo "[LINK] $@"

$(OBJ_DIR)/%.o: $(SRC_DIR)/% | deps
	@mkdir -p $(dir $@)
	@$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@
	@echo "[CXX] $<"

-include $(DEPS)

distclean:
	@rm -rf $(TOML_DIR) $(CLI11_HEADER) $(TMP_DIR)
	@echo "[CLEAN] Dependencies removed"


clean: distclean
	@rm -rf $(OBJ_DIR) $(BIN_DIR)
	@echo "[CLEAN] Build files removed"