# ACVP Integration — Workflow, Client Interaction, and API Flow

**File:** `docs/acvp/integration.md`  
**Relates to:** Sprint 9 — ACVP Demo Server Submission  
**Prerequisites:** `docs/acvp/credentials.md`, Sprint 8 complete (local simulation passing)

---

## 1. What This Document Covers

This document explains:

1. What ACVP is and how it relates to CAVP
2. How a client (your C library) interacts with the NIST ACVTS server
3. The full ACVP API flow — from registration to certificate
4. How Sprint 8's local simulation maps to the real server exchange
5. Practical steps for Sprint 9 execution

---

## 2. Conceptual Foundation

### 2.1 CAVP, ACVP, and ACVTS — the three terms

These three terms are related but distinct. Confusing them is a common source
of mistakes in FIPS documentation.

| Term | Full name | What it is |
|---|---|---|
| CAVP | Cryptographic Algorithm Validation Program | The NIST *program* that governs algorithm testing and issues certificates |
| ACVP | Automated Cryptographic Validation Protocol | The JSON-over-HTTPS *protocol* used to conduct CAVP testing |
| ACVTS | Automated Cryptographic Validation Testing System | The NIST *server infrastructure* that implements ACVP at `acvts.nist.gov` |

The relationship: **CAVP** is the program. **ACVP** is how the program is
conducted today. **ACVTS** is the server you talk to. You implement ACVP
to participate in CAVP via ACVTS.

### 2.2 What ACVP replaced

Before ACVP, algorithm validation used **CAVS** (Cryptographic Algorithm
Validation System) — a manual, file-based process. A vendor would download
test vector files, run them through their implementation, and email results
back to NIST. This was slow (weeks per round-trip) and error-prone.

ACVP replaced CAVS with a real-time JSON API. The same exchange that took
weeks now takes minutes. CAVS files are no longer accepted for new submissions.

### 2.3 The role of the demo server

NIST operates two ACVTS environments:

| Environment | URL | Purpose |
|---|---|---|
| Demo / sandbox | `demo.acvts.nist.gov` | Testing and exploration — no real certificates issued |
| Production | `acvts.nist.gov` | Real submissions — certificates issued by NIST |

Sprint 9 targets the **demo server**. This is the correct starting point —
it lets you validate your client implementation and JSON handling without
consuming real certificate quota or requiring a CSTL engagement.

### 2.4 Where this project sits in the CAVP process

A full CAVP submission requires:

```
Vendor implements algorithm
        ↓
Vendor tests locally (Sprint 8 — done)
        ↓
Vendor submits to ACVTS demo server (Sprint 9)
        ↓
Vendor engages an accredited CSTL (lab)
        ↓
CSTL tests via ACVTS production server
        ↓
NIST issues CAVP certificate
        ↓
Certificate number used in CMVP submission
```

This project completes the first two steps. The remaining steps require
an accredited lab and are beyond the scope of this reference implementation.

---

## 3. How the ACVP Client-Server Interaction Works

### 3.1 The core idea

ACVP is a **request-response protocol over HTTPS**. The server holds the
test vectors (inputs and expected outputs). The client receives the inputs,
runs them through the implementation under test, and returns the outputs.
The server compares outputs against its expected values and reports pass/fail.

This is the inverse of how Sprint 8's local simulation worked. In Sprint 8:

```
generate_vectors.py  →  holds expected values (embedded in request JSON)
acvp_runner          →  runs C library, returns outputs
validate_responses.py →  compares outputs against expected
```

In the real ACVP exchange:

```
ACVTS server         →  holds expected values (never sent to client)
your client          →  runs C library, returns outputs
ACVTS server         →  compares outputs, returns pass/fail
```

The key difference: in real ACVP **the client never sees the expected
values**. The server holds them privately. This is why the `"expected"`
field in our Sprint 8 request JSON must be stripped before submission —
it is a local simulation convention that has no place in the real protocol.

### 3.2 Authentication

Every request to the ACVTS server is authenticated with:

1. **TLS client certificate** — mutual TLS (mTLS). Both server and client
   present certificates. Your client certificate proves your identity to NIST.
