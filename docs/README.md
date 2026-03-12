# fips-crypto — Documentation Index

This index is the entry point for all project documentation. Documents are
organised by recommended reading order. Each document links back here and
to its neighbours so you can navigate the full set without returning to the
repo root.

---

## Start Here

If you are new to this project or to FIPS 140-3, start with the overview:

**[How FIPS Validation Works](how-fips-validation-works.md)**  
A complete, technical explanation of the FIPS 140-3 validation landscape —
why it exists, how the CMVP and CAVP processes work end to end, what this
project implements, and how each sprint maps to a real certification
requirement. Includes glossary and references.

**[CMVP Certification Process](cmvp-process.md)**  
A visual overview of the full certification path from implementation to
CAVP certificate, with this project's completed and in-progress steps
highlighted. Useful orientation before reading the core FIPS documents.

---

## Core FIPS Documents

These three documents form the heart of the project. Together they describe
what the module is, what it does, and where its boundary lies — the same
artifacts a CSTL would review in a real CMVP engagement.

| Document | Description |
|---|---|
| [CMVP Certification Process](cmvp-process.md) | Visual diagram of the full certification path from implementation to CAVP certificate. Shows where this project sits relative to vendor, CST lab, and CMVP phases, with completed and in-progress steps highlighted. |
| [Security Policy](security-policy.md) | Non-proprietary security policy structured to SP 800-140Br1 Annex B (B.2.1–B.2.12). Covers roles, services, algorithms, self-tests, key management, and 12 documented known gaps. |
| [Algorithm Inventory](algorithm-inventory.md) | Complete inventory of approved, allowed, and non-approved algorithms. Includes ACVP validation status table and CAVP coverage tracking. |
| [Module Boundary](boundary.md) | Precise definition of the cryptographic module boundary — what is inside, what is outside, all external dependencies, CSPs, and generated boundary analysis output. |

---

## ACVP Validation Pipeline

These documents cover the Automated Cryptographic Validation Protocol
simulation pipeline built in Sprint 8 and the planned demo server submission
in Sprint 9.

| Document | Description |
|---|---|
| [ACVP Validation Design Record](acvp_validation_prompt.md) | Complete design record for the ACVP pipeline — architecture decisions, schema conventions, ground truth rationale, validation methodology, and the Gemini prompt used to generate the independent validation scripts. |
| [ACVP Integration Workflow](acvp/integration.md) | How the ACVP client-server exchange works, the full six-phase API flow with JSON examples, how Sprint 8's local simulation maps to the real server exchange, and Sprint 9 execution steps. |
| [ACVP Credentials Setup](acvp/credentials.md) | Step-by-step guide to ACVTS demo server registration, TLS client certificate generation (OpenSSL commands), macOS Keychain integration, TOTP setup from scratch, and security rules for credential management. |

---

## Recommended Reading Order

For a first-time reader working through the project end to end:

```
1. how-fips-validation-works.md     ← understand the landscape
2. cmvp-process.md                  ← see where this project fits visually
3. security-policy.md               ← the core FIPS document
4. algorithm-inventory.md           ← what is implemented and validated
5. boundary.md                      ← where the module boundary sits
6. acvp_validation_prompt.md        ← how the ACVP pipeline was designed
7. acvp/integration.md              ← how ACVP client-server works
8. acvp/credentials.md              ← how to set up for Sprint 9
```

---

## Repo-Level Documents

| Document | Description |
|---|---|
| [README](../README.md) | Project overview, build instructions, test commands, and ACVP simulation setup. |
| [SECURITY](../SECURITY.md) | Vulnerability disclosure policy, known limitations, and supported versions. |
| [CONTRIBUTORS](../CONTRIBUTORS.md) | Author record, AI tool acknowledgment, and third-party library attributions. |
| [Sprint Plan](../plan.md) | Sprint history from Sprint 0 through Sprint 9, with deliverables and FIPS references per sprint. |

---

## Document Status

| Document | Status | Last updated |
|---|---|---|
| `how-fips-validation-works.md` | ✅ Current | Sprint 8 |
| `cmvp-process.md` | ✅ Current | Sprint 8 |
| `security-policy.md` | ✅ Current | Sprint 8 |
| `algorithm-inventory.md` | ✅ Current | Sprint 8 |
| `boundary.md` | ✅ Current | Sprint 7 |
| `acvp_validation_prompt.md` | ✅ Current | Sprint 8 |
| `acvp/integration.md` | ✅ Current | Sprint 8 |
| `acvp/credentials.md` | ✅ Current | Sprint 8 |

---

*[Back to repo root](../README.md)*
