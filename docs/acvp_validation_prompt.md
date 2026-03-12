# ACVP Validation Infrastructure — Gemini Prompt & Design Reference

**File:** `docs/acvp_validation_prompt.md`
**Purpose:** Prompt used to generate `acvp/scripts/generate_vectors.py` and
`acvp/scripts/validate_responses.py` via Gemini. Kept as a permanent record
of the design decisions, schema conventions, and constraints that govern the
validation infrastructure.

---

## Context

This project is a minimal FIPS 140-3 cryptographic library written in ANSI C.
The library implements AES-128/256-CBC, SHA-256, HMAC-SHA-256, and PBKDF2.
Sprints 0–7 are complete: 54/54 unit tests passing, security policy and
algorithm inventory documents in place.

Sprint 8 introduces ACVP (Automated Cryptographic Validation Protocol)
simulation — a local, offline pipeline that produces and validates
ACVP-format JSON. The goal is to simulate the full ACVP exchange locally
first, then replay it against NIST's demo server (`demo.acvts.nist.gov`) in
Sprint 9.

**Terminology:**

| Term  | Meaning |
|-------|---------|
| CAVP  | Cryptographic Algorithm Validation Program — the NIST program |
| ACVP  | Automated Cryptographic Validation Protocol — the JSON protocol |
| ACVTS | Automated Cryptographic Validation Testing System — NIST's server |
| AFT   | Algorithm Functional Test — the test type used here (not MCT) |

---

## What You Need to Produce

Two Python scripts only. Do not write any C code, Makefiles, or cryptographic
implementations.

### Script 1: `acvp/scripts/generate_vectors.py` — Stage 1

- Uses the Python `cryptography` library as the **sole trusted reference**
- Generates random inputs via `os.urandom()` — no hardcoded vectors
- Writes one ACVP-format request JSON file per algorithm to `acvp/request/`
- Embeds a local `"expected"` field in each test case (see schema extension below)
- Runnable standalone from repo root: `python3 acvp/scripts/generate_vectors.py`
- Prints clear progress output showing which files were written and how many
  vectors each contains

### Script 2: `acvp/scripts/validate_responses.py` — Stage 3

- Reads each response JSON from `acvp/response/` (written by the C runner)
- Reads the corresponding request JSON from `acvp/request/` to retrieve the
  `"expected"` ground truth
- Performs an **independent comparison** of `response["output"]` vs
  `request["expected"]` — do NOT trust the `"passed"` field written by the
  C runner alone; always recheck independently
- Prints a per-test-case pass/fail table and a final summary in this format:

```
ACVP-AES-CBC:          8/8   passed
ACVP-SHA2-256:         11/11 passed
ACVP-HMAC-SHA2-256:    12/12 passed
ACVP-KDF-PBKDF2:       12/12 passed
──────────────────────────────────────
TOTAL:                 43/43 passed
```

- Exits with code 0 if all pass, non-zero if any fail
- Runnable standalone from repo root: `python3 acvp/scripts/validate_responses.py`

---

## Repo Layout (Relevant Parts)

```
fips-certification/
├── acvp/
│   ├── Makefile                   ← not your concern
│   ├── request/                   ← generate_vectors.py writes here
│   │   ├── aes_cbc.json
│   │   ├── sha256.json
│   │   ├── hmac_sha256.json
│   │   └── pbkdf2.json
│   ├── response/                  ← C runner writes here; validate_responses.py reads here
│   │   └── .gitkeep
│   └── scripts/
│       ├── generate_vectors.py    ← YOU WRITE THIS
│       └── validate_responses.py  ← YOU WRITE THIS
├── src/                           ← C library source (not your concern)
└── docs/
    └── acvp_validation_prompt.md  ← this file
```

All paths must be resolved relative to repo root, not relative to the script's
own location. Scripts live two levels below the repo root
(`acvp/scripts/`), so repo root = `Path(__file__).resolve().parents[2]`.

---

## Algorithm Specifications

### 1. ACVP-AES-CBC — `acvp/request/aes_cbc.json`

**Algorithm string:** `"ACVP-AES-CBC"`
**vsId:** 1

Two testGroups:

| tgId | direction | keyLen |
|------|-----------|--------|
| 1    | encrypt   | 256    |
| 2    | decrypt   | 256    |

