CC      = gcc
CFLAGS  = -Wall -Wextra -std=c99 -Iinclude

.PHONY: all compile link test clean

all: compile link test

compile:
	$(CC) $(CFLAGS) -c -o src/aes.o    src/aes.c
	$(CC) $(CFLAGS) -c -o src/sha256.o src/sha256.c
	$(CC) $(CFLAGS) -c -o src/hmac.o   src/hmac.c
	$(CC) $(CFLAGS) -c -o src/fips.o   src/fips.c
	@echo "Compile OK"

link: compile
	$(CC) $(CFLAGS) -o tests/test_aes       tests/test_aes.c       src/aes.o
	$(CC) $(CFLAGS) -o tests/test_sha256    tests/test_sha256.c    src/sha256.o
	$(CC) $(CFLAGS) -o tests/test_hmac      tests/test_hmac.c      src/hmac.o src/sha256.o
	$(CC) $(CFLAGS) -o tests/test_fips_mode tests/test_fips_mode.c src/fips.o
	@echo "Link OK"

test: link
	@echo "Running tests..."
	./tests/test_aes
	./tests/test_sha256
	./tests/test_hmac
	./tests/test_fips_mode

clean:
	rm -f src/*.o tests/test_aes tests/test_sha256 tests/test_hmac tests/test_fips_mode
