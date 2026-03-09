CC      = gcc
CFLAGS  = -Wall -Wextra -std=c99 -Iinclude

SRC     = $(wildcard src/*.c)
OBJ     = $(SRC:.c=.o)

TESTS   = $(wildcard tests/test_*.c)
TEST_BINS = $(TESTS:tests/test_%.c=tests/test_%)

.PHONY: all test clean

all: $(OBJ)
	@echo "Build OK"

tests/test_%: tests/test_%.c $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^

test: $(TEST_BINS)
	@echo "Running tests..."
	@for t in $(TEST_BINS); do \
		echo "  $$t"; \
		$$t; \
	done

clean:
	rm -f src/*.o tests/test_aes tests/test_sha256 tests/test_hmac \
	      tests/test_fips_mode tests/test_self_test tests/test_pbkdf2
