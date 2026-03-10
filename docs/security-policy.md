# Non-Proprietary Security Policy

**Module:** fips-crypto  
**Version:** 1.0.0  
**Security Level:** 1  
**Standard:** FIPS 140-3 / ISO/IEC 19790:2012  
**Document status:** Draft — learning project, not a real CMVP submission  

> This document follows the structure required by FIPS 140-3 Annex A for a
> Non-Proprietary Security Policy (NSP). In a real submission this document
> would be reviewed and published by NIST on the CMVP website.

---

## 1. Module Overview

fips-crypto is a minimal cryptographic library implemented in ANSI C (C99).
It provides a small set of FIPS-approved cryptographic primitives built from
first principles with FIPS 140-3 constraints enforced at the API level.

The module is a software module operating at Security Level 1. It has no
physical security mechanisms and makes no assumptions about the physical
security of the platform on which it runs.

| Field | Value |
|---|---|
| Module name | fips-crypto |
| Version | 1.0.0 |
| Type | Software |
| Security level | 1 |
| Validation standard | FIPS 140-3 / ISO/IEC 19790:2012 |
| Language | ANSI C (C99) |
| Supported platforms | macOS (Apple clang), Linux (gcc) |

---

## 2. Cryptographic Boundary

The cryptographic boundary is logical. It encompasses the following compiled
object files and their corresponding source and header files:

| Component | File |
|---|---|
| AES-128/256 CBC | `src/aes.c`, `include/aes.h` |
| SHA-256 | `src/sha256.c`, `include/sha256.h` |
| HMAC-SHA-256 | `src/hmac.c`, `include/hmac.h` |
| PBKDF2-HMAC-SHA-256 | `src/pbkdf2.c`, `include/pbkdf2.h` |
| FIPS mode and approval | `src/fips.c`, `include/fips.h` |
| Power-on self-tests | `src/self_test.c`, `include/self_test.h` |

Everything outside this boundary — the calling application, the C standard
library, the operating system, the hardware — is the operational environment.

For a full boundary analysis run `make analyse-boundary` from the repo root.
See `docs/boundary.md` for the complete boundary definition.

---

## 3. Approved Algorithms

The following algorithms are approved for use in FIPS mode. Each requires a
CAVP certificate before a real CMVP submission (see Section 10).

| Algorithm | Standard | Key / Output Size | Mode |
|---|---|---|---|
| AES | FIPS 197, SP 800-38A | 128-bit, 256-bit | CBC |
| SHA-256 | FIPS 180-4 | 256-bit output | — |
| HMAC-SHA-256 | FIPS 198-1 | Key ≥ 112 bits | — |
| PBKDF2-HMAC-SHA-256 | SP 800-132 | Variable output | Iterations ≥ 1000, Salt ≥ 16 bytes |

---

## 4. Non-Approved Algorithms

The following algorithms are blocked when FIPS mode is active. Calling any
function that uses these algorithms returns `FIPS_ERR_NOT_APPROVED`.

| Algorithm | Reason |
|---|---|
| MD5 | Not a FIPS-approved algorithm |
| SHA-1 | Deprecated for most uses per SP 800-131A |
| DES | Withdrawn — insufficient key length |
| RC4 | Not a FIPS-approved algorithm |
| Blowfish | Not a FIPS-approved algorithm |

---

## 5. Roles, Services and Authentication

### Roles

FIPS 140-3 requires at least one operator role. At Security Level 1
authentication is not required.

| Role | Description |
|---|---|
| Crypto Officer | Responsible for enabling FIPS mode and initiating self-tests |
| User | Calling application that uses cryptographic services |

### Services

**Crypto Officer services:**

| Service | Description | Functions |
|---|---|---|
| Enable FIPS mode | Activates algorithm blocking and requires KATs to pass | `fips_mode_enable()` |
| Disable FIPS mode | Returns module to non-approved mode (development only) | `fips_mode_disable()` |
| Run self-tests | Executes all KATs and sets approval state | `fips_self_test_run()` |
| Query status | Returns current mode and approval state | `fips_mode_status()`, `fips_mode_active()` |

**User services:**

| Service | Description | Functions |
|---|---|---|
| AES encryption | Encrypt data using AES-CBC | `aes_init()`, `aes_cbc_encrypt()` |
| AES decryption | Decrypt data using AES-CBC | `aes_init()`, `aes_cbc_decrypt()` |
| Hashing | Compute SHA-256 digest | `sha256_init()`, `sha256_update()`, `sha256_final()`, `sha256()` |
| Message authentication | Compute HMAC-SHA-256 | `hmac_sha256_init()`, `hmac_sha256_update()`, `hmac_sha256_final()`, `hmac_sha256()` |
| Key derivation | Derive key material using PBKDF2 | `pbkdf2_hmac_sha256()` |
| Key material zeroization | Securely erase key material from memory | `aes_zeroize()` |
| Algorithm check | Verify an algorithm is approved before use | `fips_check_algorithm()` |

### Authentication

Not applicable at Security Level 1. No authentication is required to assume
either role.

---

## 6. Physical Security

Not applicable. fips-crypto is a software module at Security Level 1. No
physical security mechanisms are implemented or required.

---

## 7. Operational Environment

fips-crypto operates on a general-purpose computing platform. The module
makes the following assumptions about its operational environment:

- The C standard library (`memcpy`, `memset`, `memcmp`) behaves correctly
- The compiler-injected stack protection (`__stack_chk_fail`) is available
- The calling application does not modify module memory directly
- The platform provides a reliable source of entropy for key material (the
  module does not generate keys internally — keys must be provided by the caller)

---

## 8. Cryptographic Key Management

