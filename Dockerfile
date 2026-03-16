# fips-crypto — Development Environment
#
# Single image covering:
#   - Build and compile (gcc, make, pkg-config, libcjson)
#   - Unit tests (make test)
#   - ACVP simulation pipeline (make acvp-test)
#   - ACVP demo server submission (Sprint 9 Python scripts)
#   - SAST and secrets scanning (semgrep)
#
# Build:
#   make docker-build
#
# Run all tests:
#   make docker-test
#
# Interactive shell:
#   make docker-shell
#
# Base: ubuntu:24.04 — matches a typical CI runner environment.
# TLS credentials (.acvp-credentials/) are mounted at runtime,
# never baked into the image.

FROM ubuntu:24.04

# ---------------------------------------------------------------------------
# Prevent apt from prompting for timezone during tzdata install
# ---------------------------------------------------------------------------
ENV DEBIAN_FRONTEND=noninteractive

# ---------------------------------------------------------------------------
# System packages
# ---------------------------------------------------------------------------
RUN apt-get update && apt-get install -y --no-install-recommends \
    # Build toolchain
    gcc \
    make \
    pkg-config \
    # C standard library headers (required for stdint.h, string.h etc.)
    libc6-dev \
    # cJSON (replaces brew install cjson on macOS)
    libcjson-dev \
    # Boundary analysis
    binutils \
    # Python runtime
    python3 \
    python3-pip \
    python3-venv \
    # TLS / credential tooling
    openssl \
    # Utilities
    ca-certificates \
    curl \
    git \
    && rm -rf /var/lib/apt/lists/*

# ---------------------------------------------------------------------------
# Python dependencies
# Install into a venv at /opt/venv so pip --break-system-packages
# is not needed and the path is predictable across Ubuntu versions.
# ---------------------------------------------------------------------------
ENV VIRTUAL_ENV=/opt/venv
RUN python3 -m venv $VIRTUAL_ENV
ENV PATH="$VIRTUAL_ENV/bin:$PATH"

RUN pip install --upgrade pip && \
    pip install \
        cryptography \
        requests \
        semgrep

# ---------------------------------------------------------------------------
# Working directory
# The repo is mounted here at runtime — not copied into the image.
# This keeps the image reusable across code changes without rebuilding.
# ---------------------------------------------------------------------------
WORKDIR /workspace

# ---------------------------------------------------------------------------
# Default command — run the full test suite
# Override with docker-shell target for interactive use.
# ---------------------------------------------------------------------------
CMD ["make", "test"]
