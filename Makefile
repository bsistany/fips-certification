CC      = clang
CFLAGS  = -Wall -Wextra -std=c99 -Iinclude

.PHONY: compile link test clean

all: compile link test

# Step 1: compile source files to object files
compile:
	$(CC) $(CFLAGS) -c -o src/aes.o src/aes.c
	@echo "Compile OK"

# Step 2: link test binaries
link: compile
	$(CC) $(CFLAGS) -o tests/test_aes tests/test_aes.c src/aes.o
	@echo "Link OK"

# Run tests
test: link
	@echo "Running tests..."
	./tests/test_aes

clean:
	rm -f src/*.o tests/test_aes
