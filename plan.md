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

## Sprint 8 — CAVP Simulation ⬜
**Goal:** Simulate CAVP algorithm validation using ACVP-format test vectors fed to the actual C library. Produce results in ACVP response format for each approved algorithm.

**Deliverables:**
- `cavp/aes/vectors.json` + `cavp/aes/results.json`
- `cavp/sha256/vectors.json` + `cavp/sha256/results.json`
- `cavp/hmac/vectors.json` + `cavp/hmac/results.json`
- `cavp/pbkdf2/vectors.json` + `cavp/pbkdf2/results.json`
- `cavp/run_all.py` — harness that drives the C library via subprocess

**Reference:** NIST ACVP, SP 800-140B
