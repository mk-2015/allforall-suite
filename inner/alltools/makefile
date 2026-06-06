SRC   := src
BIN   := bin

CC        := g++
CCFLAGS   := -Wall -Wextra -std=c++17
LINKFLAGS := 

SOURCES := $(sort $(wildcard $(SRC)/*.cpp))

.PHONY: all clean

all: 
	@for file in $(SOURCES); do \
		filename=$$(basename "$$file" .cpp); \
		echo " [CC] $$file"; \
		$(CC) $(CCFLAGS) -c "$$file" -o $(BIN)/"$$filename".o || exit 1; \
		echo " [LD] $$file"; \
		$(CC) $(BIN)/"$$filename".o $(LINKFLAGS) -o $(BIN)/"$$filename" || exit 1; \
	done

clean:
	rm -rf $(BIN)