- 4 test cases per group (8 total)
- Per test: random key (32 bytes), random IV (16 bytes), random plaintext
- **Plaintext length must be an exact multiple of 16 bytes.** The C
  implementation does not perform PKCS#7 padding — misaligned input will
  produce wrong output, not an error.
- For encrypt groups: input field is `"pt"`, expected is the ciphertext
- For decrypt groups: input field is `"ct"` (encrypt a random plaintext first,
  use the ciphertext as input), expected is the original plaintext
- `"keyLen"` in JSON is in **bits** (value: 256)
- All byte values as lowercase hex strings

Example test case (encrypt):
```json
{
  "tcId": 1,
  "key":      "a3f1c2...",
  "iv":       "00c1d2...",
  "pt":       "6bc1bee2...",
  "expected": "3ad77bb4..."
}
```

---

### 2. ACVP-SHA2-256 — `acvp/request/sha256.json`

**Algorithm string:** `"ACVP-SHA2-256"`
**vsId:** 2

Single testGroup (`tgId=1`), `"testType": "AFT"`.

11 test cases across three categories:

| Category              | Count | Length       |
|-----------------------|-------|--------------|
| Empty message         | 1     | 0 bytes      |
| Short messages        | 5     | 1–63 bytes   |
| Block boundary        | 5     | exactly 64 bytes |

- `"len"` field is message length in **bits** (e.g. 64 bytes → `"len": 512`)
- `"msg"` field is lowercase hex (empty string `""` for the zero-length case)
- `"expected"` is the SHA-256 digest as lowercase hex

Example test case (empty message):
```json
{ "tcId": 1, "len": 0, "msg": "", "expected": "e3b0c44998fc1c14..." }
```

---

### 3. ACVP-HMAC-SHA2-256 — `acvp/request/hmac_sha256.json`

**Algorithm string:** `"ACVP-HMAC-SHA2-256"`
**vsId:** 3

Three testGroups:

| tgId | keyLen (bytes) | macLen (bytes) | Notes             |
|------|----------------|----------------|-------------------|
| 1    | 16             | 16             | Truncated output  |
| 2    | 32             | 32             | Full output       |
| 3    | 64             | 32             | Oversized key     |

- 4 test cases per group (12 total)
- Per test: random key (`keyLen` bytes), random message (16–128 bytes)
- `"keyLen"` and `"macLen"` in JSON are in **bits**
- `"expected"` is the first `macLen` bytes of the full HMAC-SHA-256 output,
  as lowercase hex (truncation for tgId=1)
- All byte values as lowercase hex strings

---

### 4. ACVP-KDF-PBKDF2 — `acvp/request/pbkdf2.json`

**Algorithm string:** `"ACVP-KDF-PBKDF2"`
**vsId:** 4

Single testGroup (`tgId=1`), 12 test cases.

Coverage: 4 repetitions × 3 key lengths (16, 32, 64 bytes), interleaved so
all three key lengths appear evenly.

Fixed parameters:
- `iterationCount`: 10000
- `"hashAlg"`: `"SHA2-256"`

**CRITICAL schema conventions for PBKDF2 — these differ from other algorithms:**

- `"password"` is an **ASCII string** (not hex) — random printable ASCII,
  8–32 characters
- `"salt"` is an **ASCII string** (not hex) — random printable ASCII,
  8–32 characters
- `"keyLen"` is in **bits** (128, 256, or 512 for 16, 32, 64 bytes)

The C runner will cast the ASCII string directly to `uint8_t*` and use
`strlen()` for the length. Do not hex-encode password or salt.

Example test case:
```json
{
  "tcId":           1,
  "password":       "mySecretPass",
  "salt":           "randomSalt42",
  "iterationCount": 10000,
  "keyLen":         256,
  "hashAlg":        "SHA2-256",
  "expected":       "a3f1c2..."
}
```

---

## Local Schema Extension — the `"expected"` Field

This is a **local-only addition** to the ACVP schema, not part of the real
ACVP specification. It must be stripped before any submission to
`acvts.nist.gov` in Sprint 9.

Each test case in every request JSON embeds an `"expected"` field containing
the Python `cryptography`-computed reference output as a lowercase hex string.
This carries ground truth from Stage 1 through to Stage 3 without a separate
file.

