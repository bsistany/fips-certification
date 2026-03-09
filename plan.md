# fips-crypto — Sprint Plan

## Sprint 0 — Repo & Plan ⬜
**Goal:** Establish repo, README, folder structure, and Makefile skeleton.

**Deliverables:**
- `README.md`
- `plan.md` (this file)
- Folder structure with placeholders
- `Makefile` skeleton
- `setup.sh`

**Done when:** Repo is navigable, `make` runs without error.

---

## Sprint 1 — AES-128/256 CBC ⬜
**Goal:** Implement AES block cipher (key expansion, SubBytes, ShiftRows, MixColumns, AddRoundKey) and CBC mode. Validate against NIST FIPS 197 test vectors.

**Deliverables:**
- `src/aes.c`
- `include/aes.h`
- `tests/test_aes.c`

**Reference:** FIPS 197

---

## Sprint 2 — SHA-256 ⬜
**Goal:** Implement SHA-256 (message schedule, compression function, padding). Validate against FIPS 180-4 test vectors.

**Deliverables:**
- `src/sha256.c`
- `include/sha256.h`
- `tests/test_sha256.c`

**Reference:** FIPS 180-4

---

## Sprint 3 — HMAC-SHA-256 ⬜
**Goal:** Implement HMAC construction over SHA-256. Validate against FIPS 198-1 test vectors.

**Deliverables:**
- `src/hmac.c`
- `include/hmac.h`
- `tests/test_hmac.c`

**Reference:** FIPS 198-1

---

## Sprint 4 — FIPS Mode Flag + Algorithm Blocking ⬜
**Goal:** Add a global FIPS mode flag. Any call to a non-approved algorithm returns a `FIPS_ERROR`. Approved algorithms pass through normally.

**Deliverables:**
- `src/fips.c`
- `include/fips.h`
- `tests/test_fips_mode.c`

**Reference:** FIPS 140-3 §7.6

---

## Sprint 5 — Self-Tests / KATs ⬜
**Goal:** Wire Known Answer Tests for each algorithm to run automatically at module load. If any KAT fails, the module sets an error state and blocks all operations.

**Deliverables:**
- `src/self_test.c`
- `include/self_test.h`
- `tests/test_self_test.c`

**Reference:** FIPS 140-3 §10.3

---

## Sprint 6 — PBKDF2 ⬜
**Goal:** Implement PBKDF2-HMAC-SHA-256 key derivation. Validate against SP 800-132 test vectors.

**Deliverables:**
- `src/pbkdf2.c`
- `include/pbkdf2.h`
- `tests/test_pbkdf2.c`

**Reference:** SP 800-132

---

## Sprint 7 — FIPS Documentation ⬜
**Goal:** Write the module boundary definition, algorithm inventory, and security policy grounded in the code we built.

**Deliverables:**
- `docs/boundary.md`
- `docs/algorithm-inventory.md`
- `docs/security-policy.md`

**Reference:** FIPS 140-3 Annex A
