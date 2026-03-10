# Security Policy

## Scope

fips-crypto is a **learning project** implementing FIPS 140-3 cryptographic
primitives in ANSI C. It is not intended for production use.

> **Not for production use.** This library is for educational purposes only.
> It has not been validated by a NIST-accredited Cryptographic Security Testing
> Laboratory (CSTL) and carries no CMVP certificate.

---

## Reporting a Vulnerability

If you discover a security vulnerability in this repository — including
implementation errors in the cryptographic primitives, memory safety issues,
or incorrect FIPS mode enforcement — please report it privately rather than
opening a public issue.

**Report via GitHub private vulnerability disclosure:**
[github.com/bsistany/fips-certification/security/advisories/new](https://github.com/bsistany/fips-certification/security/advisories/new)

Please include:
- A clear description of the vulnerability
- The affected file(s) and function(s)
- A minimal reproducer or test case if possible
- Your assessment of severity

You can expect an acknowledgment within 7 days.

---

## Known Limitations

The following are **known gaps** documented in `docs/security-policy.md`
Appendix B. They are intentional limitations of a learning project, not
undisclosed vulnerabilities:

| Gap | Description |
|---|---|
| No software integrity test | Module binary is not verified at startup |
| No DRBG | Keys must be supplied by the caller; no internal key generation |
| Zeroization not guaranteed | Uses `memset` rather than `explicit_bzero` or `memset_s` |
| No constant-time operations | AES table lookups and MAC comparison are not constant-time |
| No CAVP certificates | Algorithm testing is local simulation only (Sprint 8) |

---

## Supported Versions

This is a single-branch learning project. Only the current `main` branch is
maintained. No backports or patch releases are planned.
