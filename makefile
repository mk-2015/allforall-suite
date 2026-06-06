SRC   := src
LIB   := lib
BIN   := bin
INC   := outinclude
INNER := inner
OUT   := out

CC      := g++
CCFLAGS := -Wall -Wextra -std=c++17

SUBPROJECTS := $(wildcard $(INNER)/*)

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
    else ifeq ($(findstring BSD,$(UNAME_S)),BSD)
        OS_TYPE := LINUX
    else ifeq ($(UNAME_S),Darwin)
        OS_TYPE := MACOS
    else
        OS_TYPE := UNKNOWN
    endif
endif

.PHONY: all setup build_subprojects deploy clean

all: setup build_subprojects deploy

install: all
ifeq ($(OS_TYPE),MSYS)
	cp -r $(OUT)/bin/* /usr/bin/ 2>/dev/null || true
	cp -r $(OUT)/lib/* /usr/lib/ 2>/dev/null || true
	mkdir -p /usr/include
	cp -r $(OUT)/include/* /usr/include/ 2>/dev/null || true
else ifeq ($(OS_TYPE),LINUX)
	cp -r $(OUT)/bin/* /usr/bin/ 2>/dev/null || true
	cp -r $(OUT)/lib/* /usr/lib/ 2>/dev/null || true
	mkdir -p /usr/include
	cp -r $(OUT)/include/* /usr/include/ 2>/dev/null || true
else ifeq ($(OS_TYPE),WINDOWS)
	mkdir -p /allforall/bin /allforall/lib /allforall/include
	-[ -d $(OUT)/bin ] && cp -r $(OUT)/bin/* /allforall/bin/ 2>/dev/null || true
	-[ -d $(OUT)/lib ] && cp -r $(OUT)/lib/* /allforall/lib/ 2>/dev/null || true
	-[ -d $(OUT)/include ] && cp -r $(OUT)/include/* /allforall/include/ 2>/dev/null || true
	setx PATH "%PATH%;C:\allforall\bin" /M
	setx C_INCLUDE_PATH "%C_INCLUDE_PATH%;C:\allforall\include" /M
	setx CPLUS_INCLUDE_PATH "%CPLUS_INCLUDE_PATH%;C:\allforall\include" /M
	setx LIBRARY_PATH "%LIBRARY_PATH%;C:\allforall\lib" /M
else ifeq ($(OS_TYPE),MACOS)
	@echo "MacOs not supported."
	@exit 1
else
	@echo "Unsupported OS detection."
	@exit 1
endif

setup:
	@mkdir -p $(BIN) $(LIB) $(INC) $(OUT)/bin $(OUT)/lib $(OUT)/include

build_subprojects:
	@for dir in $(SUBPROJECTS); do \
		if [ -f "$$dir/Makefile" ] || [ -f "$$dir/makefile" ]; then \
			$(MAKE) -C $$dir; \
		fi; \
		if [ -f "$$dir/.project" ]; then \
			MODE=$$(cat $$dir/.project | tr -d "[:space:]'"); \
			if [ "$$MODE" = "lib" ]; then \
				[ -d $$dir/lib ] && cp -r $$dir/lib/* $(LIB)/ 2>/dev/null || true; \
				[ -d $$dir/include ] && cp -r $$dir/include/* $(INC)/ 2>/dev/null || true; \
			elif [ "$$MODE" = "bin" ]; then \
				[ -d $$dir/bin ] && cp -r $$dir/bin/* $(BIN)/ 2>/dev/null || true; \
			elif [ "$$MODE" = "binlib" ]; then \
				[ -d $$dir/bin ] && cp -r $$dir/bin/* $(BIN)/ 2>/dev/null || true; \
				[ -d $$dir/lib ] && cp -r $$dir/lib/* $(LIB)/ 2>/dev/null || true; \
				[ -d $$dir/include ] && cp -r $$dir/include/* $(INC)/ 2>/dev/null || true; \
			fi; \
		fi; \
	done

deploy:
	@-[ -d $(BIN) ] && cp -r $(BIN)/* $(OUT)/bin/ 2>/dev/null || true
	@-[ -d $(LIB) ] && cp -r $(LIB)/* $(OUT)/lib/ 2>/dev/null || true
	@-[ -d $(INC) ] && cp -r $(INC)/* $(OUT)/include/ 2>/dev/null || true

clean:
	rm -rf $(BIN) $(LIB) $(INC) $(OUT)
	@for dir in $(SUBPROJECTS); do \
		if [ -f "$$dir/Makefile" ] || [ -f "$$dir/makefile" ]; then \
			$(MAKE) -C $$dir clean; \
		fi; \
	done