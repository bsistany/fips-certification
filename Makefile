CC      = gcc
CFLAGS  = -Wall -Wextra -std=c99 -Iinclude

.PHONY: all compile link test clean analyse-boundary acvp-test

all: compile link test

compile:
	$(CC) $(CFLAGS) -c -o src/aes.o       src/aes.c
	$(CC) $(CFLAGS) -c -o src/sha256.o    src/sha256.c
	$(CC) $(CFLAGS) -c -o src/hmac.o      src/hmac.c
	$(CC) $(CFLAGS) -c -o src/fips.o      src/fips.c
	$(CC) $(CFLAGS) -c -o src/self_test.o src/self_test.c
	$(CC) $(CFLAGS) -c -o src/pbkdf2.o    src/pbkdf2.c
	@echo "Compile OK"

link: compile
	$(CC) $(CFLAGS) -o tests/test_aes       tests/test_aes.c       src/aes.o
	$(CC) $(CFLAGS) -o tests/test_sha256    tests/test_sha256.c    src/sha256.o
	$(CC) $(CFLAGS) -o tests/test_hmac      tests/test_hmac.c      src/hmac.o src/sha256.o
	$(CC) $(CFLAGS) -o tests/test_fips_mode tests/test_fips_mode.c src/fips.o src/self_test.o src/aes.o src/sha256.o src/hmac.o
	$(CC) $(CFLAGS) -o tests/test_self_test tests/test_self_test.c \
	      src/self_test.o src/fips.o src/aes.o src/sha256.o src/hmac.o
	$(CC) $(CFLAGS) -o tests/test_pbkdf2    tests/test_pbkdf2.c \
	      src/pbkdf2.o src/hmac.o src/sha256.o src/fips.o src/self_test.o src/aes.o
	@echo "Link OK"

test: link
	@echo "Running tests..."
	./tests/test_aes
	./tests/test_sha256
	./tests/test_hmac
	./tests/test_fips_mode
	./tests/test_self_test
	./tests/test_pbkdf2

analyse-boundary: compile
	python3 tools/analyze_boundary.py

clean:
	rm -f src/*.o tests/test_aes tests/test_sha256 tests/test_hmac \
	      tests/test_fips_mode tests/test_self_test tests/test_pbkdf2

acvp-test: compile
	$(MAKE) -C acvp acvp-test

acvp-clean:
	$(MAKE) -C acvp clean
# ---------------------------------------------------------------------------
# Docker targets (Sprint 10)
# ---------------------------------------------------------------------------
DOCKER_IMAGE = fips-crypto:dev
DOCKER_RUN   = docker run --rm \
                 -v "$(PWD)":/workspace \
                 -v "$(PWD)/.acvp-credentials":/workspace/.acvp-credentials:ro \
                 $(DOCKER_IMAGE)

.PHONY: docker-build docker-test docker-semgrep docker-shell

## docker-build : build the fips-crypto:dev image
docker-build:
	docker build -t $(DOCKER_IMAGE) .

## docker-test  : run full test suite inside the container
##                (make test + make acvp-test)
docker-test: docker-build
	$(DOCKER_RUN) sh -c "make test && make acvp-test"

## docker-semgrep : run SAST and secrets scan inside the container
docker-semgrep: docker-build
	$(DOCKER_RUN) sh -c "semgrep --config .semgrep.yml src/ acvp/src/ tools/"

## docker-shell : interactive shell with repo mounted
docker-shell: docker-build
	docker run --rm -it \
	  -v "$(PWD)":/workspace \
	  -v "$(PWD)/.acvp-credentials":/workspace/.acvp-credentials:ro \
	  $(DOCKER_IMAGE) /bin/bash

# ---------------------------------------------------------------------------
# Semgrep targets (host — requires semgrep installed in .venv)
# ---------------------------------------------------------------------------
.PHONY: semgrep

## semgrep : run SAST and secrets scan on the host
semgrep:
	.venv/bin/semgrep --config .semgrep.yml src/ acvp/src/ tools/