---

## Response JSON Structure

The C runner (Stage 2) writes this structure. `validate_responses.py` must
be able to parse it:

```json
{
  "vsId": 1,
  "algorithm": "ACVP-AES-CBC",
  "testGroups": [
    {
      "tgId": 1,
      "tests": [
        {
          "tcId":   1,
          "output": "3ad77bb4...",
          "passed": true
        }
      ]
    }
  ]
}
```

The `"passed"` field is written by the C runner based on its own internal
comparison. `validate_responses.py` must **independently recheck** by
comparing `"output"` against `"expected"` from the corresponding request
JSON. A bug in the runner's comparison logic must not produce a silent false
positive in the validation report.

Matching logic for the validator:
1. Load `acvp/request/<algo>.json` — build a map of `tcId → expected`
2. Load `acvp/response/<algo>.json` — iterate testGroups → tests
3. For each test: compare `test["output"].lower()` == `expected.lower()`
4. Record pass/fail independently of `test["passed"]`

---

## Constraints

- Python 3.9+ compatible
- Only external dependency: `cryptography` (`pip install cryptography`)
- No hardcoded test vectors — all inputs from `os.urandom()` or `random`
  seeded from system entropy
- All hex strings lowercase
- JSON files written with `indent=2` and a trailing newline
- Paths resolved relative to repo root (not script location)
- Provide a clear, actionable error message if `cryptography` is not installed
- `validate_responses.py` exits with code 0 if all pass, non-zero otherwise

---

## What You Do NOT Need to Do

- Do not write `acvp_runner.c` or any C code
- Do not write any Makefile
- Do not implement any cryptographic primitives yourself — use the
  `cryptography` library exclusively for all reference computations
- Do not handle MCT (Monte Carlo Test) vectors — AFT only
- Do not add a `--seed` flag for reproducible vectors — that is deferred

---

## Reference: JSON Shape Examples

The following shows the expected top-level structure for each request file.
Use this as a schema reference, not as hardcoded values.

### aes_cbc.json (top-level shape)
```json
{
  "vsId": 1,
  "algorithm": "ACVP-AES-CBC",
  "revision": "1.0",
  "testGroups": [
    {
      "tgId": 1,
      "direction": "encrypt",
      "keyLen": 256,
      "tests": [
        { "tcId": 1, "key": "...", "iv": "...", "pt": "...", "expected": "..." }
      ]
    },
    {
      "tgId": 2,
      "direction": "decrypt",
      "keyLen": 256,
      "tests": [
        { "tcId": 1, "key": "...", "iv": "...", "ct": "...", "expected": "..." }
      ]
    }
  ]
}
```

### sha256.json (top-level shape)
```json
{
  "vsId": 2,
  "algorithm": "ACVP-SHA2-256",
  "revision": "1.0",
  "testGroups": [
    {
      "tgId": 1,
      "testType": "AFT",
      "tests": [
        { "tcId": 1, "len": 0,   "msg": "",    "expected": "e3b0c449..." },
        { "tcId": 2, "len": 48,  "msg": "a3f1c2...", "expected": "..." },
        { "tcId": 7, "len": 512, "msg": "...",  "expected": "..." }
      ]
    }
  ]
}
```

### hmac_sha256.json (top-level shape)
```json
{
  "vsId": 3,
  "algorithm": "ACVP-HMAC-SHA2-256",
  "revision": "1.0",
  "testGroups": [
    { "tgId": 1, "keyLen": 128, "macLen": 128, "tests": [ ... ] },
    { "tgId": 2, "keyLen": 256, "macLen": 256, "tests": [ ... ] },
    { "tgId": 3, "keyLen": 512, "macLen": 256, "tests": [ ... ] }
  ]
}
```

### pbkdf2.json (top-level shape)
```json
{
  "vsId": 4,
  "algorithm": "ACVP-KDF-PBKDF2",
  "revision": "1.0",
  "testGroups": [
    {
      "tgId": 1,
      "tests": [
        {
          "tcId": 1,
          "password":       "asciiStringHere",
          "salt":           "asciiStringHere",
          "iterationCount": 10000,
          "keyLen":         256,
          "hashAlg":        "SHA2-256",
          "expected":       "..."
        }
      ]
    }
  ]
}
```