2. **JWT bearer token** — obtained by logging in with your credentials and
   TOTP code. Included in every API request as an `Authorization` header.
3. **TOTP (Time-based One-Time Password)** — two-factor authentication
   required at login. See `docs/acvp/credentials.md` for setup.

### 3.3 Sessions and vector sets

ACVP organises work into **sessions** and **vector sets**:

- A **session** (`testSession`) groups one or more algorithm requests
- Each algorithm in a session produces a **vector set** (`vsId`) containing
  the test groups and test cases
- Vector sets are processed and responses submitted independently

This maps directly to our Sprint 8 structure — each `vsId` in our local
JSON files (1=AES, 2=SHA-256, 3=HMAC, 4=PBKDF2) corresponds to one
vector set in a real session.

---

## 4. The Full ACVP API Flow

The real ACVP exchange has six distinct phases. Each phase is a separate
HTTPS call.

### Phase 1 — Login

Obtain a JWT token by authenticating with your credentials and TOTP code.

```
POST /acvp/v1/login
Body: { "password": "...", "totpPassword": "123456" }

Response: { "accessToken": "eyJ...", "expiresIn": 1800 }
```

The token expires after 30 minutes. All subsequent requests include:
```
Authorization: Bearer eyJ...
```

### Phase 2 — Register capabilities

Tell the server which algorithms you want to test and their parameters.
This is called a **capabilities registration**.

```
POST /acvp/v1/testSessions
Body:
{
  "isSample": true,
  "algorithms": [
    {
      "algorithm": "ACVP-AES-CBC",
      "revision": "1.0",
      "direction": ["encrypt", "decrypt"],
      "keyLen": [256]
    },
    {
      "algorithm": "ACVP-SHA2-256",
      "revision": "1.0",
      "messageLength": [{ "min": 0, "max": 65536, "increment": 8 }]
    },
    ...
  ]
}

Response: { "testSession": { "url": "/acvp/v1/testSessions/12345", "vsIds": [1,2,3,4] } }
```

`"isSample": true` requests sample vectors (a smaller, fixed set used for
testing your client). Use `false` for a full validation run.

### Phase 3 — Retrieve vector sets

For each `vsId` returned in Phase 2, retrieve the test vectors:

```
GET /acvp/v1/testSessions/12345/vectorSets/1

Response:
{
  "vsId": 1,
  "algorithm": "ACVP-AES-CBC",
  "testGroups": [
    {
      "tgId": 1,
      "direction": "encrypt",
      "keyLen": 256,
      "tests": [
        { "tcId": 1, "key": "...", "iv": "...", "pt": "..." }
      ]
    }
  ]
}
```

Notice: **no `"expected"` field**. The server never reveals expected values.

This response has the same structure as our Sprint 8 `acvp/request/*.json`
files — with the `"expected"` field removed. The `acvp_runner` can process
it directly after this field is stripped.

### Phase 4 — Run your implementation

Feed each vector set through `acvp_runner` exactly as in Sprint 8:

```bash
./acvp/acvp_runner <vector_set.json> <response.json>
```

The runner produces the same ACVP-format response JSON as in Sprint 8.

### Phase 5 — Submit responses

For each vector set, submit your response JSON to the server:

```
POST /acvp/v1/testSessions/12345/vectorSets/1/results
Body: <contents of response.json>

Response: { "disposition": "approved" }   ← all passed
       or: { "disposition": "failed" }    ← one or more failed
```

`"approved"` means your outputs matched the server's expected values for
every test case. `"failed"` means at least one test case did not match —
the server will indicate which `tcId` failed.

### Phase 6 — Retrieve results

After submitting all vector sets, retrieve the final session disposition:

```
GET /acvp/v1/testSessions/12345/results

Response:
{
  "testSession": {
    "vsIds": [1, 2, 3, 4],
    "disposition": "approved"
  }
}
```

On the demo server, `"approved"` confirms your implementation is correct
against NIST's authoritative vectors. On the production server, this
disposition triggers the CAVP certificate issuance process (via your CSTL).

