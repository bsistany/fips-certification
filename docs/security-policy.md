# Non-Proprietary Security Policy

**Module:** fips-crypto  
**Version:** 1.0.0  
**Security Level:** 1  
**Standard:** FIPS 140-3 / ISO/IEC 19790:2012  
**Document status:** Draft — learning project, not a real CMVP submission  

> This document follows the section order required by SP 800-140Br1 and
> ISO/IEC 19790:2012 Annex B (B.2.1 through B.2.12). In a real submission
> this document would be reviewed by a CSTL and published by NIST on the
> CMVP website. Sections not applicable at Security Level 1 are explicitly
> marked as such with justification, as required by the standard.

---

## B.2.1 General

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
| Module type | Software |
| Security level | 1 |
| Validation standard | FIPS 140-3 / ISO/IEC 19790:2012 |
| Language | ANSI C (C99) |
| Tested platforms | macOS 14 (Apple clang 17.0.0, x86_64), Linux (gcc, x86_64) |

---

## B.2.2 Cryptographic Module Specification

### Cryptographic Boundary

The cryptographic boundary is logical. It encompasses the following compiled
object files and their corresponding source and header files:

| Component | Source | Header |
|---|---|---|
| AES-128/256 CBC | `src/aes.c` | `include/aes.h` |
| SHA-256 | `src/sha256.c` | `include/sha256.h` |
| HMAC-SHA-256 | `src/hmac.c` | `include/hmac.h` |
| PBKDF2-HMAC-SHA-256 | `src/pbkdf2.c` | `include/pbkdf2.h` |
| FIPS mode and approval | `src/fips.c` | `include/fips.h` |
| Power-on self-tests | `src/self_test.c` | `include/self_test.h` |
| Internal FIPS interface | `src/fips_internal.h` | *(internal only — not in `include/`)* |

Everything outside this boundary — the calling application, the C standard
library, the operating system, and the hardware — is the operational environment.

For a full boundary analysis run `make analyse-boundary` from the repo root.
See `docs/boundary.md` for the complete boundary definition.

### Block Diagram

The following diagram illustrates the logical location of the module with
respect to the calling application, the C standard library, and the operating
system, as required by SP 800-140Br1 B.2.2.

```
  ┌─────────────────────────────────────────────────────┐
  │               Calling Application                    │
  │         (outside cryptographic boundary)             │
  └────────────────────────┬────────────────────────────┘
                           │  Public API
                           │  (include/*.h)
  ╔════════════════════════▼════════════════════════════╗
  ║          fips-crypto — Cryptographic Boundary        ║
  ║                                                      ║
  ║   ┌──────────┐  ┌──────────┐  ┌──────────────────┐  ║
  ║   │  aes.c   │  │ sha256.c │  │     hmac.c       │  ║
  ║   └──────────┘  └──────────┘  └──────────────────┘  ║
  ║   ┌──────────┐  ┌──────────┐  ┌──────────────────┐  ║
  ║   │ pbkdf2.c │  │  fips.c  │  │  self_test.c     │  ║
  ║   └──────────┘  └──────────┘  └──────────────────┘  ║
  ║                                                      ║
  ║   Internal interface: src/fips_internal.h            ║
  ╚════════════════════════╤════════════════════════════╝
                           │  External dependencies
                           │  (memcpy, memset, memcmp,
                           │   __stack_chk_fail)
  ┌────────────────────────▼────────────────────────────┐
  │           C Standard Library / OS / Hardware         │
  │         (outside cryptographic boundary)             │
  └─────────────────────────────────────────────────────┘
```

