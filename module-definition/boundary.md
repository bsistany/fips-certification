# Cryptographic Module Boundary Definition

## 1. Module Identification

| Field | Value |
|---|---|
| Module Name | PyCA Cryptography Library |
| Version | 46.0.5 |
| Module Type | Software |
| Security Level | 1 |
| Standard | FIPS 140-3 / ISO/IEC 19790:2012 |

---

## 2. Module Boundary

FIPS 140-3 requires a clearly defined cryptographic boundary separating the module from everything outside it.

For a software module, the boundary is **logical**, not physical. It is defined as:

- All Python source files under `cryptography/hazmat/` (the "hazardous materials" primitives layer)
- The compiled Rust extension (`cryptography_rust`) and OpenSSL bindings included in the package
- The public API surface exposed through `cryptography.hazmat.primitives`

Everything outside this boundary — the calling application, the OS, the Python interpreter, hardware — is considered the **operational environment**.

---

## 3. Interfaces (per FIPS 140-3 §7.6)

| Interface | Description | Examples |
|---|---|---|
| Data Input | Plaintext, ciphertext, keys, IVs, salts passed into the module | `cipher.update(plaintext)` |
| Data Output | Ciphertext, digests, signatures, derived keys returned by the module | `encryptor.finalize()` |
| Control Input | API calls that direct the module's operation | `Cipher(algorithms.AES(key), modes.CBC(iv))` |
| Status Output | Return values, exceptions, and error codes | `FIPSModeError`, `InvalidSignature` |

---

## 4. Roles and Services

FIPS 140-3 requires at least one operator role. At Security Level 1, authentication is not required.

| Role | Description |
|---|---|
| Crypto Officer | The developer or deployer who installs the module, enables FIPS mode, and configures the operational environment |
| User | The calling application that invokes cryptographic services through the public API |

### Approved Services

| Service | Role |
|---|---|
| Symmetric encryption/decryption (AES) | User |
| Hashing (SHA-2, SHA-3) | User |
| Message authentication (HMAC, CMAC) | User |
| Digital signature generation and verification (RSA, ECDSA) | User |
| Key agreement (ECDH, FFDH) | User |
| Key derivation (HKDF, PBKDF2, KBKDF) | User |
| DRBG (via OpenSSL) | User |
| FIPS mode enable/status check | Crypto Officer |
| Self-test execution | Crypto Officer / Module |

---

## 5. FIPS Mode Indicator

The module provides a FIPS mode indicator as required by FIPS 140-3 §7.6:

```python
from cryptography.hazmat.primitives.ciphers import Cipher, algorithms, modes

# Check FIPS mode status
from cryptography.hazmat.bindings.openssl.binding import Binding
fips_enabled = Binding().lib.FIPS_mode()
```

When FIPS mode is active, instantiation of any non-approved algorithm raises `ValueError` or `UnsupportedAlgorithm`, preventing its use.

---

## 6. Self-Tests

FIPS 140-3 §10.3 requires the following self-tests:

| Test | Trigger | Description |
|---|---|---|
| Known Answer Test (KAT) | Module load (POST) | Runs fixed-input/output tests for each approved algorithm |
| Pairwise Consistency Test | Asymmetric key generation | Verifies generated key pair works correctly before use |
| Continuous Random Number Generator Test (CRNGT) | Each DRBG call | Checks that consecutive outputs are not identical |
| Software Integrity Test | Module load (POST) | HMAC-SHA-256 computed over module files and compared to reference value |

If any POST fails, the module enters an error state and all cryptographic operations are blocked.

---

## 7. Cryptographic State Variables (CSPs)

All Critical Security Parameters reside in RAM only. No CSPs are written to disk by the module.

| CSP | Algorithm | Storage | Zeroization |
|---|---|---|---|
| AES key | AES-CBC/GCM/CTR | RAM | Object deletion |
| RSA private key | RSA-PKCS1v1.5 / RSA-PSS | RAM | Object deletion |
| EC private key | ECDSA / ECDH | RAM | Object deletion |
| HMAC key | HMAC-SHA-2/3 | RAM | Object deletion |
| PBKDF2 password | PBKDF2 | RAM | Object deletion |
| DRBG internal state | CTR_DRBG / HMAC_DRBG | RAM (OpenSSL) | OpenSSL cleanup |

### Known Gap — Memory Zeroization

Python's garbage collector does not guarantee immediate or secure memory zeroization. Sensitive key material may persist in RAM after object deletion until the GC reclaims the memory. This is a known limitation of software modules running on general-purpose Python interpreters and will be documented as a finding in the lab report (Sprint 6).

---

## 8. Physical Security

Not applicable. This is a Software module at Security Level 1. Physical security requirements do not apply.
