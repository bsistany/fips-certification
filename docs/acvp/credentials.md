# ACVP Credentials — Registration, TLS, and TOTP Setup

**File:** `docs/acvp/credentials.md`  
**Relates to:** Sprint 9 — ACVP Demo Server Submission  
**Security note:** This document describes the credential process without
recording any secrets. Never commit private keys, passwords, TOTP seeds,
or JWT tokens to the repository.

---

## 1. What Credentials the ACVTS Server Requires

The NIST ACVTS demo server requires three things to authenticate a client:

| Credential | What it is | When used |
|---|---|---|
| Account (username + password) | Created during registration | Login (Phase 1) |
| TOTP code | Time-based one-time password from an authenticator app | Login (Phase 1) |
| TLS client certificate | X.509 certificate proving your identity at the TLS layer | Every HTTPS request |

All three are required. A missing or invalid TLS client certificate will
cause the connection to be rejected before your login credentials are
even checked.

---

## 2. Conceptual Background

### 2.1 What is mutual TLS (mTLS)?

Normal HTTPS is one-way TLS — the server presents a certificate to prove
its identity to the client (you verify you're talking to `demo.acvts.nist.gov`
and not an impostor). The client does not present a certificate.

**Mutual TLS** is two-way — both sides present certificates. The server
presents its certificate (as usual), and the client also presents a
certificate. The server uses your client certificate to verify that you
are an authorised user before accepting any requests.

For ACVTS this means:
- NIST has a CA (Certificate Authority) that signs client certificates
- When you register, NIST issues you a signed client certificate
- Your HTTPS client presents this certificate on every connection
- Without it, the server rejects the connection at the TLS handshake level

### 2.2 What is TOTP?

TOTP (Time-based One-Time Password, RFC 6238) is a two-factor authentication
mechanism. It works as follows:

1. During registration, the server gives you a **secret seed** (usually
   displayed as a QR code)
2. You store this seed in an authenticator app (Google Authenticator,
   Authy, 1Password, etc.)
3. The app uses the seed + current time to compute a 6-digit code that
   changes every 30 seconds
4. When you log in, you provide this 6-digit code alongside your password
5. The server computes the same code independently and verifies they match

The seed is a shared secret between you and the server. It must be stored
securely — if someone obtains your seed, they can generate valid TOTP codes.

**Key point:** TOTP codes are time-sensitive. Your machine's clock must be
accurate (within ~30 seconds of UTC). On macOS, enable automatic time sync:
`System Settings → General → Date & Time → Set time automatically`.

### 2.3 What is a CSR?

A CSR (Certificate Signing Request) is a file you generate locally that
contains your public key and identity information. You submit it to NIST
during registration. NIST signs it with their CA key and returns a signed
certificate. You never share your private key — only the CSR.

The flow:
```
You generate:  private key + CSR
You send:      CSR to NIST
NIST returns:  signed certificate (your public key + NIST's signature)
You keep:      private key + signed certificate (never share private key)
```

---

## 3. Registration Process

### Step 1 — Request access

Send an email to the ACVP team requesting access to the demo server:

**To:** acvp@nist.gov  
**Subject:** ACVTS Demo Server Access Request  
**Body:** Include your name, organisation, and intended use — specifically
that you have completed local ACVP simulation and want to validate your
client implementation against NIST-authoritative test vectors.

NIST will reply with detailed instructions including CSR requirements and
how to submit via their secure file transfer system. Response times vary —
allow 3–10 business days.

### Step 2 — Review NIST's instructions and prepare to submit your CSR

NIST's reply email will contain detailed CSR requirements and instructions
for uploading via their Secure File Collaboration (SFC) service at
`https://sfc.doc.gov`. There is no online registration portal — the
entire credential exchange happens via email and SFC.

Read the instructions carefully before generating your CSR. When you are
ready to upload, reply to NIST's email stating your CSR is ready. NIST
will then send you an SFC invitation — this arrives as a **secure message
inside SFC itself**, not as a regular email. Create your SFC account at
`https://sfc.doc.gov` using the same email address you used in your
access request, then check your SFC Inbox for the invitation message.

**SFC account note:** SFC accounts go dormant after a period of inactivity
per NIST/DoC policy. A dormant account cannot self-reactivate — only NIST
can reactivate it by sending a new message to your registered email address.
Keep this in mind if you need to renew credentials in the future.

**Important:** NIST cannot accept a CSR via email attachment — it must
go through SFC.