---

## 5. How Sprint 8 Maps to the Real Flow

This table shows the direct correspondence between our local simulation
and the real ACVP exchange:

| Sprint 8 (local) | Real ACVP (Sprint 9) | Notes |
|---|---|---|
| `generate_vectors.py` writes `acvp/request/*.json` | Phase 3: GET vector sets from server | Server provides inputs; we no longer generate them |
| `"expected"` field in request JSON | Not present | Strip before submission |
| `acvp_runner` reads request, writes response | Phase 4: same runner, same JSON | No changes to runner needed |
| `acvp/response/*.json` | Phase 5: POST results to server | Submit response JSON via HTTPS |
| `validate_responses.py` compares output vs expected | Phase 5 response: server reports pass/fail | Server replaces local validator |
| Local summary report | Phase 6: GET session results | Server provides authoritative disposition |

The `acvp_runner` binary requires **no changes** for Sprint 9. The only
new work is:
1. A Python client script to handle the HTTPS calls (Phases 1–3, 5–6)
2. A pre-submission script to strip `"expected"` from request JSON
3. TLS and credential configuration (see `docs/acvp/credentials.md`)

---

## 6. Sprint 9 Execution Steps

### Step 1 — Complete credential setup
Follow `docs/acvp/credentials.md` in full before proceeding.

### Step 2 — Install Python ACVP client dependencies

```bash
source .venv/bin/activate
pip install requests  # HTTPS client
```

### Step 3 — Login and obtain JWT token

```bash
python3 acvp/scripts/acvp_login.py
# Writes .acvp_token to repo root (gitignored)
```

### Step 4 — Register capabilities and retrieve vector sets

```bash
python3 acvp/scripts/acvp_register.py
# Writes server-provided vector sets to acvp/request/server_*.json
```

### Step 5 — Run the runner against server vectors

```bash
# Strip "expected" is not needed — server vectors don't have it
./acvp/acvp_runner acvp/request/server_aes_cbc.json     acvp/response/server_aes_cbc.json
./acvp/acvp_runner acvp/request/server_sha256.json       acvp/response/server_sha256.json
./acvp/acvp_runner acvp/request/server_hmac_sha256.json  acvp/response/server_hmac_sha256.json
./acvp/acvp_runner acvp/request/server_pbkdf2.json       acvp/response/server_pbkdf2.json
```

### Step 6 — Submit responses and retrieve results

```bash
python3 acvp/scripts/acvp_submit.py
# Submits all response files, retrieves and prints final disposition
```

### Step 7 — Compare against local simulation

```bash
python3 acvp/scripts/acvp_compare.py
# Diffs server vector inputs against Sprint 8 local vectors
# Confirms server disposition matches local simulation results
```

---

## 7. Files To Be Created in Sprint 9

| File | Purpose |
|---|---|
| `acvp/scripts/acvp_login.py` | Phase 1 — login, write JWT token |
| `acvp/scripts/acvp_register.py` | Phase 2–3 — register capabilities, retrieve vector sets |
| `acvp/scripts/acvp_submit.py` | Phase 5–6 — submit responses, retrieve disposition |
| `acvp/scripts/acvp_compare.py` | Diff server vectors vs local simulation vectors |
| `acvp/config/capabilities.json` | Algorithm capabilities registration payload |
| `.acvp_token` | JWT token (gitignored — never commit) |

---

## 8. References

| Document | URL |
|---|---|
| ACVP Specification | https://pages.nist.gov/ACVP/ |
| ACVP Protocol GitHub | https://github.com/usnistgov/ACVP |
| ACVTS Demo Server | https://demo.acvts.nist.gov |
| SP 800-140Br1 | https://csrc.nist.gov/publications/detail/sp/800/140b/r1/final |
| ACVP Server Specification (draft) | https://github.com/usnistgov/ACVP-Server |

---

**Navigation:** [Documentation Index](../README.md) · Previous: [ACVP Validation Design Record](../acvp_validation_prompt.md) · Next: [ACVP Credentials Setup](credentials.md)
