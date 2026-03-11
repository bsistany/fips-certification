# How FIPS Cryptographic Validation Works

**File:** `docs/how-fips-validation-works.md`  
**Audience:** Technical readers with no prior FIPS 140-3 or CMVP experience  
**Purpose:** Explains the FIPS 140-3 validation landscape, how this project
maps to it, and what a real certification journey looks like end to end.

---

## Table of Contents

1. [Why Cryptographic Module Validation Exists](#1-why-cryptographic-module-validation-exists)
2. [The Standards Landscape](#2-the-standards-landscape)
3. [The CMVP Process — End to End](#3-the-cmvp-process--end-to-end)
4. [Algorithm Validation — CAVP and ACVP](#4-algorithm-validation--cavp-and-acvp)
5. [The Role of a CSTL](#5-the-role-of-a-cstl)
6. [What This Project Implements](#6-what-this-project-implements)
7. [What This Project Is Not](#7-what-this-project-is-not)
8. [Sprint-by-Sprint Traceability](#8-sprint-by-sprint-traceability)
9. [What Would Be Needed to Go Further](#9-what-would-be-needed-to-go-further)
10. [Glossary](#10-glossary)
11. [References](#11-references)

---

## 1. Why Cryptographic Module Validation Exists

Cryptography is foundational to information security. Governments, financial
institutions, healthcare providers, and defence contractors all depend on
cryptographic software to protect sensitive data. But cryptographic
implementations are notoriously difficult to get right — subtle bugs in AES
key scheduling, SHA-256 padding, or HMAC construction can completely undermine
security guarantees while producing output that looks superficially correct.

The question "does this cryptographic library work correctly?" cannot be
answered by reading the code or running basic tests alone. It requires:

- Validation against authoritative test vectors from standards bodies
- Verification that unapproved algorithms are blocked at runtime
- Evidence that key material is zeroized after use
- Proof that the module tests itself at startup and fails safe if something
  is wrong

**FIPS 140-3** (Federal Information Processing Standard 140-3) is the US
government standard that defines what "correctly implemented" means for a
cryptographic module. Any vendor supplying cryptographic software to US
federal agencies — and many private sector organisations — must demonstrate
compliance with FIPS 140-3.

The standard is administered jointly by NIST (National Institute of Standards
and Technology) and CCCS (Canadian Centre for Cyber Security) through a
programme called the **CMVP** (Cryptographic Module Validation Program).

---

## 2. The Standards Landscape

Understanding FIPS 140-3 requires situating it within a broader set of
standards. They form a hierarchy:

```
┌─────────────────────────────────────────────────────────────┐
│  ISO/IEC 19790:2012                                         │
│  Security requirements for cryptographic modules            │
│  (the international standard — FIPS 140-3 adopts this)     │
└───────────────────────────┬─────────────────────────────────┘
                            │ adopted by
┌───────────────────────────▼─────────────────────────────────┐
│  FIPS 140-3                                                 │
│  US/Canada federal standard for cryptographic modules       │
│  Defines Security Levels 1–4                                │
└──────┬──────────────────────────┬───────────────────────────┘
       │ implemented via          │ algorithm requirements via
┌──────▼──────────┐    ┌──────────▼──────────────────────────┐
│  CMVP           │    │  Algorithm standards                 │
│  The validation │    │  FIPS 197 (AES)                     │
│  programme      │    │  FIPS 180-4 (SHA)                   │
│                 │    │  FIPS 198-1 (HMAC)                  │
│                 │    │  SP 800-132 (PBKDF2)                │
│                 │    │  SP 800-90A (DRBG)                  │
└──────┬──────────┘    └─────────────────────────────────────┘
       │ algorithm validation via
┌──────▼──────────┐
│  CAVP           │
│  Algorithm      │
│  Validation     │
│  Programme      │
└──────┬──────────┘
       │ conducted using
┌──────▼──────────┐
│  ACVP           │
│  Automated      │
│  Crypto         │
│  Validation     │
│  Protocol       │
└─────────────────┘
```

### Security Levels

FIPS 140-3 defines four security levels. Each level adds requirements on
top of the previous:

| Level | Key characteristics |
|---|---|
| 1 | Software-only modules. Basic algorithm correctness and self-tests. No physical security required. |
| 2 | Adds tamper-evidence requirements (seals, coatings). Role-based authentication. |
| 3 | Adds tamper-resistance and identity-based authentication. Key zeroization on tamper detection. |
| 4 | Highest physical security. Complete envelope protection. Environmental failure protection. |

**This project targets Security Level 1** — the appropriate level for a
pure software cryptographic library with no hardware security requirements.

---

## 3. The CMVP Process — End to End

A vendor seeking a FIPS 140-3 certificate follows this process:

```
┌─────────────────────────────────────────────────────────────┐
│  1. IMPLEMENT                                               │
│     Build the cryptographic module to FIPS 140-3            │
│     requirements. Document the boundary, interfaces,        │
│     algorithms, self-tests, and security policy.            │
└───────────────────────────┬─────────────────────────────────┘
                            │
┌───────────────────────────▼─────────────────────────────────┐
│  2. ALGORITHM VALIDATION (CAVP)                             │
│     Submit each approved algorithm to NIST's ACVP server.   │
│     Receive a CAVP certificate number for each algorithm.   │
│     CAVP certificates are a prerequisite for CMVP.          │
└───────────────────────────┬─────────────────────────────────┘
                            │
┌───────────────────────────▼─────────────────────────────────┐
│  3. ENGAGE A CSTL                                           │
│     Select an accredited Cryptographic Security Testing     │
│     Laboratory. The CSTL tests the module independently     │
│     and prepares the validation report.                     │
└───────────────────────────┬─────────────────────────────────┘
                            │
┌───────────────────────────▼─────────────────────────────────┐
│  4. CSTL TESTING                                            │
│     The CSTL validates: algorithm correctness, self-tests,  │
│     key zeroization, security policy accuracy, boundary     │
│     definition, and all SP 800-140Br1 documentation         │
│     requirements.                                           │
└───────────────────────────┬─────────────────────────────────┘
                            │
┌───────────────────────────▼─────────────────────────────────┐
│  5. NIST REVIEW                                             │
│     CSTL submits the validation package to NIST/CCCS.       │
│     NIST reviews the package. This phase currently takes    │
│     12–24 months due to queue length.                       │
└───────────────────────────┬─────────────────────────────────┘
                            │
┌───────────────────────────▼─────────────────────────────────┐
│  6. CERTIFICATE ISSUED                                      │
│     NIST publishes the certificate on the CMVP website.     │
│     The certificate lists: module name, version, vendor,    │
│     security level, validated algorithms, and the           │
│     non-proprietary security policy document.               │
└─────────────────────────────────────────────────────────────┘
```

### The Security Policy

A central deliverable of the CMVP process is the **Non-Proprietary Security
Policy** — a public document that describes the module's cryptographic
boundary, approved algorithms, roles and services, self-tests, and key
management. It must follow the structure defined by SP 800-140Br1 and
ISO/IEC 19790:2012 Annex B (sections B.2.1 through B.2.12).

This document is published by NIST alongside the certificate and is
available to anyone who wants to understand what a certified module does
and does not provide.

This project's security policy lives at `docs/security-policy.md` and
follows the SP 800-140Br1 structure throughout.

---

## 4. Algorithm Validation — CAVP and ACVP

### 4.1 What algorithm validation proves

Algorithm validation answers a specific, narrow question: **does this
implementation of algorithm X produce the correct output for a given input?**

It does not answer: is the implementation secure against side-channel
attacks? Is the surrounding code free of memory safety bugs? Is the module
correctly integrated into its operational environment?

Those questions are addressed by the broader CMVP process. Algorithm
validation is a necessary but not sufficient condition for a CMVP certificate.

### 4.2 CAVP — the programme

The **CAVP** (Cryptographic Algorithm Validation Program) is the NIST
programme that governs algorithm testing. Every approved algorithm used in
a FIPS 140-3 module must have a CAVP certificate before the module can
receive a CMVP certificate.

CAVP certificates are algorithm-specific, not module-specific. A single
CAVP certificate for AES can be referenced by multiple CMVP submissions.

### 4.3 ACVP — the protocol

**ACVP** (Automated Cryptographic Validation Protocol) is the JSON-over-HTTPS
protocol used to conduct CAVP testing. It replaced the older CAVS
(Cryptographic Algorithm Validation System) file-based process.

The exchange works as follows:

```
┌─────────────────────┐         ┌──────────────────────────┐
│   Your client       │         │   NIST ACVTS server      │
│   (implementation   │         │   (holds expected        │
│    under test)      │         │    values privately)     │
└────────┬────────────┘         └──────────┬───────────────┘
         │                                 │
         │  1. Register capabilities       │
         │ ──────────────────────────────► │
         │                                 │
         │  2. Receive test vectors        │
         │ ◄────────────────────────────── │
         │     (inputs only, no expected   │
         │      values)                    │
         │                                 │
         │  3. Run implementation          │
         │     (local — no network)        │
         │                                 │
         │  4. Submit outputs              │
         │ ──────────────────────────────► │
         │                                 │
         │  5. Receive pass/fail           │
         │ ◄────────────────────────────── │
         │                                 │
```

The client never sees the expected values — the server holds them privately
and performs the comparison. This is a deliberate design: if the client knew
the expected outputs, it could produce correct responses without a correct
implementation.

### 4.4 Test types

ACVP defines two primary test types:

| Type | Name | Description |
|---|---|---|
| AFT | Algorithm Functional Test | Fixed inputs → verify outputs. The basic correctness test. |
| MCT | Monte Carlo Test | Iterative chaining of algorithm output back into input. Tests internal state consistency. |

This project implements **AFT only**. MCT is deferred.

### 4.5 The validation confidence chain

Not all algorithm validation evidence carries the same weight:

| Level | Method | Who holds expected values | Strength |
|---|---|---|---|
| 1 | Local simulation against a reference library | Your own reference (e.g. Python `cryptography`) | Good — reference has CAVP lineage |
| 2 | NIST ACVP demo server | NIST (non-authoritative sandbox) | Stronger — NIST-authoritative vectors |
| 3 | NIST ACVP production server via CSTL | NIST (authoritative) | Definitive — CAVP certificate issued |

This project achieves Level 1 (Sprint 8) and targets Level 2 (Sprint 9).
Level 3 requires an accredited CSTL.

---

## 5. The Role of a CSTL

A **CSTL** (Cryptographic Security Testing Laboratory) is an independent
laboratory accredited by NIST's NVLAP (National Voluntary Laboratory
Accreditation Program) to test cryptographic modules against FIPS 140-3.

CSTLs are the gatekeepers of the CMVP process. A vendor cannot submit
directly to NIST — all submissions must go through an accredited CSTL.

What a CSTL does:

- Reviews the vendor's module implementation and documentation
- Conducts independent testing of all claimed algorithms via ACVP
- Tests self-tests, zeroization, boundary definition, and role enforcement
- Prepares the **Validation Test Report (VTR)** — the primary submission
  artifact reviewed by NIST
- Submits the VTR and supporting documentation to NIST on the vendor's behalf
- Works with the vendor to resolve any findings before NIST review

Well-known CSTLs include Leidos, atsec Information Security, UL, and
Lightship Security. CSTL testing typically costs $20,000–$50,000 and the
full process (CSTL testing + NIST review) takes 12–24 months.

---

## 6. What This Project Implements

This project is a learning implementation of a FIPS 140-3 Security Level 1
software cryptographic module. It implements the components of the CMVP
process that can be meaningfully explored without an accredited lab.

### Cryptographic boundary

The module boundary is a logical boundary encompassing six compiled object
files:

```
╔══════════════════════════════════════════════════════════════╗
║  fips-crypto — Cryptographic Boundary                       ║
║                                                             ║
║  ┌──────────┐  ┌──────────┐  ┌──────────────────────────┐  ║
║  │  aes.c   │  │ sha256.c │  │        hmac.c            │  ║
║  └──────────┘  └──────────┘  └──────────────────────────┘  ║
║  ┌──────────┐  ┌──────────┐  ┌──────────────────────────┐  ║
║  │ pbkdf2.c │  │  fips.c  │  │      self_test.c         │  ║
║  └──────────┘  └──────────┘  └──────────────────────────┘  ║
╚══════════════════════════════════════════════════════════════╝
```

### Approved algorithms

| Algorithm | Standard | Key/Output Size |
|---|---|---|
| AES-CBC | FIPS 197, SP 800-38A | 128-bit, 256-bit keys |
| SHA-256 | FIPS 180-4 | 256-bit digest |
| HMAC-SHA-256 | FIPS 198-1 | Key ≥ 112 bits |
| PBKDF2-HMAC-SHA-256 | SP 800-132 | Variable, iterations ≥ 1000 |

### FIPS mode enforcement

The module implements a FIPS mode flag (`fips_mode_enable()`) that blocks
calls to non-approved algorithms at runtime. Any call to MD5, SHA-1, DES,
RC4, or Blowfish returns `FIPS_ERR_NOT_APPROVED` when FIPS mode is active.

### Self-tests

The module implements Known Answer Tests (KATs) for AES-CBC, SHA-256, and
HMAC-SHA-256 that run when `fips_self_test_run()` is called. If any KAT
fails, the module sets an error state and all subsequent cryptographic
operations fail until the module is restarted.

### ACVP simulation

All four approved algorithms have been validated against Python's
`cryptography` library (which has CAVP lineage via OpenSSL/BoringSSL) using
ACVP-format JSON vectors. 43/43 vectors pass. The pipeline is reproducible
via `make acvp-test`. Demo server submission is planned for Sprint 9.

### Security policy

The module's non-proprietary security policy (`docs/security-policy.md`)
follows the SP 800-140Br1 Annex B structure (B.2.1–B.2.12), documents all
known gaps, and would serve as the starting point for a real CSTL engagement.

---

## 7. What This Project Is Not

Being precise about limitations is as important as documenting capabilities.

| Claim | Reality |
|---|---|
| This is a production cryptographic library | It is not. It is a learning project. Do not use it to protect real data. |
| This module is FIPS 140-3 certified | It is not. No CSTL has tested it. No NIST certificate has been issued. |
| This module has CAVP certificates | It does not. ACVP local simulation is not equivalent to a CAVP certificate. |
| The self-tests meet FIPS 140-3 §10.3.1 | Partially. The software integrity test (HMAC over the binary) is not implemented (Gap #1). |
| Zeroization is guaranteed | Not fully. `memset` is used rather than `explicit_bzero` or `memset_s`, which the C standard does not guarantee will not be optimised away (Gap #6). |
| The implementation is timing-attack resistant | It is not. AES table lookups and MAC comparison are not constant-time (Gap #12). |
| A DRBG is implemented | It is not. Keys must be supplied by the caller (Gap #2). |

These gaps are intentional limitations of a learning project, not oversights.
They are fully documented in `docs/security-policy.md` Appendix B.

---

## 8. Sprint-by-Sprint Traceability

Each sprint in this project corresponds to one or more requirements from
FIPS 140-3 or its supporting standards. This table makes that mapping
explicit.

| Sprint | Deliverable | FIPS 140-3 / CMVP Requirement |
|---|---|---|
| Sprint 0 | Repo structure, Makefile, README | Lifecycle assurance — SP 800-140Br1 B.2.11 |
| Sprint 1 | AES-128/256-CBC | Approved algorithm — FIPS 197, SP 800-38A |
| Sprint 2 | SHA-256 | Approved algorithm — FIPS 180-4 |
| Sprint 3 | HMAC-SHA-256 | Approved algorithm — FIPS 198-1 |
| Sprint 4 | FIPS mode flag + algorithm blocking | Approved services — FIPS 140-3 §7.4, §7.6 |
| Sprint 5 | Self-tests / KATs | Power-on self-tests — FIPS 140-3 §10.3.1 |
| Sprint 6 | PBKDF2-HMAC-SHA-256 | Approved KDF — SP 800-132 |
| Sprint 7 | Security policy, boundary, inventory | SP 800-140Br1 B.2.1–B.2.12; FIPS 140-3 §7.1 |
| Fixing Sprint | `fips_internal.h`, CSP access hardening | CSP protection — FIPS 140-3 §7.9 |
| NSP Alignment | Security policy restructured to B.2.1–B.2.12 | SP 800-140Br1 Annex B full alignment |
| Sprint 8 | ACVP simulation pipeline | CAVP prerequisite — CMVP requirement |
| Sprint 9 | ACVP demo server submission | CAVP prerequisite — NIST-authoritative validation |

---

## 9. What Would Be Needed to Go Further

The following work would be required to move from this learning project
toward a real CMVP submission:

### Technical gaps to close

| Gap | Effort | Reference |
|---|---|---|
| Software integrity test (HMAC over module binary at startup) | Medium | FIPS 140-3 §10.3.1 |
| Replace `memset` with `explicit_bzero` / `memset_s` | Low | FIPS 140-3 §7.9 |
| Enforce HMAC minimum key length (112 bits) at API level | Low | FIPS 198-1 |
| Implement CTR_DRBG or HMAC_DRBG | High | SP 800-90A Rev 1 |
| Constant-time AES and MAC comparison | Medium | Practical security |
| ECDSA and ECDH (P-256 minimum) | Very high | FIPS 186-5 |

### Process requirements

| Requirement | What it involves |
|---|---|
| CAVP certificates | Submit to NIST ACVP production server via CSTL |
| Engage a CSTL | Select an NVLAP-accredited lab; budget $20,000–$50,000 |
| Formal CM plan | Branch protection, access controls, formal change control |
| Secure delivery procedure | Signed releases, published checksums |
| NIST review queue | Currently 12–24 months after CSTL submission |

### The honest gap

The architectural and documentary foundation this project establishes is
the same foundation a real submission would build on. The security policy
structure, boundary definition, algorithm inventory, and ACVP pipeline are
all aligned with what a CSTL would expect to see. What is missing is the
accredited lab engagement and the production ACVP submission — both of which
require resources beyond a learning project.

---

## 10. Glossary

| Term | Definition |
|---|---|
| **AFT** | Algorithm Functional Test. Fixed-input ACVP test type that verifies correct output for given inputs. |
| **ACVP** | Automated Cryptographic Validation Protocol. JSON-over-HTTPS protocol used to conduct CAVP algorithm testing. |
| **ACVTS** | Automated Cryptographic Validation Testing System. NIST's server infrastructure that implements ACVP. |
| **CAVP** | Cryptographic Algorithm Validation Program. NIST programme that validates individual cryptographic algorithms and issues algorithm certificates. |
| **CAVS** | Cryptographic Algorithm Validation System. The legacy file-based system ACVP replaced. No longer accepted. |
| **CCCS** | Canadian Centre for Cyber Security. Co-administers CMVP with NIST for Canadian federal requirements. |
| **CMVP** | Cryptographic Module Validation Program. The NIST/CCCS programme that validates complete cryptographic modules and issues module certificates. |
| **CSP** | Critical Security Parameter. Secret information (keys, passwords) whose disclosure or modification compromises a module's security. |
| **CSTL** | Cryptographic Security Testing Laboratory. An NVLAP-accredited lab authorised to test modules for FIPS 140-3 compliance. |
| **DRBG** | Deterministic Random Bit Generator. A FIPS-approved pseudorandom number generator. Required for internal key generation. |
| **FIPS** | Federal Information Processing Standard. US government standards published by NIST. |
| **FIPS 140-3** | The current standard for cryptographic module security requirements, adopted from ISO/IEC 19790:2012. |
| **KAT** | Known Answer Test. A self-test that verifies an algorithm produces a known correct output for a fixed input. |
| **MCT** | Monte Carlo Test. An ACVP test type that chains algorithm output back into input iteratively to test internal state. |
| **mTLS** | Mutual TLS. A TLS configuration where both client and server present certificates, used by ACVTS for client authentication. |
| **NVLAP** | National Voluntary Laboratory Accreditation Program. NIST programme that accredits CSTLs. |
| **POST** | Power-On Self-Test. Self-tests that run at module initialisation to verify correct operation before any cryptographic services are provided. |
| **PSP** | Public Security Parameter. Non-secret information whose integrity is important to module security. |
| **Security Level** | FIPS 140-3 defines four security levels (1–4) with increasing physical and logical security requirements. |
| **SP 800-140Br1** | NIST Special Publication defining security policy requirements for CMVP submissions. |
| **TOTP** | Time-based One-Time Password (RFC 6238). Two-factor authentication mechanism used by ACVTS. |
| **VTR** | Validation Test Report. The primary artifact a CSTL submits to NIST as part of a CMVP submission. |

---

## 11. References

| Document | Title | URL |
|---|---|---|
| FIPS 140-3 | Security Requirements for Cryptographic Modules | https://csrc.nist.gov/publications/detail/fips/140/3/final |
| ISO/IEC 19790:2012 | Security requirements for cryptographic modules | https://www.iso.org/standard/52906.html |
| FIPS 197 | Advanced Encryption Standard | https://csrc.nist.gov/publications/detail/fips/197/final |
| FIPS 180-4 | Secure Hash Standard | https://csrc.nist.gov/publications/detail/fips/180/4/final |
| FIPS 198-1 | The Keyed-Hash Message Authentication Code | https://csrc.nist.gov/publications/detail/fips/198/1/final |
| FIPS 186-5 | Digital Signature Standard | https://csrc.nist.gov/publications/detail/fips/186/5/final |
| SP 800-38A | Block Cipher Modes of Operation | https://csrc.nist.gov/publications/detail/sp/800/38a/final |
| SP 800-90A Rev 1 | Recommendation for Random Number Generation Using DRBGs | https://csrc.nist.gov/publications/detail/sp/800/90a/rev-1/final |
| SP 800-131A Rev 2 | Transitioning Cryptographic Algorithms and Key Lengths | https://csrc.nist.gov/publications/detail/sp/800/131a/rev-2/final |
| SP 800-132 | Recommendation for Password-Based Key Derivation | https://csrc.nist.gov/publications/detail/sp/800/132/final |
| SP 800-140Br1 | CMVP Security Policy Requirements | https://csrc.nist.gov/publications/detail/sp/800/140b/r1/final |
| ACVP Specification | Automated Cryptographic Validation Protocol | https://pages.nist.gov/ACVP/ |
| CMVP Website | Active FIPS 140-3 certificates | https://csrc.nist.gov/projects/cryptographic-module-validation-program |
| NVLAP CSTL List | Accredited testing laboratories | https://www.nist.gov/nvlap/nvlap-accredited-laboratories |
