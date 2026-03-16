# fips-crypto — Sprint Plan

## Sprint 0 — Repo & Plan ✅
**Goal:** Establish repo, README, folder structure, and Makefile skeleton.

**Deliverables:**
- `README.md`
- `plan.md` (this file)
- Folder structure with placeholders
- `Makefile` skeleton
- `setup.sh`

---

## Sprint 1 — AES-128/256 CBC ✅
**Goal:** Implement AES block cipher and CBC mode. Validate against NIST FIPS 197 test vectors.

**Deliverables:**
- `src/aes.c`
- `include/aes.h`
- `tests/test_aes.c`

**Reference:** FIPS 197

---

## Sprint 2 — SHA-256 ✅
**Goal:** Implement SHA-256. Validate against FIPS 180-4 test vectors.

**Deliverables:**
- `src/sha256.c`
- `include/sha256.h`
- `tests/test_sha256.c`

**Reference:** FIPS 180-4

---

## Sprint 3 — HMAC-SHA-256 ✅
**Goal:** Implement HMAC construction over SHA-256. Validate against FIPS 198-1 test vectors.

**Deliverables:**
- `src/hmac.c`
- `include/hmac.h`
- `tests/test_hmac.c`

**Reference:** FIPS 198-1

---

## Sprint 4 — FIPS Mode Flag + Algorithm Blocking ✅
**Goal:** Add a global FIPS mode flag. Any call to a non-approved algorithm returns `FIPS_ERR_NOT_APPROVED`.

**Deliverables:**
- `src/fips.c`
- `include/fips.h`
- `tests/test_fips_mode.c`

**Reference:** FIPS 140-3 §7.6

---

## Sprint 5 — Self-Tests / KATs ✅
**Goal:** Wire Known Answer Tests for each algorithm to run at module load. If any KAT fails, block all operations.

**Deliverables:**
- `src/self_test.c`
- `include/self_test.h`
- `tests/test_self_test.c`

**Reference:** FIPS 140-3 §10.3.1

---

## Sprint 6 — PBKDF2 ✅
**Goal:** Implement PBKDF2-HMAC-SHA-256 key derivation. Validate against SP 800-132 test vectors.

**Deliverables:**
- `src/pbkdf2.c`
- `include/pbkdf2.h`
- `tests/test_pbkdf2.c`

**Reference:** SP 800-132

---

## Sprint 7 — FIPS Documentation ✅
**Goal:** Write the module boundary definition, algorithm inventory, and security policy grounded in the code we built.

**Deliverables:**
- `docs/boundary.md`
- `docs/algorithm-inventory.md`
- `docs/security-policy.md`
- `tools/analyze_boundary.py`

**Reference:** FIPS 140-3 Annex A

---

## Fixing Sprint ✅
**Goal:** Resolve known security gaps identified during Sprint 7 documentation review.

**Changes:**
- Moved `fips_set_self_test_passed` and `fips_self_test_passed` out of public API
- Added `src/fips_internal.h` — internal header for trusted modules only
- `fips_mode_enable` now resets self-test state, requiring KATs to re-run
- Updated all tests to use `fips_mode_status()` instead of raw flag access
- Updated `docs/boundary.md` and `docs/security-policy.md` to reflect resolved gap

**Reference:** FIPS 140-3 §10.3.1

---

## NSP Alignment Sprint ✅
**Goal:** Restructure `docs/security-policy.md` to fully align with the section order and content requirements of SP 800-140Br1 and ISO/IEC 19790:2012 Annex B.

**Changes:**
- Renumbered all sections to B.2.1 through B.2.12 in the required order
- Added B.2.3 Cryptographic Module Interfaces (was missing)
- Added B.2.5 Software/Firmware Security with tested platforms and compiler flags (was missing)
- Added B.2.8 Non-Invasive Security — marked N/A at Level 1 with justification (was missing)
- Added B.2.11 Lifecycle Assurance with CM table and known gaps (was missing)
- Added B.2.12 Mitigation of Other Attacks — marked N/A with justification (was missing)
- Added ASCII block diagram to B.2.2 (was missing); noted as Gap #9
- Restructured B.2.9 SSP table to match SP 800-140Br1 required columns
- Demoted Approved/Non-Approved Algorithms, Known Gaps, and References to appendices
- Known gaps expanded from 8 to 12 items to cover newly identified documentation gaps

**Reference:** SP 800-140Br1, ISO/IEC 19790:2012 Annex B

---

## Sprint 8 — CAVP Simulation ⬜
**Goal:** Simulate CAVP algorithm validation using ACVP-format test vectors fed to the actual C library. Produce results in ACVP response format for each approved algorithm.

**Deliverables:**
- `cavp/aes/vectors.json` + `cavp/aes/results.json`
- `cavp/sha256/vectors.json` + `cavp/sha256/results.json`
- `cavp/hmac/vectors.json` + `cavp/hmac/results.json`
- `cavp/pbkdf2/vectors.json` + `cavp/pbkdf2/results.json`
- `cavp/run_all.py` — harness that drives the C library via subprocess

**Reference:** NIST ACVP, SP 800-140B

---

## Sprint 9 — ACVP Demo Server Submission ⬜
**Goal:** Submit all four approved algorithms to the NIST ACVTS demo server and achieve `"approved"` disposition for each vector set.

**Deliverables:**
- `acvp/scripts/acvp_login.py` — Phase 1: login, write JWT token
- `acvp/scripts/acvp_register.py` — Phase 2–3: register capabilities, retrieve vector sets
- `acvp/scripts/acvp_submit.py` — Phase 5–6: submit responses, retrieve disposition
- `acvp/scripts/acvp_compare.py` — diff server vectors vs local simulation vectors
- `acvp/config/capabilities.json` — algorithm capabilities registration payload
- `docs/acvp/credentials.md` updated with actual NIST process

**Reference:** NIST ACVTS demo server, ACVP protocol

---

## Sprint 10 — Docker + SAST ⬜
**Goal:** Containerise the full development environment for reproducibility,
CI/CD readiness, and host isolation. Add Semgrep for SAST and secrets scanning.

**Deliverables:**
- `Dockerfile` — single Ubuntu 24.04 image: gcc, make, libcjson, Python, semgrep
- `.dockerignore` — excludes credentials and build artifacts from build context
- `.semgrep.yml` — SAST rules (unsafe functions, non-approved algorithms) + secrets rules
- `Makefile` — new targets: `docker-build`, `docker-test`, `docker-semgrep`, `docker-shell`, `semgrep`
- `acvp/Makefile` — platform-aware error messages (macOS / Ubuntu / Docker)
- `docs/docker.md` — how to build, run, and use the container

**FIPS relevance:**
- Reproducible build environment supports SP 800-140Br1 B.2.11 lifecycle assurance
- Semgrep `fips-crypto-memset-zeroize` rule surfaces Gap #6 (zeroization) at every scan
- Semgrep `fips-crypto-non-approved-*` rules enforce algorithm boundary at code level
- Secrets scanning ensures CSPs and ACVTS credentials are never accidentally committed

**Reference:** SP 800-140Br1 B.2.11 (Lifecycle Assurance)
