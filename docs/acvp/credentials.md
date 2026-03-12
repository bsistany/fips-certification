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
**Body:** Include your name, organisation (or "independent researcher /
learning project"), and intended use (algorithm validation testing).

NIST will reply with instructions and a registration link. Response times
vary — allow 3–10 business days.

### Step 2 — Complete online registration

Follow NIST's registration link. You will:
- Create a username and password
- Be shown a QR code for TOTP setup (do this immediately — see Section 4)
- Receive instructions for submitting a CSR

Do not close the TOTP QR code page until you have scanned it and verified
your authenticator app is generating valid codes.

### Step 3 — Generate your private key and CSR

Run the following from your repo root. This generates a 2048-bit RSA key
pair and a CSR. The private key stays on your machine; only the CSR is
submitted to NIST.

```bash
# Create a directory for credentials (gitignored — see Section 6)
mkdir -p .acvp-credentials

# Generate private key (2048-bit RSA)
openssl genrsa -out .acvp-credentials/client.key 2048

# Generate CSR
openssl req -new \
  -key .acvp-credentials/client.key \
  -out .acvp-credentials/client.csr \
  -subj "/CN=fips-crypto-demo/O=YourName/C=CA"
```

Adjust `/CN=`, `/O=`, and `/C=` to match your identity. NIST may have
specific requirements for these fields — check their registration instructions.

Verify the CSR looks correct:
```bash
openssl req -text -noout -in .acvp-credentials/client.csr
```

### Step 4 — Submit the CSR to NIST

Follow NIST's instructions for CSR submission (typically via their
registration portal or by email). Submit the contents of
`.acvp-credentials/client.csr` — not the private key.

### Step 5 — Receive and store your signed certificate

NIST will return a signed certificate file (typically `client.crt` or
`client.pem`). Save it to `.acvp-credentials/client.crt`.

Verify the certificate:
```bash
openssl x509 -text -noout -in .acvp-credentials/client.crt
```

Check that:
- `Issuer` shows NIST's CA
- `Subject` matches what you put in your CSR
- `Validity` shows the certificate has not expired

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

### Step 2 — Scan the QR code

During registration, NIST displays a QR code. Open your authenticator
app and scan it. The app will add a new entry (typically labelled with
the server name or your username).

**If you miss the QR code:** Contact NIST to have your TOTP seed reset.
There is no way to recover it after the registration page is closed.

### Step 3 — Verify your setup

Wait for the app to show a 6-digit code. In the NIST registration portal,
enter the code to confirm your TOTP is working correctly before finalising
registration.

### Step 4 — Back up your TOTP seed

Most authenticator apps allow you to export or back up your seeds. Do this
now. If you lose access to your authenticator app (lost phone, app reset),
you will be locked out and will need to contact NIST to reset your 2FA.

**Secure backup options:**
- Export the seed to your password manager (1Password, Bitwarden, etc.)
- Print the QR code and store it securely offline

---

## 5. macOS Keychain Integration (Optional but Recommended)

On macOS you can store your TLS client certificate in the system Keychain,
which allows HTTPS clients to use it without referencing a file path.

### Import certificate and key into Keychain

```bash
# Combine private key and certificate into a PKCS#12 bundle
openssl pkcs12 -export \
  -inkey .acvp-credentials/client.key \
  -in    .acvp-credentials/client.crt \
  -out   .acvp-credentials/client.p12 \
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
| `.acvp-credentials/client.csr` | Contains only your public key — no secrets |
| `.acvp-credentials/client.crt` | Contains only your signed public certificate |
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
openssl x509 -enddate -noout -in .acvp-credentials/client.crt
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
