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

## Sprint 8 — ACVP Simulation (Local) ✅
**Goal:** Simulate CAVP algorithm validation locally using ACVP-format JSON
vectors fed to the C library via a C runner binary. Produce ACVP-format
response JSON and validate output against an independent Python reference.

**Pipeline (three stages):**
- Stage 1 — `generate_vectors.py`: generates ACVP-format request JSON using
  Python `cryptography` lib as trusted reference
- Stage 2 — `acvp_runner`: C binary reads request JSON, calls the library,
  writes ACVP-format response JSON
- Stage 3 — `validate_responses.py`: independently compares C output against
  Python reference; does not trust runner's own `"passed"` field

**Deliverables:**
- `acvp/src/acvp_runner.c` — single binary dispatching on `"algorithm"` field
- `acvp/Makefile` — builds runner; `make acvp-test` drives full pipeline
- `acvp/scripts/generate_vectors.py` — Stage 1 (Gemini)
- `acvp/scripts/validate_responses.py` — Stage 3 (Gemini; bug fixed by author)
- `acvp/request/*.json` — runtime-generated, gitignored
- `acvp/response/*.json` — runtime-generated, gitignored
- `docs/acvp_validation_prompt.md` — design record and Gemini prompt
- `LICENSE`, `CONTRIBUTORS.md`, `SECURITY.md` — repo hygiene
- Updated: `README.md`, `docs/algorithm-inventory.md`, `docs/security-policy.md`

**Results:** 43/43 ACVP vectors passing (8 AES-CBC, 11 SHA-256, 12 HMAC-SHA-256, 12 PBKDF2)

**Design decisions:**
- ACVP = protocol (files, targets, code); CAVP = program (policy docs)
- `"expected"` embedded in request JSON — local simulation convention only;
  to be stripped before Sprint 9 demo server submission
- Validation scripts authored by Gemini for independence from C implementation;
  validator bug (tcId-only lookup across multiple testGroups) caught during
  integration and corrected
- AFT only; MCT deferred
- No TLS, no server registration — Sprint 9

**Reference:** NIST ACVP, SP 800-140Br1

---

## Sprint 9 — ACVP Demo Server Submission ⬜
**Goal:** Register with NIST's ACVP demo server (`demo.acvts.nist.gov`) and
replay the Sprint 8 local simulation as a real ACVP exchange.

**Planned deliverables:**
- ACVP client configuration and TLS credentials
- Strip `"expected"` from request JSON before submission
- Submit vectors to demo server; retrieve official response
- Compare demo server results against local simulation results
- Document any discrepancies

**Reference:** NIST ACVTS, SP 800-140Br1
