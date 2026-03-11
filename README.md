# fips-crypto

A minimal cryptographic library written in ANSI C, built from first principles with FIPS 140-3 constraints in mind.

This project explores the FIPS 140-3 validation process by building a minimal cryptographic module and simulating the steps required for certification. Every primitive is implemented from scratch — no external crypto dependencies. The goal is to understand what FIPS 140-3 actually requires by building something that could plausibly go through certification.

> **Not for production use.** This library is for educational purposes only.

---

## Algorithms

| Algorithm | Status |
|---|---|
| AES-128/256 (CBC) | ✅ Sprint 1 |
| SHA-256 | ✅ Sprint 2 |
| HMAC-SHA-256 | ✅ Sprint 3 |
| FIPS mode + blocking | ✅ Sprint 4 |
| Self-tests / KATs | ✅ Sprint 5 |
| PBKDF2 | ✅ Sprint 6 |
| ACVP simulation (local) | ✅ Sprint 8 |

54/54 unit tests passing. 43/43 ACVP vectors passing.

---

## Structure

```
fips-crypto/
├── src/          ← Algorithm implementations (.c)
├── include/      ← Public headers (.h)
├── tests/        ← Unit test vectors and test runners
├── acvp/         ← ACVP simulation pipeline (Sprints 8+)
│   ├── src/          ← acvp_runner.c (C binary, Stage 2)
│   ├── scripts/      ← generate_vectors.py, validate_responses.py
│   ├── request/      ← ACVP request JSON (runtime-generated, gitignored)
│   └── response/     ← ACVP response JSON (runtime-generated, gitignored)
├── docs/         ← FIPS boundary, algorithm inventory, security policy
├── Makefile
└── README.md
```

---

## Building

```bash
make
```

## Running Unit Tests

```bash
make test
```

## ACVP Simulation

Requires [cJSON](https://github.com/DaveGamble/cJSON) and the Python
`cryptography` package:

```bash
brew install cjson pkg-config
python3 -m venv .venv
source .venv/bin/activate
pip install cryptography
```

Run the full three-stage pipeline:

```bash
make acvp-test
```

This runs:
1. **Stage 1** — `generate_vectors.py` generates fresh random ACVP-format
   request vectors using Python's `cryptography` library as the reference
2. **Stage 2** — `acvp_runner` feeds each request through the C library and
   writes ACVP-format response JSON
3. **Stage 3** — `validate_responses.py` independently compares C output
   against the Python reference and reports pass/fail per algorithm

---

## FIPS Documentation

| Document | Description |
|---|---|
| `docs/security-policy.md` | Security policy structured to SP 800-140Br1 Annex B |
| `docs/algorithm-inventory.md` | Approved algorithm inventory with ACVP coverage |
| `docs/boundary.md` | Cryptographic module boundary definition |
| `docs/acvp_validation_prompt.md` | ACVP pipeline design reference and decision log |

---

## Development Tools

Claude (Anthropic) assisted with architecture design, code generation, and
sprint planning. Gemini (Google) generated the ACVP validation scripts
(`generate_vectors.py`, `validate_responses.py`) as an independent validation
layer. All design decisions, code review, integration, and testing were
performed by the author.

---

## References

- [FIPS 140-3](https://csrc.nist.gov/publications/detail/fips/140/3/final)
- [FIPS 197 — AES](https://csrc.nist.gov/publications/detail/fips/197/final)
- [FIPS 180-4 — SHA](https://csrc.nist.gov/publications/detail/fips/180/4/final)
- [FIPS 198-1 — HMAC](https://csrc.nist.gov/publications/detail/fips/198/1/final)
- [SP 800-132 — PBKDF](https://csrc.nist.gov/publications/detail/sp/800/132/final)
- [SP 800-140Br1 — CMVP Security Policy](https://csrc.nist.gov/publications/detail/sp/800/140b/r1/final)
- [ACVP Specification](https://pages.nist.gov/ACVP/)
- [NIST ACVTS Demo Server](https://demo.acvts.nist.gov)
