# CMVP Certification Process

This document shows the end-to-end path from implementation to a FIPS 140-3
validation certificate, and maps the work done in this project onto that path.

---

```mermaid
flowchart TD

    subgraph Vendor ["Vendor"]
        V1["**Implement FIPS 140-3 Controls**
        fips-crypto: AES, SHA-256, HMAC, PBKDF2 ✓"]
        V2["**Create Security Policy & Evidence**
        security-policy.md, boundary.md,
        algorithm-inventory.md ✓"]
        V3["Address Lab Findings
        _(out of scope)_"]
        V4["Address CMVP Questions
        _(out of scope)_"]
    end

    subgraph CST_Lab ["CST Lab (NVLAP)"]
        L1a["**ACVP Algorithm Testing**
        Sprint 8 — local simulation ✓
        43/43 vectors passing"]
        L1b["**ACVP Algorithm Testing**
        Sprint 9 — demo server
        awaiting NIST ⟳"]
        L2["Physical & Operational Testing
        _(out of scope)_"]
        L3["Review Source Code
        _(out of scope)_"]
        L4["Submit Final Test Report
        _(out of scope)_"]
        L5["Manage Coordination Phase
        _(out of scope)_"]
    end

    subgraph CMVP ["CMVP (NIST & CCCS)"]
        C1["Review Pending Queue
        _(out of scope)_"]
        C2["Technical Review
        _(out of scope)_"]
        C3["Issue Validation Certificate
        _(out of scope)_"]
    end

    %% Project flows (solid)
    V1 --> V2
    V2 --> L1a
    V2 --> L1b

    %% Out-of-scope flows (dashed via linkStyle)
    L1a --> L2
    L1b --> L2
    L2  --> L3
    L3  -- Non-compliance --> V3
    V3  --> L3
    L3  --> L4
    L4  --> C1
    L4  --> L5
    C1  --> C2
    C2  -- Clarifications --> L5
    L5  --> V4
    V4  --> L5
    L5  --> C2
    C2  --> C3

    %% Styling — project nodes
    style V1  fill:#95d5b2,stroke:#0F6E56,color:#085041
    style V2  fill:#95d5b2,stroke:#0F6E56,color:#085041
    style L1a fill:#95d5b2,stroke:#0F6E56,color:#085041
    style L1b fill:#ffd166,stroke:#BA7517,color:#633806

    %% Styling — out-of-scope nodes
    style V3  fill:#e0e0e0,stroke:#aaa,color:#888
    style V4  fill:#e0e0e0,stroke:#aaa,color:#888
    style L2  fill:#e0e0e0,stroke:#aaa,color:#888
    style L3  fill:#e0e0e0,stroke:#aaa,color:#888
    style L4  fill:#e0e0e0,stroke:#aaa,color:#888
    style L5  fill:#e0e0e0,stroke:#aaa,color:#888
    style C1  fill:#e0e0e0,stroke:#aaa,color:#888
    style C2  fill:#e0e0e0,stroke:#aaa,color:#888
    style C3  fill:#e0e0e0,stroke:#aaa,color:#888

    %% Link styles — out-of-scope edges dashed
    %% Edges 0-1: V1→V2, V2→L1a, V2→L1b  (project, solid — default)
    %% Edges 3 onward: out of scope
    linkStyle 3,4,5,6,7,8,9,10,11,12,13,14,15,16 stroke:#aaa,stroke-dasharray:5 3
```

---

## Legend

| Style | Meaning |
|---|---|
| Green node | Complete in this project |
| Amber node | In progress in this project |
| Grey node (dashed edges) | Out of scope — requires an accredited NVLAP lab and CMVP engagement |

---

## Where this project fits

This project covers the first two vendor steps and the first CST Lab step:

- **V1 — Implement FIPS 140-3 Controls**: the `fips-crypto` C library with
  AES-128/256-CBC, SHA-256, HMAC-SHA-256, PBKDF2-HMAC-SHA-256, self-tests,
  and FIPS mode flag. Sprints 1–6 complete, 54/54 unit tests passing.

- **V2 — Create Security Policy & Evidence**: `docs/security-policy.md`
  structured to SP 800-140Br1 Annex B (B.2.1–B.2.12), `docs/boundary.md`,
  and `docs/algorithm-inventory.md`. NSP Alignment Sprint complete.

- **L1a — ACVP Algorithm Testing (local simulation)**: Sprint 8 complete.
  43/43 ACVP-format vectors passing across all four algorithms using a
  Python-harness / C-runner pipeline.

- **L1b — ACVP Algorithm Testing (demo server)**: Sprint 9 in progress.
  Awaiting NIST ACVTS demo server credentials to submit vectors to
  `demo.acvts.nist.gov` and obtain an authoritative disposition.

Everything from L2 onward requires engagement with an accredited Cryptographic
Security Testing (CST) laboratory under NVLAP, and is outside the scope of
this reference implementation.

---

**Navigation:** [Documentation Index](../README.md) · [ACVP Integration Workflow](acvp/integration.md) · [Security Policy](security-policy.md)
