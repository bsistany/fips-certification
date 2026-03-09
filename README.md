# fips-crypto

A minimal cryptographic library written in ANSI C, built from first principles with FIPS 140-3 constraints in mind.

This is a learning project. Every primitive is implemented from scratch — no external crypto dependencies. The goal is to understand what FIPS 140-3 actually requires by building something that could plausibly go through certification.

> **Not for production use.** This library is for educational purposes only.

---

## Algorithms

| Algorithm | Status |
|---|---|
| AES-128/256 (CBC) | ⬜ Sprint 1 |
| SHA-256 | ⬜ Sprint 2 |
| HMAC-SHA-256 | ⬜ Sprint 3 |
| FIPS mode + blocking | ⬜ Sprint 4 |
| Self-tests / KATs | ⬜ Sprint 5 |
| PBKDF2 | ⬜ Sprint 6 |

---

## Structure

```
fips-crypto/
├── src/          ← Algorithm implementations (.c)
├── include/      ← Public headers (.h)
├── tests/        ← Test vectors and test runner
├── docs/         ← FIPS boundary, algorithm inventory, security policy
├── Makefile
└── README.md
```

---

## Building

```bash
make
```

## Running Tests

```bash
make test
```

---

## References

- [FIPS 140-3](https://csrc.nist.gov/publications/detail/fips/140/3/final)
- [FIPS 197 — AES](https://csrc.nist.gov/publications/detail/fips/197/final)
- [FIPS 180-4 — SHA](https://csrc.nist.gov/publications/detail/fips/180/4/final)
- [FIPS 198-1 — HMAC](https://csrc.nist.gov/publications/detail/fips/198/1/final)
- [SP 800-132 — PBKDF](https://csrc.nist.gov/publications/detail/sp/800/132/final)
