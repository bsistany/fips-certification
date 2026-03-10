# Algorithm Inventory

**Module:** fips-crypto  
**Version:** 1.0.0  
**Reference:** FIPS 140-3, SP 800-131A Rev 2, FIPS 197, FIPS 180-4, FIPS 198-1, SP 800-132  

---

## Classification Key

| Status | Meaning |
|---|---|
| ✅ Approved | Allowed in FIPS mode. Requires CAVP certificate before CMVP submission. |
| ⚠️ Allowed | Permitted under specific conditions only (legacy use, key transport, etc.) |
| 🚫 Non-Approved | Blocked in FIPS mode. Returns `FIPS_ERR_NOT_APPROVED`. |
| ⬜ Not Implemented | Not present in this module. |

---

## 1. Symmetric Encryption

| Algorithm | Key Size | Mode | Status | Reference | Implemented |
|---|---|---|---|---|---|
| AES | 128-bit | CBC | ✅ Approved | FIPS 197, SP 800-38A | Yes — `src/aes.c` |
| AES | 256-bit | CBC | ✅ Approved | FIPS 197, SP 800-38A | Yes — `src/aes.c` |
| AES | 128/256-bit | GCM | ✅ Approved | FIPS 197, SP 800-38D | ⬜ Not implemented |
| AES | 128/256-bit | CTR | ✅ Approved | FIPS 197, SP 800-38A | ⬜ Not implemented |
| AES | 128/256-bit | ECB | ⚠️ Allowed | FIPS 197 | ⬜ Not implemented |
| 3DES | 112/168-bit | CBC | ⚠️ Allowed | SP 800-131A (decrypt only, legacy) | ⬜ Not implemented |
| DES | 56-bit | Any | 🚫 Non-Approved | Withdrawn | ⬜ Not implemented |
| RC4 | Any | — | 🚫 Non-Approved | Not a FIPS algorithm | ⬜ Not implemented |
| Blowfish | Any | Any | 🚫 Non-Approved | Not a FIPS algorithm | ⬜ Not implemented |

---

## 2. Hashing

| Algorithm | Output Size | Status | Reference | Implemented |
|---|---|---|---|---|
| SHA-256 | 256-bit | ✅ Approved | FIPS 180-4 | Yes — `src/sha256.c` |
| SHA-224 | 224-bit | ✅ Approved | FIPS 180-4 | ⬜ Not implemented |
| SHA-384 | 384-bit | ✅ Approved | FIPS 180-4 | ⬜ Not implemented |
| SHA-512 | 512-bit | ✅ Approved | FIPS 180-4 | ⬜ Not implemented |
| SHA-3-256 | 256-bit | ✅ Approved | FIPS 202 | ⬜ Not implemented |
| SHA-1 | 160-bit | ⚠️ Allowed | SP 800-131A (verification only) | ⬜ Not implemented |
| MD5 | 128-bit | 🚫 Non-Approved | Not a FIPS algorithm | ⬜ Not implemented |
| BLAKE2 | Any | 🚫 Non-Approved | Not a FIPS algorithm | ⬜ Not implemented |

---

## 3. Message Authentication

| Algorithm | Key Size | Status | Reference | Implemented |
|---|---|---|---|---|
| HMAC-SHA-256 | ≥ 112-bit (14 bytes) | ✅ Approved | FIPS 198-1 | Yes — `src/hmac.c` |
| HMAC-SHA-384 | ≥ 112-bit | ✅ Approved | FIPS 198-1 | ⬜ Not implemented |
| HMAC-SHA-512 | ≥ 112-bit | ✅ Approved | FIPS 198-1 | ⬜ Not implemented |
| CMAC-AES | 128/256-bit | ✅ Approved | SP 800-38B | ⬜ Not implemented |
| HMAC-SHA-1 | ≥ 112-bit | ⚠️ Allowed | SP 800-131A (KDF use only) | ⬜ Not implemented |
| HMAC-MD5 | Any | 🚫 Non-Approved | Not a FIPS algorithm | ⬜ Not implemented |

> **Note on HMAC key length:** FIPS 198-1 and SP 800-131A require HMAC keys
> to be at least 112 bits (14 bytes) in FIPS mode. Our implementation enforces
> this via `fips_check_algorithm()` but does not currently enforce minimum key
> length at the HMAC layer itself. This is a known gap.

---

## 4. Key Derivation

