CC      = gcc
CFLAGS  = -Wall -Wextra -std=c99 -Iinclude

.PHONY: all compile link test clean

all: compile link test

compile:
	$(CC) $(CFLAGS) -c -o src/aes.o    src/aes.c
	$(CC) $(CFLAGS) -c -o src/sha256.o src/sha256.c
	@echo "Compile OK"

link: compile
	$(CC) $(CFLAGS) -o tests/test_aes    tests/test_aes.c    src/aes.o
	$(CC) $(CFLAGS) -o tests/test_sha256 tests/test_sha256.c src/sha256.o
	@echo "Link OK"

test: link
	@echo "Running tests..."
	./tests/test_aes
	./tests/test_sha256

clean:
	rm -f src/*.o tests/test_aes tests/test_sha256
