#ifndef SHA256_H
#define SHA256_H

/*
 * sha256.h — SHA-256
 *
 * Reference: FIPS 180-4 (Secure Hash Standard)
 * https://csrc.nist.gov/publications/detail/fips/180/4/final
 *
 * Output: 32 bytes (256 bits)
 * Block size: 64 bytes (512 bits)
 */

#include <stdint.h>
#include <stddef.h>

#define SHA256_DIGEST_SIZE  32  /* bytes */
#define SHA256_BLOCK_SIZE   64  /* bytes */

typedef struct {
    uint32_t state[8];       /* running hash state H0..H7 */
    uint64_t bit_count;      /* total bits processed */
    uint8_t  buf[SHA256_BLOCK_SIZE]; /* partial block buffer */
    size_t   buf_len;        /* bytes currently in buf */
} SHA256_CTX;

/*
 * sha256_init   — initialise context with FIPS 180-4 initial hash values
 * sha256_update — process len bytes of data
 * sha256_final  — output 32-byte digest and zeroize context
 */
void sha256_init  (SHA256_CTX *ctx);
void sha256_update(SHA256_CTX *ctx, const uint8_t *data, size_t len);
void sha256_final (SHA256_CTX *ctx, uint8_t digest[SHA256_DIGEST_SIZE]);

/*
 * sha256 — one-shot convenience wrapper
 */
void sha256(const uint8_t *data, size_t len,
            uint8_t digest[SHA256_DIGEST_SIZE]);

#endif /* SHA256_H */