### Key types and sizes

| Key type | Algorithm | Size | Storage |
|---|---|---|---|
| AES key | AES-128-CBC | 128 bits | RAM only |
| AES key | AES-256-CBC | 256 bits | RAM only |
| HMAC key | HMAC-SHA-256 | ≥ 112 bits | RAM only |
| PBKDF2 password | PBKDF2-HMAC-SHA-256 | Variable | RAM only |

### Key entry and output

Keys are passed into the module as byte arrays through the public API. The
module does not generate keys internally (no DRBG is implemented — known gap).
Keys are never output by the module in plaintext.

### Zeroization

All Critical Security Parameters (CSPs) are zeroized using volatile memory
writes to prevent compiler optimisation from eliding the operation. Zeroization
occurs at the following points:

- `aes_zeroize()` — explicitly called by the Crypto Officer or User after key use
- `hmac_sha256_final()` — inner digest buffer zeroed automatically on finalisation
- `pbkdf2_hmac_sha256()` — intermediate U and T buffers zeroed on completion
- `hmac_sha256_init()` — local key material zeroed after key schedule setup

> **Known gap:** The C standard does not guarantee volatile writes will not be
> optimised away by all compilers. A production implementation should use
> `explicit_bzero()` (POSIX) or `memset_s()` (C11 Annex K) instead.

---

## 9. Self-Tests

### Power-On Self-Tests (POST)

The following self-tests run automatically when `fips_self_test_run()` is
called. In a production deployment this must be called before any
cryptographic operation.

| Test | Algorithm | Vector source |
|---|---|---|
| KAT — AES-128-CBC encrypt | AES-128-CBC | FIPS 197 Appendix B |
| KAT — SHA-256 | SHA-256 | FIPS 180-4 Appendix B.1 |
| KAT — HMAC-SHA-256 | HMAC-SHA-256 | RFC 4231 Test Case 1 |

If any KAT fails the module sets `self_test_ok = 0` and all subsequent calls
to `fips_check_algorithm()` return `FIPS_ERR_SELF_TEST`. The module must be
restarted to attempt the self-tests again.

### Missing self-tests (known gaps)

| Test | Description | Reference |
|---|---|---|
| Software integrity test | HMAC-SHA-256 over module binary compared to reference value embedded at build time | FIPS 140-3 §10.3.1 |
| CRNGT | Continuous RNG test — not applicable until a DRBG is implemented | FIPS 140-3 §10.3.2 |
| PCT | Pairwise consistency test — not applicable until asymmetric algorithms are implemented | FIPS 140-3 §10.3.3 |

### On-demand self-tests

The Crypto Officer may call `fips_self_test_run()` at any time to re-run all
KATs. This is the only mechanism for clearing an error state.

---

## 10. CAVP Validation

CAVP certificates are required for every approved algorithm before a CMVP
submission. The table below documents the validation status for this module.

| Algorithm | CAVP Required | Status |
|---|---|---|
| AES-128-CBC | Yes | Simulated — Sprint 8 planned |
| AES-256-CBC | Yes | Simulated — Sprint 8 planned |
| SHA-256 | Yes | Simulated — Sprint 8 planned |
| HMAC-SHA-256 | Yes | Simulated — Sprint 8 planned |
| PBKDF2-HMAC-SHA-256 | Yes | Simulated — Sprint 8 planned |

In a real submission each algorithm would be tested against NIST's ACVP server
at `demo.acvts.nist.gov` and receive a certificate number that is referenced
in this document.

---

## 11. Known Gaps Summary

The following gaps would need to be resolved before a real CMVP submission.
They are documented here for completeness and as a learning reference.

| # | Gap | Impact | Reference |
|---|---|---|---|
| 1 | No software integrity test in POST | Module binary could be tampered without detection | FIPS 140-3 §10.3.1 |
| 2 | No DRBG | Cannot generate keys internally | SP 800-90A Rev 1 |
| 3 | No CRNGT | Not applicable until DRBG is implemented | FIPS 140-3 §10.3.2 |
| 4 | No asymmetric algorithms | No digital signatures or key agreement | FIPS 186-5 |
| 5 | No PCT | Not applicable until asymmetric algorithms are implemented | FIPS 140-3 §10.3.3 |
| 6 | Zeroization uses volatile writes only | Not guaranteed by C standard to be effective on all compilers | FIPS 140-3 §7.9 |
| 7 | HMAC minimum key length not enforced at API level | Caller could pass a key shorter than 112 bits in FIPS mode | FIPS 198-1, SP 800-131A |
| 8 | No CAVP certificates | All algorithm testing is internal — not externally validated | CMVP prerequisite |

---

## 12. References

| Document | Title |
|---|---|
| FIPS 140-3 | Security Requirements for Cryptographic Modules |
| FIPS 197 | Advanced Encryption Standard |
| FIPS 180-4 | Secure Hash Standard |
| FIPS 198-1 | The Keyed-Hash Message Authentication Code |
| FIPS 202 | SHA-3 Standard |
| FIPS 186-5 | Digital Signature Standard |
| SP 800-38A | Recommendation for Block Cipher Modes of Operation |
| SP 800-90A Rev 1 | Recommendation for Random Number Generation Using DRBGs |
| SP 800-131A Rev 2 | Transitioning the Use of Cryptographic Algorithms and Key Lengths |
| SP 800-132 | Recommendation for Password-Based Key Derivation |
| SP 800-140 series | FIPS 140-3 Derived Test Requirements |
| RFC 4231 | HMAC-SHA Identifiers and Test Vectors |
| RFC 8018 | PKCS #5: Password-Based Cryptography Specification Version 2.1 |
