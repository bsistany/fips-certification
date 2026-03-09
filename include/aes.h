#ifndef AES_H
#define AES_H

/*
 * aes.h — AES-128/256 CBC mode
 *
 * Reference: FIPS 197 (Advanced Encryption Standard)
 *            https://csrc.nist.gov/publications/detail/fips/197/final
 *
 * Supports:
 *   - AES-128 (key = 16 bytes, 10 rounds)
 *   - AES-256 (key = 32 bytes, 14 rounds)
 *   - CBC mode (encrypt and decrypt)
 *
 * Block size is always 16 bytes (128 bits).
 * IV must be exactly 16 bytes.
 * Input must be a multiple of 16 bytes (no padding handled here).
 */

#include <stdint.h>
#include <stddef.h>

#define AES_BLOCK_SIZE   16  /* bytes */
#define AES_128_KEY_SIZE 16  /* bytes */
#define AES_256_KEY_SIZE 32  /* bytes */

/* Maximum round keys: AES-256 needs 15 x 4 = 60 words */
#define AES_MAX_ROUND_KEYS 60

typedef struct {
    uint32_t round_keys[AES_MAX_ROUND_KEYS];
    int      nr;   /* number of rounds: 10 (AES-128) or 14 (AES-256) */
} AES_CTX;

/*
 * aes_init — expand key into round key schedule
 * Returns 0 on success, -1 on invalid key length.
 */
int aes_init(AES_CTX *ctx, const uint8_t *key, size_t key_len);

/*
 * aes_cbc_encrypt / aes_cbc_decrypt
 * len must be a multiple of AES_BLOCK_SIZE.
 * Returns 0 on success, -1 on invalid length.
 */
int aes_cbc_encrypt(const AES_CTX *ctx,
                    const uint8_t iv[AES_BLOCK_SIZE],
                    const uint8_t *in, uint8_t *out, size_t len);

int aes_cbc_decrypt(const AES_CTX *ctx,
                    const uint8_t iv[AES_BLOCK_SIZE],
                    const uint8_t *in, uint8_t *out, size_t len);

/*
 * aes_zeroize — erase key material from context (FIPS 140-3 CSP zeroization)
 */
void aes_zeroize(AES_CTX *ctx);

#endif /* AES_H */
