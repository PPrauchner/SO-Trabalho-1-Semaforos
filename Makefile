CC         = gcc
CFLAGS     = -std=c11 -Wall -Wextra -O0 -pthread
OPT_CFLAGS = -std=c11 -Wall -Wextra -O2 -pthread
LDFLAGS    = -pthread

BIN      = bin/prodcons
OPT_BIN  = bin/prodcons-o2
TEST_BIN = bin/test_buffer
OBJECTS  = src/buffer.o src/main.o
BATTERY  = tests/battery.sh

all: $(BIN)

$(BIN): $(OBJECTS)
	mkdir -p bin
	$(CC) $(OBJECTS) $(LDFLAGS) -o $@

src/buffer.o: src/buffer.c src/buffer.h
src/main.o: src/main.c src/buffer.h

# Binário -O2, separado do -O0: os tempos só se comparam sob a mesma flag.
opt: $(OPT_BIN)

$(OPT_BIN): src/buffer.c src/main.c src/buffer.h
	mkdir -p bin
	$(CC) $(OPT_CFLAGS) src/buffer.c src/main.c $(LDFLAGS) -o $@

$(TEST_BIN): tests/test_buffer.c src/buffer.c src/buffer.h
	mkdir -p bin
	$(CC) $(CFLAGS) tests/test_buffer.c src/buffer.c $(LDFLAGS) -o $@

test: $(TEST_BIN)
	./$(TEST_BIN)

# Medição oficial: as três condições sob -O0.
battery: $(BIN)
	bash $(BATTERY) $(BIN)

# Achado do relatório: se a race some sob -O2, é resultado, não falha de build.
battery-opt: $(OPT_BIN)
	-bash $(BATTERY) $(OPT_BIN)
	@echo "(-O2 e achado do relatorio: um FAIL aqui e resultado a relatar, nao falha de build)"

# Bateria completa: seam de módulo + seam de binário.
check: test battery

clean:
	rm -f $(OBJECTS) $(BIN) $(OPT_BIN) $(TEST_BIN)
	rm -rf bin

.PHONY: all opt test battery battery-opt check clean
