CC      = gcc
CFLAGS  = -std=c11 -Wall -Wextra -O0 -pthread
LDFLAGS = -pthread

BIN     = bin/prodcons
OBJECTS = src/buffer.o src/main.o

all: $(BIN)

$(BIN): $(OBJECTS)
	mkdir -p bin
	$(CC) $(OBJECTS) $(LDFLAGS) -o $@

src/buffer.o: src/buffer.c src/buffer.h
src/main.o: src/main.c src/buffer.h

clean:
	rm -f $(OBJECTS) $(BIN)
	rm -rf bin

.PHONY: all clean
