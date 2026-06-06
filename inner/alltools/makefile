SRC   := src
BIN   := bin
OBJ   := obj

CC        := g++
CCFLAGS   := -Wall -Wextra -std=c++17
LINKFLAGS := 

SOURCES := $(sort $(wildcard $(SRC)/*.cpp))

.PHONY: all clean

all: 
	@for file in $(SOURCES); do \
		filename=$$(basename "$$file" .cpp); \
		echo " [CC] $$file"; \
		$(CC) $(CCFLAGS) -c "$$file" -o $(OBJ)/"$$filename".o || exit 1; \
		$(CC) $(OBJ)/"$$filename".o $(LINKFLAGS) -o $(BIN)/"$$filename" || exit 1; \
	done
	
	@rm -rf $(OBJ) 

clean:
	rm -rf $(BIN)