#ifndef PBKDF2_H
#define PBKDF2_H

/*
 * pbkdf2.h — PBKDF2-HMAC-SHA-256
 *
 * Reference: NIST SP 800-132 (Recommendation for Password-Based Key Derivation)
 * https://csrc.nist.gov/publications/detail/sp/800/132/final
 *
 * Also defined in RFC 8018 Section 5.2.
 *
 * FIPS 140-3 requirements (SP 800-132 Section 5):
 *   - Underlying PRF must be an approved algorithm (HMAC-SHA-256 qualifies)
 *   - Salt must be at least 16 bytes
 *   - Iteration count must be at least 1000
 *   - Output key length must be <= (2^32 - 1) * hLen (effectively unlimited)
 */

#include <stdint.h>
#include <stddef.h>

#define PBKDF2_MIN_SALT_LEN   16    /* bytes — SP 800-132 minimum      */
#define PBKDF2_MIN_ITERATIONS 1000  /* SP 800-132 minimum              */

/*
 * pbkdf2_hmac_sha256 — derive a key from a password
 *
 * password     : raw password bytes
 * password_len : length of password
 * salt         : random salt (must be >= PBKDF2_MIN_SALT_LEN in FIPS mode)
 * salt_len     : length of salt
 * iterations   : iteration count (must be >= PBKDF2_MIN_ITERATIONS in FIPS mode)
 * out          : output buffer for derived key
 * out_len      : desired key length in bytes
 *
 * Returns  0 on success
 *         -1 if parameters violate FIPS constraints (in FIPS mode)
 */
int pbkdf2_hmac_sha256(const uint8_t *password, size_t password_len,
                       const uint8_t *salt,     size_t salt_len,
                       uint32_t       iterations,
                       uint8_t       *out,      size_t out_len);

#endif /* PBKDF2_H */
