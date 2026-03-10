# acvp/Makefile
#
# Builds acvp_runner and drives the three-stage ACVP simulation pipeline:
#
#   Stage 1 — generate_vectors.py  : writes acvp/request/*.json
#   Stage 2 — acvp_runner          : reads request/, writes response/
#   Stage 3 — validate_responses.py: diffs output vs expected, reports
#
# Usage (from repo root):
#   make -C acvp          # build acvp_runner only
#   make -C acvp clean    # remove binary, objects, and response JSON
#
# Or via the top-level Makefile target:
#   make acvp-test        # full pipeline: generate → run → validate
#
# Prerequisites:
#   brew install cjson
#   pip install cryptography

# ---------------------------------------------------------------------------
# Compiler — mirror the top-level Makefile exactly
# ---------------------------------------------------------------------------
CC     = gcc
CFLAGS = -Wall -Wextra -std=c99
PYTHON = ../.venv/bin/python3

# Include paths:
#   -I../include   : project headers (aes.h, sha256.h, hmac.h, pbkdf2.h, ...)
#   cJSON headers  : resolved via pkg-config
CJSON_CFLAGS := $(shell pkg-config --cflags libcjson 2>/dev/null)
CJSON_LIBS   := $(shell pkg-config --libs   libcjson 2>/dev/null)

# Friendly error if cJSON is not installed
ifeq ($(CJSON_CFLAGS),)
$(error cJSON not found. Install with: brew install cjson)
endif

ALL_CFLAGS = $(CFLAGS) -I../include $(CJSON_CFLAGS)

# ---------------------------------------------------------------------------
# Source and object files
# ---------------------------------------------------------------------------

# Runner source lives alongside this Makefile
RUNNER_SRC = src/acvp_runner.c

# Pre-compiled object files from the top-level build.
# Must mirror the full dependency chain required by test_pbkdf2 (the heaviest
# consumer) — all six objects are needed.
SRC_OBJS = \
    ../src/aes.o       \
    ../src/sha256.o    \
    ../src/hmac.o      \
    ../src/pbkdf2.o    \
    ../src/fips.o      \
    ../src/self_test.o

RUNNER_BIN = acvp_runner

# ---------------------------------------------------------------------------
# Directories
# ---------------------------------------------------------------------------
REQUEST_DIR  = request
RESPONSE_DIR = response
SCRIPTS_DIR  = scripts

# ---------------------------------------------------------------------------
# Phony targets
# ---------------------------------------------------------------------------
.PHONY: all clean acvp-test check-deps

# ---------------------------------------------------------------------------
# Default: build the runner binary
# ---------------------------------------------------------------------------
all: check-deps $(SRC_OBJS) $(RUNNER_BIN)

$(RUNNER_BIN): $(RUNNER_SRC) $(SRC_OBJS)
	$(CC) $(ALL_CFLAGS) -o $@ $(RUNNER_SRC) $(SRC_OBJS) $(CJSON_LIBS)
	@echo "Build OK: $(RUNNER_BIN)"

# Compile top-level .o files if they don't exist or are stale.
# Mirrors the top-level Makefile compile step exactly.
../src/%.o: ../src/%.c
	$(CC) $(CFLAGS) -I../include -c -o $@ $<

# ---------------------------------------------------------------------------
# Dependency check
# ---------------------------------------------------------------------------
check-deps:
	@command -v $(PYTHON) >/dev/null 2>&1 || \
	    { echo "ERROR: python not found"; exit 1; }
	@$(PYTHON) -c "import cryptography" 2>/dev/null || \
	    { echo "ERROR: Python 'cryptography' package not found."; \
	      echo "       Install with: pip install cryptography"; exit 1; }
	@pkg-config --exists libcjson || \
	    { echo "ERROR: cjson not found. Install with: brew install cjson"; exit 1; }

# ---------------------------------------------------------------------------
# Full pipeline: generate → run → validate
# ---------------------------------------------------------------------------
acvp-test: all
	@echo ""
	@echo "=== Stage 1: Generating ACVP request vectors ==="
	$(PYTHON) $(SCRIPTS_DIR)/generate_vectors.py

	@echo ""
	@echo "=== Stage 2: Running acvp_runner ==="
	./$(RUNNER_BIN) $(REQUEST_DIR)/aes_cbc.json     $(RESPONSE_DIR)/aes_cbc.json
	./$(RUNNER_BIN) $(REQUEST_DIR)/sha256.json       $(RESPONSE_DIR)/sha256.json
	./$(RUNNER_BIN) $(REQUEST_DIR)/hmac_sha256.json  $(RESPONSE_DIR)/hmac_sha256.json
	./$(RUNNER_BIN) $(REQUEST_DIR)/pbkdf2.json       $(RESPONSE_DIR)/pbkdf2.json

	@echo ""
	@echo "=== Stage 3: Validating responses ==="
	$(PYTHON) $(SCRIPTS_DIR)/validate_responses.py

# ---------------------------------------------------------------------------
# Clean
# ---------------------------------------------------------------------------
clean:
	rm -f $(RUNNER_BIN)
	rm -f $(RESPONSE_DIR)/*.json
	@echo "Clean OK"
