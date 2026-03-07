# FIPS 140-3 Certification Learning Project

> **Goal:** Learn the FIPS 140-3 certification process from a practitioner's point of view by taking an open-source cryptography library through a simulated (but technically rigorous) end-to-end certification process. The repo serves as evidence of the journey.

---

## Target Package

**Python `cryptography` library (PyCA)**
- Repo: https://github.com/pyca/cryptography
- Status: NOT FIPS 140-3 certified (the library itself; it can use a certified OpenSSL backend but that's a different thing)
- Why chosen: Widely used in production, wraps C extensions (libssl/libcrypto), has real algorithm implementations to test, and is complex enough to be realistic

---

## Approach: Thin Slices, End-to-End

Agile, de-risked iterations. Each sprint produces a real artifact that goes into the repo. No big up-front design. Each iteration is independently valuable and builds on the last.

**Cost policy:** Any step that requires real money (lab fees, NIST submission) is simulated with documented fidelity. The simulation is explicitly labeled so the record is honest.

---

## Repository Structure

```
fips-certification-learning/
├── plan.md                        ← this file (living document)
├── README.md                      ← GitHub-friendly intro
├── module-definition/
│   ├── boundary.md                ← cryptographic module boundary definition
│   └── algorithm-inventory.md     ← approved vs non-approved algorithms
├── security-policy/
│   └── security-policy.md         ← FIPS 140-3 Security Policy (draft)
├── acvts-testing/
│   ├── README.md                  ← how ACVTS works
│   ├── aes/
│   │   ├── test-vectors.json      ← NIST-style vectors
│   │   ├── run_tests.py           ← test harness
│   │   └── results.json          ← pass/fail results
│   ├── sha2/
│   ├── sha3/
│   ├── hmac/
│   ├── rsa/
│   └── ecdsa/
├── lab-simulation/
│   ├── checklist.md               ← CMVP lab review checklist (simulated)
│   └── lab-report.md              ← simulated lab findings report
└── certificate-simulation/
    └── certificate.md             ← mock CMVP validation certificate
```

---

## Iterations

### Sprint 0 — Repo & Plan ✅ IN PROGRESS
**Goal:** Establish the repo, this plan, and a working README. Prove the skeleton end-to-end.

Deliverables:
- `plan.md` (this file)
- `README.md` with project overview
- Empty folder structure with placeholder files

Done when: Repo exists, plan is committed, structure is navigable.

---

### Sprint 1 — Module Boundary & Algorithm Inventory
**Goal:** Define exactly *what* is being certified. This is the first real FIPS task a practitioner does and the most commonly underestimated.

Deliverables:
- `module-definition/boundary.md` — defines the cryptographic boundary (what's in, what's out, what the physical/logical perimeter is for a software module)
- `module-definition/algorithm-inventory.md` — full table of algorithms used by the library, each tagged: Approved / Non-Approved / Allowed (conditional)

Key concepts practiced:
- Software module boundary definition (FIPS 140-3 §7.5)
- Approved Security Functions (per NIST SP 800-140C)
- Non-approved algorithm handling (must be disabled or indicator required)

Done when: A reviewer could read both docs and know exactly what is and isn't in scope.

---

### Sprint 2 — First Algorithm Test: AES (Thin ACVTS Slice)
**Goal:** Run the first real cryptographic validation test. Prove the test harness works end-to-end before adding more algorithms.

Deliverables:
- `acvts-testing/README.md` — explains ACVTS, how vector-based testing works, what KAT/MCT/AFT mean
- `acvts-testing/aes/test-vectors.json` — NIST ACVTS-format vectors (KAT: Known Answer Test, MCT: Monte Carlo Test)
- `acvts-testing/aes/run_tests.py` — harness that feeds vectors to the `cryptography` library and captures output
- `acvts-testing/aes/results.json` — actual pass/fail output with comparison

Key concepts practiced:
- ACVTS (Automated Cryptographic Validation Testing System)
- KAT vs MCT vs AFT test types
- How NIST vector format works (prompt/expected response JSON)
- AES-CBC, AES-GCM modes

Done when: Script runs, results.json shows pass/fail per vector, at least one deliberate failure is demonstrated and explained.

---

### Sprint 3 — Expand Testing: SHA-2, SHA-3, HMAC
**Goal:** Repeat the ACVTS pattern for hash and MAC algorithms. Build confidence in the harness.

Deliverables:
- Same structure as Sprint 2, repeated for: SHA-256, SHA-384, SHA-512, SHA3-256, SHA3-512, HMAC-SHA-256
- `acvts-testing/sha2/`, `acvts-testing/sha3/`, `acvts-testing/hmac/`

Key concepts practiced:
- Hash algorithm validation nuances (message length, bit-oriented vs byte-oriented)
- HMAC key length requirements
- How the ACVTS groups algorithms into "algorithm capabilities"

Done when: All three algorithm families have test vectors, harness, and results.

---

### Sprint 4 — Asymmetric: RSA and ECDSA
**Goal:** Test the most complex algorithms. Asymmetric crypto has stricter FIPS constraints (key sizes, curves, padding modes).

Deliverables:
- `acvts-testing/rsa/` — RSA keygen, sign, verify (PKCS#1 v1.5 and PSS, approved key sizes only: 2048, 3072, 4096)
- `acvts-testing/ecdsa/` — ECDSA keygen, sign, verify (P-256, P-384, P-521 only)

Key concepts practiced:
- Why RSA-1024 is not approved and how to handle its presence in the library
- Approved curves vs non-approved curves (e.g., secp256k1 is NOT approved)
- Randomness requirements for key generation (DRBG dependency)

Done when: Both families tested, non-approved variants explicitly documented as out-of-scope.

---

### Sprint 5 — Security Policy Document (Draft)
**Goal:** Write the mandatory public-facing Security Policy. This is the document NIST publishes on the CMVP website for every validated module.

Deliverables:
- `security-policy/security-policy.md` — structured per FIPS 140-3 Annex A requirements

Required sections (per standard):
1. Module Overview
2. Module Boundary and Interfaces
3. Approved Security Functions
4. Non-Approved but Allowed Security Functions
5. Non-Approved Security Functions (prohibited in FIPS mode)
6. Roles, Services, and Authentication
7. Physical Security (N/A for software — documented as such)
8. Operational Environment
9. Key and CSP Management
10. Self-Tests (power-up, conditional)
11. Mitigation of Other Attacks

Key concepts practiced:
- What a real Security Policy looks like (read any on CMVP website for comparison)
- Roles: Crypto Officer vs User
- CSP (Critical Security Parameters) — keys, passwords, seeds
- Self-test requirements: KAT on power-up, pairwise consistency test on key gen

Done when: Document matches FIPS 140-3 Annex A structure; a knowledgeable reader could use it as a real submission draft.

---

### Sprint 6 — Lab Simulation (Simulated — Labeled)
**Goal:** Simulate the CMVP accredited lab review process. In reality this costs $50K–$200K and takes months. We document what a lab *would* do.

Deliverables:
- `lab-simulation/checklist.md` — the review checklist a CMVP lab uses (derived from NIST SP 800-140 series and ISO/IEC 19790)
- `lab-simulation/lab-report.md` — a simulated lab findings report: findings, observations, and a pass/fail recommendation

> ⚠️ **SIMULATION NOTE:** This step represents a ~$100K engagement with an accredited CMVP testing lab (e.g., Leidos, atsec, UL, Acumen Security). The checklist and report are modeled on real lab artifacts for learning purposes only.

Key concepts practiced:
- What labs actually check beyond algorithm testing (documentation, entropy sources, self-tests, error states)
- Common findings that cause submissions to fail
- The difference between a lab *observation* and a lab *finding*

Done when: Checklist covers all FIPS 140-3 requirement areas; report references specific sections of the standard.

---

### Sprint 7 — Mock Certificate (Simulated — Labeled)
**Goal:** Produce the final artifact: a mock CMVP validation certificate, matching the format of real certificates on the NIST website.

Deliverables:
- `certificate-simulation/certificate.md` — mock certificate in CMVP format

> ⚠️ **SIMULATION NOTE:** Real NIST CMVP review takes 12–24 months and results in a public entry at https://csrc.nist.gov/projects/cryptographic-module-validation-program. This is a simulated equivalent for learning purposes.

Certificate will include:
- Module name and version
- Certificate number (mock)
- Validation date
- Security Level
- Approved algorithms with CAVP certificate references (mock)
- Operational environment
- Tested configuration

Done when: Certificate is indistinguishable in structure from a real CMVP certificate.

---

## Key Reference Documents

| Document | What it is |
|---|---|
| FIPS 140-3 | The standard itself (superseded FIPS 140-2 in 2019) |
| ISO/IEC 19790:2012 | The international standard FIPS 140-3 is based on |
| NIST SP 800-140 | FIPS 140-3 derived test requirements (parent) |
| NIST SP 800-140B | FIPS 140-3 Derived Test Requirements: Vendor Evidence |
| NIST SP 800-140C | Approved Security Functions |
| NIST SP 800-140D | Approved Sensitive Security Parameter Generation and Establishment Methods |
| NIST SP 800-140E | Approved Authentication Mechanisms |
| NIST SP 800-140F | Approved Non-Invasive Attack Mitigation Test Metrics |
| NIST SP 800-90A | Deterministic Random Bit Generators (DRBG) |

All freely available at https://csrc.nist.gov

---

## Glossary

| Term | Meaning |
|---|---|
| FIPS | Federal Information Processing Standard |
| CMVP | Cryptographic Module Validation Program (run by NIST + CCCS) |
| ACVTS | Automated Cryptographic Validation Testing System |
| CAVP | Cryptographic Algorithm Validation Program (predecessor, now part of ACVTS) |
| KAT | Known Answer Test — fixed input, compare to known output |
| MCT | Monte Carlo Test — iterative chaining test for statistical properties |
| AFT | Functional Test — tests a range of valid inputs |
| CSP | Critical Security Parameter — secret data (keys, seeds, passwords) |
| PSP | Public Security Parameter — non-secret data (public keys, certificates) |
| SP | Security Policy — the mandatory public document |
| Security Level | 1–4 rating; most software targets Level 1 |
| Module Boundary | The defined perimeter of what is being validated |

---

## Status Tracker

| Sprint | Title | Status |
|---|---|---|
| 0 | Repo & Plan | 🟡 In Progress |
| 1 | Module Boundary & Algorithm Inventory | ⬜ Not Started |
| 2 | AES Testing (thin ACVTS slice) | ⬜ Not Started |
| 3 | SHA-2, SHA-3, HMAC Testing | ⬜ Not Started |
| 4 | RSA and ECDSA Testing | ⬜ Not Started |
| 5 | Security Policy Document | ⬜ Not Started |
| 6 | Lab Simulation | ⬜ Not Started |
| 7 | Mock Certificate | ⬜ Not Started |

---

## Session Notes

*Use this section to log decisions, discoveries, and deferred items across AI sessions.*

- **2026-03-07** — Project initiated. Chose PyCA `cryptography` library as target. Decided on thin-slice agile approach. plan.md created as persistent memory anchor.