| Algorithm | PRF | Iterations | Salt | Status | Reference | Implemented |
|---|---|---|---|---|---|---|
| PBKDF2 | HMAC-SHA-256 | ≥ 1000 | ≥ 16 bytes | ✅ Approved | SP 800-132 | Yes — `src/pbkdf2.c` |
| HKDF | HMAC-SHA-256 | N/A | N/A | ✅ Approved | SP 800-56C Rev 2 | ⬜ Not implemented |
| KBKDF | HMAC-SHA-256 | N/A | N/A | ✅ Approved | SP 800-108 | ⬜ Not implemented |
| PBKDF2 | HMAC-SHA-1 | ≥ 1000 | ≥ 16 bytes | ⚠️ Allowed | SP 800-132 (legacy) | ⬜ Not implemented |
| Scrypt | — | — | — | 🚫 Non-Approved | Not a FIPS algorithm | ⬜ Not implemented |
| Argon2 | — | — | — | 🚫 Non-Approved | Not a FIPS algorithm | ⬜ Not implemented |

---

## 5. Random Number Generation

| Algorithm | Status | Reference | Implemented |
|---|---|---|---|
| CTR_DRBG (AES-256) | ✅ Approved | SP 800-90A Rev 1 | ⬜ Not implemented — known gap |
| HMAC_DRBG (SHA-256) | ✅ Approved | SP 800-90A Rev 1 | ⬜ Not implemented — known gap |
| Hash_DRBG (SHA-256) | ✅ Approved | SP 800-90A Rev 1 | ⬜ Not implemented — known gap |
| /dev/urandom (entropy source) | ⚠️ Allowed | Platform dependent | ⬜ Not implemented |

> **Note:** A DRBG is required for any key generation operation. Until a DRBG
> is implemented this module cannot generate keys internally. Keys must be
> provided by the caller.

---

## 6. Asymmetric Algorithms

| Algorithm | Key Size | Status | Reference | Implemented |
|---|---|---|---|---|
| RSA-PKCS1v1.5 | 2048/3072/4096-bit | ✅ Approved | FIPS 186-5 | ⬜ Not implemented |
| RSA-PSS | 2048/3072/4096-bit | ✅ Approved | FIPS 186-5 | ⬜ Not implemented |
| ECDSA | P-256/P-384/P-521 | ✅ Approved | FIPS 186-5 | ⬜ Not implemented |
| ECDH | P-256/P-384/P-521 | ✅ Approved | SP 800-56A Rev 3 | ⬜ Not implemented |
| RSA | 1024-bit | 🚫 Non-Approved | SP 800-131A | ⬜ Not implemented |
| DSA | Any | 🚫 Non-Approved | Removed in FIPS 186-5 (2023) | ⬜ Not implemented |
| Ed25519 / Ed448 | — | 🚫 Non-Approved | Not in FIPS 186-5 | ⬜ Not implemented |
| X25519 / X448 | — | 🚫 Non-Approved | Not in FIPS 186-5 | ⬜ Not implemented |
| SECP256K1 | — | 🚫 Non-Approved | Koblitz curve, not NIST-approved | ⬜ Not implemented |

---

## 7. Algorithm Approval Table (fips.h mapping)

This table maps the `fips_algorithm_t` enum values in `include/fips.h` to
their classification. This is the enforcement table used by
`fips_check_algorithm()` at runtime.

| Enum value | Algorithm | Approved in fips.c |
|---|---|---|
| `ALG_AES_128_CBC` | AES-128-CBC | Yes |
| `ALG_AES_256_CBC` | AES-256-CBC | Yes |
| `ALG_SHA256` | SHA-256 | Yes |
| `ALG_HMAC_SHA256` | HMAC-SHA-256 | Yes |
| `ALG_PBKDF2_SHA256` | PBKDF2-HMAC-SHA-256 | Yes |
| `ALG_MD5` | MD5 | No |
| `ALG_SHA1` | SHA-1 | No |
| `ALG_DES` | DES | No |
| `ALG_RC4` | RC4 | No |
| `ALG_BLOWFISH` | Blowfish | No |

---

## 8. CAVP Validation Status

CAVP certificates are required for every approved algorithm before a CMVP
submission. The table below tracks validation status for this module.

| Algorithm | CAVP Required | Status |
|---|---|---|
| AES-128-CBC | Yes | ⬜ Simulated only — Sprint 8 planned |
| AES-256-CBC | Yes | ⬜ Simulated only — Sprint 8 planned |
| SHA-256 | Yes | ⬜ Simulated only — Sprint 8 planned |
| HMAC-SHA-256 | Yes | ⬜ Simulated only — Sprint 8 planned |
| PBKDF2-HMAC-SHA-256 | Yes | ⬜ Simulated only — Sprint 8 planned |

> For a real submission each algorithm would need to be tested against NIST's
> ACVP server (demo.acvts.nist.gov) and receive a certificate number.