### Step 3 — Generate your private key and CSR

Run the following from your `.acvp-credentials/` directory. The private
key stays on your machine — only the CSR is submitted to NIST.

**Filename convention (critical):** NIST requires an exact naming pattern:
```
OrganizationName_FirstName_LastName_Demo.key
OrganizationName_FirstName_LastName_Demo.csr
```
No spaces, no more than 3 underscores, not zipped. The ingest process
will not recognise the file if the name does not match exactly.

```bash
# Generate private key (4096-bit RSA)
openssl genrsa -out OrganizationName_FirstName_LastName_Demo.key 4096

# Generate CSR (SHA-256 signed)
openssl req -out OrganizationName_FirstName_LastName_Demo.csr \
  -key OrganizationName_FirstName_LastName_Demo.key \
  -new -sha256 \
  -subj "/emailAddress=you@domain.com/CN=Firstname Lastname/O=YourOrg/L=City/ST=Province/C=CA"
```

**Required DN attributes:**
- `emailAddress` — your email address (note: use `emailAddress`, not `EMAILADDRESS`)
- `CN` — your first and last name, or organisation name. No URLs.
- `C` — exactly 2 letters

`OU`, `L`, and `ST` are optional but recommended for completeness.

Verify the CSR looks correct:
```bash
openssl req -text -noout -in OrganizationName_FirstName_LastName_Demo.csr
```

Confirm:
- `Subject` contains `emailAddress`, `CN`, and `C`
- `Public-Key` shows `4096 bit`
- `Signature Algorithm` shows `sha256WithRSAEncryption`

### Step 4 — Submit the CSR via SFC

Once your SFC account is set up and NIST's invitation message is in your
SFC Inbox:

1. Go to `https://sfc.doc.gov` and log in
2. Click **Inbox** in the left-hand menu navigation
3. Open the message from NIST requesting your CSR
4. Click **Reply All** — a message window will open
5. Click the **Attach icon** (looks like a page with a folded corner)
6. Navigate to your `.acvp-credentials/` directory and select your `.csr` file
7. Optionally add a short comment, then click the blue **Send** button

**Important:** Do not use the "File Upload" feature in the SFC interface —
it will result in an error. The CSR must be attached to a message reply
as described above.

Submit only the `.csr` file — not the private key, not zipped.

### Step 5 — Receive certificate and TOTP seed

NIST will validate your CSR and return both the signed certificate and
your TOTP seed via an SFC message reply. Both arrive together — there is
no separate registration portal step.

Save the certificate to `.acvp-credentials/` using the same base name as
your key and CSR:
```
OrganizationName_FirstName_LastName_Demo.cer
```

Verify the certificate:
```bash
openssl x509 -text -noout -in OrganizationName_FirstName_LastName_Demo.cer
```

Check that:
- `Issuer` shows NIST's CA
- `Subject` matches what you put in your CSR
- `Validity` shows the certificate has not expired

Then proceed immediately to Section 4 to set up your TOTP seed.

---

## 4. TOTP Setup

### Step 1 — Choose an authenticator app

Any RFC 6238 compliant app works. Recommended options:

| App | Platform | Notes |
|---|---|---|
| 1Password | macOS, iOS, Android | Integrates with password manager |
| Authy | macOS, iOS, Android | Multi-device sync |
| Google Authenticator | iOS, Android | Simple, widely used |
| Microsoft Authenticator | iOS, Android | Works with any TOTP seed |

### Step 2 — Import the TOTP seed

NIST delivers your TOTP seed via SFC alongside your certificate as a
BASE64 encoded seed in a `.txt` file. Authenticator apps require the seed
in base32 encoding — convert it first:

```bash
python3 -c "
import base64, binascii
hex_seed = 'YOUR_HEX_STRING_HERE'
raw = binascii.unhexlify(hex_seed)
print(base64.b32encode(raw).decode())
"
```

Replace `YOUR_HEX_STRING_HERE` with the hex string from the file. The
output is the base32 seed your authenticator app expects.

In your authenticator app, choose **Enter setup key** or **Manual entry**:
- Account name: `ACVTS Demo`
- Key: paste the base32 string from the conversion above
- Type: **Time-based**

Tap Save. The app will immediately show a 6-digit code changing every
30 seconds.

### Step 3 — Verify your setup