> **Known gap (Gap #9):** SP 800-140Br1 B.2.2 requires a proper illustrative
> diagram for a real CMVP submission. A CSTL would require a vector diagram
> (not ASCII art) clearly labelling all logical and physical layers between
> the module and the Tested Operational Environment Physical Perimeter (TOEPP).

---

## B.2.3 Cryptographic Module Interfaces

The module exposes four interface types as defined by FIPS 140-3 §7.6.

### Interface Summary

| Interface type | Description | Examples |
|---|---|---|
| Data Input | Plaintext, keys, passwords, salts passed into the module | AES key and plaintext buffers, HMAC key and message, PBKDF2 password and salt |
| Data Output | Ciphertext, digests, MACs, derived key material returned to the caller | AES ciphertext, SHA-256 digest, HMAC tag, PBKDF2 derived key |
| Control Input | Commands that direct module operation | `fips_mode_enable()`, `fips_self_test_run()`, `fips_check_algorithm()` |
| Status Output | State and error information returned to the caller | `fips_mode_status()`, `fips_mode_active()`, return codes (`FIPS_OK`, `FIPS_ERR_*`) |

### Public API (21 functions)

| Module | Functions |
|---|---|
| `aes.c` | `aes_init`, `aes_cbc_encrypt`, `aes_cbc_decrypt`, `aes_zeroize` |
| `sha256.c` | `sha256_init`, `sha256_update`, `sha256_final`, `sha256` |
| `hmac.c` | `hmac_sha256_init`, `hmac_sha256_update`, `hmac_sha256_final`, `hmac_sha256` |
| `pbkdf2.c` | `pbkdf2_hmac_sha256` |
| `fips.c` | `fips_mode_enable`, `fips_mode_disable`, `fips_mode_active`, `fips_mode_status`, `fips_check_algorithm` |
| `self_test.c` | `fips_self_test_run` |

### External Dependencies (inputs from operational environment)

| Symbol | Source | Purpose |
|---|---|---|
| `memcpy` | C standard library | Buffer copying |
| `memset` | C standard library | Buffer zeroing |
| `memcmp` | C standard library | Digest and tag comparison |
| `__stack_chk_fail` | Compiler / OS | Stack smashing protection |

For full symbol-level detail see `docs/boundary.md`.

---

## B.2.4 Roles, Services, and Authentication

### Roles

FIPS 140-3 requires at least one operator role. At Security Level 1
authentication is not required.

| Role | Description |
|---|---|
| Crypto Officer | Responsible for enabling FIPS mode and initiating self-tests |
| User | Calling application that invokes cryptographic services |

### Services

**Crypto Officer services:**

| Service | Description | Functions |
|---|---|---|
| Enable FIPS mode | Activates algorithm blocking and resets self-test state | `fips_mode_enable()` |
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

## B.2.5 Software/Firmware Security

### Tested Operational Environments

fips-crypto is a software module. The following operational environments have
been tested:

| Platform | OS | Compiler | Architecture |
|---|---|---|---|
| macOS 14 Sonoma | Darwin | Apple clang 17.0.0 | x86_64 |
| Linux (Ubuntu 24) | Linux kernel 6.x | gcc 13.x | x86_64 |

At Security Level 1 there is no requirement for an approved operating system.
The module does not rely on OS-provided security services beyond the C
standard library.

### Compiler Flags

The module is compiled with the following flags on all tested platforms:

```
-Wall -Wextra -std=c99 -Iinclude
```

These flags enforce strict C99 conformance and enable all standard warnings.
Stack protection (`-fstack-protector`) is injected by the compiler by default
on both tested platforms, producing the `__stack_chk_fail` external dependency.

### Software Integrity

> **Known gap (Gap #1):** FIPS 140-3 §10.3.1 requires a software integrity
> test as part of the power-on self-test sequence. This would typically be
> implemented as an HMAC-SHA-256 computed over the module binary at build time
> and verified at load time. This module does not currently implement a
> software integrity test. It would need to be implemented before a real
> CMVP submission.

---

## B.2.6 Operational Environment

fips-crypto operates on a general-purpose computing platform. The module
makes the following assumptions about its operational environment:

- The C standard library (`memcpy`, `memset`, `memcmp`) behaves correctly
- The compiler-injected stack protection (`__stack_chk_fail`) is available
- The calling application does not modify module memory directly
- The platform provides a reliable source of entropy for key generation
  (the module does not generate keys internally — keys must be provided
  by the caller)
- The OS provides process isolation sufficient to protect module memory
  from other processes

At Security Level 1 there is no requirement for a validated or
FIPS-approved operating system.

---

## B.2.7 Physical Security

Not applicable. fips-crypto is a software module at Security Level 1. No
physical security mechanisms are implemented or required. The physical
security of the platform is the responsibility of the operational environment.

---

## B.2.8 Non-Invasive Security

Not applicable at Security Level 1. The FIPS 140-3 requirements for
non-invasive security (resistance to side-channel attacks such as power
analysis, electromagnetic analysis, and timing analysis) apply at Security
Levels 3 and 4. This module makes no claims of resistance to non-invasive
attacks.

> **Note:** A production cryptographic library should consider constant-time
> implementations to mitigate timing side-channels even where not required by
> the standard. The AES and HMAC implementations in this module use table
> lookups and are not hardened against timing attacks. See Gap #12 in
> Appendix B.

---

## B.2.9 Sensitive Security Parameters Management

### SSP Table

The following table lists all Sensitive Security Parameters (SSPs) in the
format required by SP 800-140Br1 B.2.9.

| SSP | Type | Size (bits) | Security Function | CAVP Cert | Generation | Input Method | Output | Storage | Zeroization |
|---|---|---|---|---|---|---|---|---|---|
| AES-128 key | CSP | 128 | AES-128-CBC encrypt/decrypt | Pending | External — caller-provided | `aes_init()` parameter | Never output | RAM only | `aes_zeroize()` — volatile writes |
| AES-256 key | CSP | 256 | AES-256-CBC encrypt/decrypt | Pending | External — caller-provided | `aes_init()` parameter | Never output | RAM only | `aes_zeroize()` — volatile writes |
| HMAC-SHA-256 key | CSP | ≥ 112 | HMAC-SHA-256 authentication | Pending | External — caller-provided | `hmac_sha256_init()` parameter | Never output | RAM only | Volatile writes in `hmac_sha256_init`, `hmac_sha256_final` |
| PBKDF2 password | CSP | Variable | PBKDF2-HMAC-SHA-256 key derivation | Pending | External — caller-provided | `pbkdf2_hmac_sha256()` parameter | Never output | RAM only | Volatile writes on function exit |
| PBKDF2 intermediate block (U) | CSP | 256 | PBKDF2 internal PRF state | N/A | Derived internally | N/A | Never output | RAM (stack) | Volatile writes on function exit |
| `self_test_ok` flag | PSP | 1 (int) | Controls module approval state | N/A | Set by `fips_self_test_run()` | Internal only (`fips_internal.h`) | Via `fips_mode_status()` only | RAM only | Reset by `fips_mode_enable()` |
| `fips_mode_on` flag | PSP | 1 (int) | Controls FIPS mode state | N/A | Set by `fips_mode_enable()` | Public API | Via `fips_mode_active()` only | RAM only | Not applicable |

*CSP = Critical Security Parameter. PSP = Public Security Parameter.*

### Key Entry and Output

Keys and passwords are passed into the module as byte arrays through the
public API. The module does not generate keys or passwords internally — no
DRBG is implemented (Gap #2). SSPs are never output in plaintext.

### Zeroization

All CSPs are zeroized using volatile memory writes at the following points:

- `aes_zeroize()` — called explicitly by the Crypto Officer or User after key use
- `hmac_sha256_final()` — inner digest buffer zeroed automatically on finalisation
- `pbkdf2_hmac_sha256()` — intermediate U and T buffers zeroed on completion
- `hmac_sha256_init()` — local key schedule zeroed after setup

> **Known gap (Gap #6):** The C standard does not guarantee volatile writes
> will not be optimised away. A production implementation should use
> `explicit_bzero()` (POSIX) or `memset_s()` (C11 Annex K) instead.

---

## B.2.10 Self-Tests

### Power-On Self-Tests (POST)

The following Known Answer Tests (KATs) run when `fips_self_test_run()` is
called. In a production deployment this must be called before any
cryptographic operation is performed.

| Test | Algorithm | Vector source |
|---|---|---|
| KAT — AES-128-CBC encrypt | AES-128-CBC | FIPS 197 Appendix B |
| KAT — SHA-256 | SHA-256 | FIPS 180-4 Appendix B.1 |
| KAT — HMAC-SHA-256 | HMAC-SHA-256 | RFC 4231 Test Case 1 |

If any KAT fails the module sets `self_test_ok = 0` and all subsequent calls
to `fips_check_algorithm()` return `FIPS_ERR_SELF_TEST`. The module must be
restarted to attempt the self-tests again. Enabling FIPS mode resets the
self-test state, requiring KATs to be re-run before the module returns to
an approved state.

### Missing Self-Tests (Known Gaps)

| Test | Description | Reference |
|---|---|---|
| Software integrity test (Gap #1) | HMAC-SHA-256 over module binary vs. reference value embedded at build time | FIPS 140-3 §10.3.1 |
| CRNGT (Gap #3) | Continuous RNG test — not applicable until a DRBG is implemented | FIPS 140-3 §10.3.2 |
| PCT (Gap #5) | Pairwise consistency test — not applicable until asymmetric algorithms are implemented | FIPS 140-3 §10.3.3 |

### On-Demand Self-Tests

The Crypto Officer may call `fips_self_test_run()` at any time to re-run all
KATs. This is the only mechanism for returning the module to an approved state
after an error.

---

## B.2.11 Lifecycle Assurance

### Configuration Management

The module source code is maintained in a Git repository hosted on GitHub
(`github.com/bsistany/fips-certification`). All changes are tracked via
Git commits. The build is fully reproducible from source using `make`.

| CM item | Method |
|---|---|
| Source versioning | Git — all changes tracked with commit messages |
| Build reproducibility | `Makefile` — deterministic build from source |
| Test coverage | 54 automated tests across 6 test binaries |
| Sprint planning and traceability | `plan.md` — documents all sprints and their deliverables |

> **Known gap (Gap #10):** A real CMVP submission requires a formal
> configuration management plan documenting CM item identification, access
> controls, change control procedures, and the CM tool itself as a CM item.
> A public GitHub repository with unprotected branches and no required review
> process does not meet the CM requirements of FIPS 140-3 §7.11.

### Delivery and Operation

The module is delivered as source code. The calling application is
responsible for compiling and linking the module using the provided
`Makefile`. Build and usage instructions are provided in `README.md`.

> **Known gap (Gap #11):** A real submission requires formal guidance
> documents covering secure delivery, installation, and initialisation
> procedures, including verification of the delivered source (e.g., signed
> releases or published checksums). None of these are currently implemented.

### Developer Testing

All cryptographic implementations are validated against published test
vectors from NIST and IETF:

| Algorithm | Test vector source |
|---|---|
| AES-128-CBC | FIPS 197 Appendix B, SP 800-38A F.2.1 |
| AES-256-CBC | SP 800-38A F.2.5 |
| SHA-256 | FIPS 180-4 Appendix B.1, B.2, B.3 |
| HMAC-SHA-256 | RFC 4231 TC1–TC4 |
| PBKDF2-HMAC-SHA-256 | RFC 6070 TC1, TC2, TC3, TC5 |

---

## B.2.12 Mitigation of Other Attacks

Not applicable. This module makes no claims regarding mitigation of attacks
outside the scope of the FIPS 140-3 security requirements. No specific
non-invasive attack mitigations have been implemented.

> **Note:** See B.2.8 for the non-invasive security statement. A production
> library should consider constant-time AES and constant-time MAC comparison
> to mitigate practical timing attacks even where not required by the
> standard. See Gap #12 in Appendix B.

---

## Appendix A — Approved and Non-Approved Algorithms

### Approved Algorithms

| Algorithm | Standard | Key / Output Size | Mode | CAVP Status |
|---|---|---|---|---|
| AES | FIPS 197, SP 800-38A | 128-bit, 256-bit | CBC | Pending — Sprint 8 |
| SHA-256 | FIPS 180-4 | 256-bit output | — | Pending — Sprint 8 |
| HMAC-SHA-256 | FIPS 198-1 | Key ≥ 112 bits | — | Pending — Sprint 8 |
| PBKDF2-HMAC-SHA-256 | SP 800-132 | Variable | Iterations ≥ 1000, Salt ≥ 16 bytes | Pending — Sprint 8 |

### Non-Approved Algorithms (blocked in FIPS mode)

| Algorithm | Reason |
|---|---|
| MD5 | Not a FIPS-approved algorithm |
| SHA-1 | Deprecated for most uses per SP 800-131A |
| DES | Withdrawn — insufficient key length |
| RC4 | Not a FIPS-approved algorithm |
| Blowfish | Not a FIPS-approved algorithm |

---

## Appendix B — Known Gaps Summary

The following gaps would need to be resolved before a real CMVP submission.

| # | Gap | Section | Impact | Reference |
|---|---|---|---|---|
| 1 | No software integrity test in POST | B.2.10 | Module binary could be tampered without detection | FIPS 140-3 §10.3.1 |
| 2 | No DRBG | B.2.9 | Cannot generate keys internally | SP 800-90A Rev 1 |
| 3 | No CRNGT | B.2.10 | Not applicable until DRBG implemented | FIPS 140-3 §10.3.2 |
| 4 | No asymmetric algorithms | B.2.4 | No digital signatures or key agreement | FIPS 186-5 |
| 5 | No PCT | B.2.10 | Not applicable until asymmetric algorithms implemented | FIPS 140-3 §10.3.3 |
| 6 | Zeroization uses volatile writes only | B.2.9 | Not guaranteed by C standard on all compilers | FIPS 140-3 §7.9 |
| 7 | HMAC minimum key length not enforced at API level | B.2.9 | Caller could pass key shorter than 112 bits | FIPS 198-1, SP 800-131A |
| 8 | No CAVP certificates | Appendix A | Algorithm testing is internal only | CMVP prerequisite |
| 9 | No proper block diagram | B.2.2 | ASCII diagram not acceptable for real CSTL review | SP 800-140Br1 B.2.2 |
| 10 | No formal CM plan | B.2.11 | GitHub without branch protection does not meet CM requirements | FIPS 140-3 §7.11 |
| 11 | No formal delivery/guidance documents | B.2.11 | No secure installation or initialisation procedures | FIPS 140-3 §7.11 |
| 12 | No timing attack mitigations | B.2.12 | AES table lookups and HMAC comparison are not constant-time | Practical concern |

---

## Appendix C — References

| Document | Title |
|---|---|
| FIPS 140-3 | Security Requirements for Cryptographic Modules |
| FIPS 197 | Advanced Encryption Standard |
| FIPS 180-4 | Secure Hash Standard |
| FIPS 198-1 | The Keyed-Hash Message Authentication Code |
| FIPS 186-5 | Digital Signature Standard |
| SP 800-38A | Recommendation for Block Cipher Modes of Operation |
| SP 800-90A Rev 1 | Recommendation for Random Number Generation Using DRBGs |
| SP 800-131A Rev 2 | Transitioning the Use of Cryptographic Algorithms and Key Lengths |
| SP 800-132 | Recommendation for Password-Based Key Derivation |
| SP 800-140Br1 | CMVP Security Policy Requirements (November 2023) |
| ISO/IEC 19790:2012 | Security requirements for cryptographic modules |
| RFC 4231 | HMAC-SHA Identifiers and Test Vectors |
| RFC 6070 | PBKDF2 Test Vectors |
| RFC 8018 | PKCS #5: Password-Based Cryptography Specification Version 2.1 |
