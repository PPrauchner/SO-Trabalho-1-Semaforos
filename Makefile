CC      = gcc
CFLAGS  = -std=c11 -Wall -Wextra -O0 -pthread
LDFLAGS = -pthread

BIN      = bin/prodcons
TEST_BIN = bin/test_buffer
OBJECTS  = src/buffer.o src/main.o

all: $(BIN)

$(BIN): $(OBJECTS)
	mkdir -p bin
	$(CC) $(OBJECTS) $(LDFLAGS) -o $@

src/buffer.o: src/buffer.c src/buffer.h
src/main.o: src/main.c src/buffer.h

$(TEST_BIN): tests/test_buffer.c src/buffer.c src/buffer.h
	mkdir -p bin
	$(CC) $(CFLAGS) tests/test_buffer.c src/buffer.c $(LDFLAGS) -o $@

test: $(TEST_BIN)
	./$(TEST_BIN)

clean:
	rm -f $(OBJECTS) $(BIN) $(TEST_BIN)
	rm -rf bin

.PHONY: all test clean