---

## Decision Log

Decisions made prior to this prompt that must be respected:

| Decision | Choice | Rationale |
|----------|--------|-----------|
| C runner interface | subprocess, JSON files in/out | Maps to real ACVP protocol shape |
| C runner structure | Single binary dispatching on `"algorithm"` | Simpler build, single entry point |
| Mismatch behaviour | Run all vectors, write pass/fail per case | No fail-fast; full coverage report |
| `"expected"` location | Embedded in request JSON | Single file carries ground truth; no separate expected/ dir |
| AES key length | 256-bit only | Matches library's primary target |
| AES directions | Both encrypt and decrypt | Full AFT coverage |
| SHA-256 categories | Empty, short (1–63B), block boundary (64B) | Covers edge cases without MCT |
| HMAC groups | (16,16), (32,32), (64,32) | Truncation, full output, oversized key |
| PBKDF2 iterations | 10000 | Moderate; avoids excessive runtime |
| PBKDF2 key lengths | 16, 32, 64 bytes | Three distinct output sizes |
| PBKDF2 password/salt | ASCII strings, not hex | Matches ACVP schema convention |
| Vector count | 10+ per algorithm | Closer to real ACVP submission density |
| Python harness split | Two separate scripts | Stage 1 and Stage 3 are independently ownable |
| cJSON (C runner) | Homebrew install | Not your concern — C runner only |
| Build system | Separate `acvp/Makefile` | Not your concern — C runner only |

---

## Validation Methodology and Ground Truth Chain

This section documents the epistemological basis of the local simulation —
what it proves, what it does not prove, and how it relates to the broader
CAVP validation process.

### Why Python's `cryptography` library is the reference

The `cryptography` library was chosen as the ground truth reference for the
following reasons:

- It is built on OpenSSL and BoringSSL, both of which hold actual CAVP
  certificates issued by NIST
- It implements the same NIST standards this module targets: FIPS 197,
  FIPS 180-4, FIPS 198-1, SP 800-132
- It is independently audited, widely deployed, and maintained by a
  dedicated security team
- It is not affiliated with this project — its independence from the C
  implementation under test is what makes the comparison meaningful

Agreement between the C library and the `cryptography` library is therefore
meaningful evidence of algorithmic correctness, not an arbitrary comparison
against an unvetted reference.

### The ground truth chain

Validation confidence increases at each level of the following chain. This
project currently sits at Level 1; Sprint 9 targets Level 2.

| Level | Method | Who holds the expected values | Strength |
|---|---|---|---|
| 1 | Local simulation | Python `cryptography` lib (CAVP lineage) | Good |
| 2 | NIST ACVP demo server | NIST | Stronger — authoritative |
| 3 | CMVP certificate | Accredited CSTL + NIST | Definitive |

Level 3 requires an accredited Cryptographic Security Testing Laboratory
(CSTL) and is beyond the scope of this project.

### What local simulation proves and does not prove

| Claim | Supported by local simulation |
|---|---|
| Algorithm logic produces correct outputs | ✅ Yes |
| ACVP JSON schema is correctly implemented | ✅ Yes |
| C library agrees with a CAVP-lineage reference | ✅ Yes |
| Implementation is timing-attack resistant | ❌ No — not tested here |
| Zeroization is guaranteed by the compiler | ❌ No — memset not guaranteed |
| Module binary integrity is verified at runtime | ❌ No — no integrity test |
| Module holds a CAVP certificate | ❌ No — requires demo server + CSTL |

### Independence as a design principle

The validation infrastructure was deliberately authored by a different AI
(Gemini) from the one that authored the C implementation (Claude). This
establishes a separation between the implementation under test and the
validation layer — analogous to the independence requirement between a
module vendor and a CSTL in the real CMVP process.

A bug in Gemini's `validate_responses.py` (incorrect `tcId`-only lookup
across multiple testGroups) was caught during integration because the runner
and validator disagreed (43/43 vs 31/43). This disagreement is exactly what
the two-layer checking was designed to surface. The bug was corrected by the
author before the Sprint 8 commit.

---

**Navigation:** [Documentation Index](README.md) · Previous: [Module Boundary](boundary.md) · Next: [ACVP Integration Workflow](acvp/integration.md)