Once added, your app will show a 6-digit code that changes every 30
seconds. Verify it works by attempting a login to the demo server. If
the TOTP code is rejected, check that your system clock is accurate
(within ~30 seconds of UTC).

### Step 4 — Back up your TOTP seed

Most authenticator apps allow you to export or back up your seeds. Do this
now. If you lose access to your authenticator app (lost phone, app reset),
you will be locked out and will need to contact NIST to reset your 2FA.

**Secure backup options:**
- Export the seed to your password manager (1Password, Bitwarden, etc.)
- Store the original hex seed file in an encrypted location (e.g. encrypted disk image)

---

## 5. macOS Keychain Integration (Optional but Recommended)

On macOS you can store your TLS client certificate in the system Keychain,
which allows HTTPS clients to use it without referencing a file path.

### Import certificate and key into Keychain

```bash
# Combine private key and certificate into a PKCS#12 bundle
openssl pkcs12 -export \
  -inkey .acvp-credentials/Independent_Bahman_Sistany_Demo.key \
  -in    .acvp-credentials/Independent_Bahman_Sistany_Demo.cer \
  -out   .acvp-credentials/Independent_Bahman_Sistany_Demo.p12 \
  -name  "ACVTS Demo Client"
```

You will be prompted to set an export password — use a strong one.

```bash
# Import into macOS Keychain
security import .acvp-credentials/client.p12 \
  -k ~/Library/Keychains/login.keychain-db \
  -T /usr/bin/curl
```

After import you can delete the `.p12` file — the certificate and key are
now stored securely in Keychain. Your Python HTTPS client can reference
the Keychain entry instead of the file path.

To verify the import:
```bash
security find-identity -v -p ssl-client
```

You should see your certificate listed with the `ACVTS Demo Client` label.

---

## 6. Security Rules — What Never Goes in the Repo

The following files must never be committed to Git under any circumstances:

| File | Why |
|---|---|
| `.acvp-credentials/client.key` | Private key — compromises your identity if leaked |
| `.acvp-credentials/client.p12` | Contains private key |
| `.acvp_token` | JWT token — grants API access until expiry |
| Any file containing your TOTP seed | Allows anyone to generate valid 2FA codes |
| Your ACVTS password | Obvious |

Ensure your `.gitignore` contains:

```gitignore
# ACVTS credentials — never commit
.acvp-credentials/
.acvp_token
```

What **can** be committed safely:

| File | Why safe |
|---|---|
| `.acvp-credentials/Independent_Bahman_Sistany_Demo.csr` | Contains only your public key — no secrets |
| `.acvp-credentials/Independent_Bahman_Sistany_Demo.cer` | Contains only your signed public certificate |
| `acvp/config/capabilities.json` | Algorithm registration payload — no secrets |

---

## 7. Certificate Lifecycle

TLS client certificates have an expiry date. For the ACVTS demo server
this is typically 1–2 years. When your certificate expires:

1. Generate a new private key and CSR (Step 3 above)
2. Submit the new CSR to NIST
3. Receive and install the new certificate
4. If using Keychain: delete the old entry and import the new one

To check your certificate's expiry:
```bash
openssl x509 -enddate -noout -in .acvp-credentials/Independent_Bahman_Sistany_Demo.cer
```

Set a calendar reminder 30 days before the expiry date.

---

## 8. Troubleshooting

| Symptom | Likely cause | Fix |
|---|---|---|
| TLS handshake failure | Certificate not presented or expired | Check client cert path; verify expiry |
| `401 Unauthorized` | Bad password or expired JWT token | Re-login to get a new token |
| `403 Forbidden` | TOTP code incorrect or clock skew | Check system time; wait for next code |
| `Connection refused` | Wrong URL or port | Confirm `demo.acvts.nist.gov` port 443 |
| TOTP codes not accepted | Clock out of sync | Enable automatic time sync on macOS |

---

## 9. References

| Document | URL |
|---|---|
| ACVTS Demo Server | https://demo.acvts.nist.gov |
| ACVP GitHub (registration guidance) | https://github.com/usnistgov/ACVP |
| RFC 6238 — TOTP | https://www.rfc-editor.org/rfc/rfc6238 |
| RFC 5280 — X.509 Certificates | https://www.rfc-editor.org/rfc/rfc5280 |
| OpenSSL Documentation | https://www.openssl.org/docs/ |

---

**Navigation:** [Documentation Index](../README.md) · Previous: [ACVP Integration Workflow](integration.md)
