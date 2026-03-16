# Docker Development Environment

**File:** `docs/docker.md`  
**Relates to:** Sprint 10 — Docker + SAST  
**Prerequisites:** Docker Desktop installed and running

---

## 1. Why Docker

The fips-crypto build depends on a specific toolchain: `gcc`, `libcjson`,
`pkg-config`, Python 3 with the `cryptography` and `requests` packages, and
`semgrep`. On macOS these are installed via Homebrew and a virtual environment.
On Linux they are installed via `apt`. The versions can differ across machines,
and the build can break silently if a dependency is missing or mismatched.

The Docker environment solves this with a single `ubuntu:24.04` image that
contains every dependency at a known version. Anyone who can run Docker can
clone the repo and build, test, and scan without installing anything on their
host machine.

Three specific benefits for this project:

**Reproducibility.** The same image produces the same build output regardless
of the host OS, Homebrew version, or Python environment. This is directly
relevant to SP 800-140Br1 B.2.11 lifecycle assurance — a reproducible build
is evidence of a controlled development environment.

**CI/CD readiness.** The `docker-test` target runs identically on a developer's
laptop and in a GitHub Actions runner. Adding CI requires only a workflow file
that calls `make docker-test`.

**Isolation.** The FIPS toolchain stays off the host machine. The
`.acvp-credentials/` directory is mounted read-only at runtime and is never
baked into the image.

---

## 2. Image Contents

The image is based on `ubuntu:24.04` and contains:

| Component | Source | Purpose |
|---|---|---|
| `gcc`, `make`, `pkg-config` | apt | Build toolchain |
| `libcjson-dev` | apt | cJSON library for acvp_runner |
| `binutils` (`nm`) | apt | Boundary analysis tool |
| `python3`, `python3-venv` | apt | Python runtime |
| `openssl` | apt | TLS / credential tooling |
| `cryptography` | pip (in `/opt/venv`) | ACVP simulation reference library |
| `requests` | pip (in `/opt/venv`) | Sprint 9 ACVP client HTTPS calls |
| `semgrep` | pip (in `/opt/venv`) | SAST and secrets scanning |

The Python venv lives at `/opt/venv` and is on `$PATH` — Python scripts
run without activating it explicitly.

---

## 3. Make Targets

| Target | Description |
|---|---|
| `make docker-build` | Build the `fips-crypto:dev` image |
| `make docker-test` | Run `make test && make acvp-test` inside the container |
| `make docker-semgrep` | Run SAST and secrets scan inside the container |
| `make docker-shell` | Interactive shell with repo mounted at `/workspace` |
| `make semgrep` | Run Semgrep on the host (requires `.venv` with semgrep installed) |

---

## 4. Usage

### 4.1 First-time setup

```bash
# Build the image (only needed once, or after Dockerfile changes)
make docker-build
```

Build takes 2–4 minutes on first run while apt packages and pip packages
are downloaded. Subsequent builds are fast due to Docker layer caching.

### 4.2 Run the full test suite

```bash
make docker-test
```

This runs `make test` (54 unit tests) followed by `make acvp-test` (43 ACVP
simulation vectors) inside the container. The repo is mounted at `/workspace`
so the container always runs against your current source files — no rebuild
needed after code changes.

Expected output ends with:
```
54/54 tests passed
43/43 vectors passed
```

### 4.3 Run SAST and secrets scan

```bash
make docker-semgrep
```

Scans `src/`, `acvp/src/`, and `tools/` against the rules in `.semgrep.yml`.
Reports are printed to stdout. A non-zero exit code means findings were
reported — review each one before committing.

To run on the host instead (requires semgrep in `.venv`):
```bash
make semgrep
```

### 4.4 Interactive shell

```bash
make docker-shell
```

Drops you into a bash shell inside the container with the repo mounted at
`/workspace`. Useful for debugging build failures or running individual
commands manually.

```bash
# Inside the container:
make compile
make test
make acvp-test
make analyse-boundary
semgrep --config .semgrep.yml src/
```

Type `exit` to leave the container. The container is removed automatically
on exit (`--rm` flag).

---

## 5. Credential Handling

The `.acvp-credentials/` directory is mounted read-only into the container:

```
-v "$(PWD)/.acvp-credentials":/workspace/.acvp-credentials:ro
```

This means:
- TLS client certificates and keys are accessible inside the container for
  Sprint 9 ACVP submission scripts
- The `:ro` flag prevents any process inside the container from modifying
  or deleting your credentials
- If `.acvp-credentials/` does not exist yet, the mount is a no-op —
  build and test still work normally

Credentials are **never baked into the image**. The `.dockerignore` file
excludes `.acvp-credentials/` from the build context as a second line of
defence.

---

## 6. Semgrep Rules

The `.semgrep.yml` file at the repo root defines two categories of rules:

### SAST rules

| Rule ID | Severity | What it catches |
|---|---|---|
| `fips-crypto-unsafe-strcpy` | ERROR | `strcpy` — use `strncpy` or `strlcpy` |
| `fips-crypto-unsafe-sprintf` | ERROR | `sprintf` — use `snprintf` |
| `fips-crypto-unsafe-gets` | ERROR | `gets` — removed from C11 |
| `fips-crypto-format-string` | WARNING | Non-literal format string argument |
| `fips-crypto-memset-zeroize` | WARNING | `memset` for zeroization — may be optimised away (Gap #6) |
| `fips-crypto-non-approved-md5` | ERROR | MD5 calls — not approved under FIPS 140-3 |
| `fips-crypto-non-approved-sha1` | ERROR | SHA-1 calls — not approved for new applications |

### Secrets rules

| Rule ID | Severity | What it catches |
|---|---|---|
| `fips-crypto-hardcoded-key-hex` | ERROR | Hardcoded hex key or secret string |
| `fips-crypto-hardcoded-totp-seed` | ERROR | Hardcoded TOTP seed (base32) |
| `fips-crypto-jwt-token` | ERROR | JWT token in source |
| `fips-crypto-private-key-pem` | ERROR | PEM private key block |

The `fips-crypto-memset-zeroize` rule intentionally fires on existing code —
it is a documented known gap (Gap #6 in `docs/security-policy.md`). Treat
it as a reminder rather than a blocker until `explicit_bzero` is implemented.

---

## 7. Adding New Rules

To add a project-specific Semgrep rule, append it to `.semgrep.yml` following
the existing pattern. Useful additions for future sprints:

- Detect calls to `rand()` or `random()` (non-approved RNG — use DRBG instead)
- Detect `strcmp` for MAC comparison (non-constant-time — use `memcmp` with care)
- Detect hardcoded iteration counts below 1000 in PBKDF2 calls

---

## 8. Rebuilding the Image

Rebuild the image when:
- `Dockerfile` changes
- A new system package is needed
- A Python dependency version needs updating

```bash
make docker-build
```

Docker layer caching means only changed layers are rebuilt. If you want a
completely clean build:

```bash
docker build --no-cache -t fips-crypto:dev .
```

---

## 9. References

| Document | URL |
|---|---|
| Docker Documentation | https://docs.docker.com |
| Ubuntu 24.04 Docker Image | https://hub.docker.com/_/ubuntu |
| Semgrep OSS Documentation | https://semgrep.dev/docs |
| Semgrep Rule Syntax | https://semgrep.dev/docs/writing-rules/rule-syntax |
| SP 800-140Br1 B.2.11 | https://csrc.nist.gov/publications/detail/sp/800/140b/r1/final |

---

**Navigation:** [Documentation Index](README.md) · Previous: [How FIPS Validation Works](how-fips-validation-works.md)
