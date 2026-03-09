/*
 * hmac.c — HMAC-SHA-256
 *
 * Reference: FIPS 198-1
 * https://csrc.nist.gov/publications/detail/fips/198/1/final
 *
 * Construction (FIPS 198-1 Section 4):
 *   HMAC(K, text) = H((K0 XOR opad) || H((K0 XOR ipad) || text))
 *
 * Where:
 *   K0   = key padded/hashed to block size (64 bytes)
 *   ipad = 0x36 repeated 64 times
 *   opad = 0x5c repeated 64 times
 */

#include "hmac.h"
#include <string.h>

void hmac_sha256_init(HMAC_SHA256_CTX *ctx,
                      const uint8_t *key, size_t key_len) {
    uint8_t k0[HMAC_SHA256_BLOCK_SIZE];
    uint8_t ipad[HMAC_SHA256_BLOCK_SIZE];
    uint8_t opad[HMAC_SHA256_BLOCK_SIZE];

    memset(k0, 0, sizeof(k0));

    /* FIPS 198-1 Section 4 Step 1:
     * If key longer than block size, hash it. Otherwise zero-pad. */
    if (key_len > HMAC_SHA256_BLOCK_SIZE) {
        sha256(key, key_len, k0);
    } else {
        memcpy(k0, key, key_len);
    }

    /* Steps 2 & 3: XOR with ipad and opad */
    for (int i = 0; i < HMAC_SHA256_BLOCK_SIZE; i++) {
        ipad[i] = k0[i] ^ 0x36;
        opad[i] = k0[i] ^ 0x5c;
    }

    /* Step 4: start inner hash with (K0 XOR ipad) */
    sha256_init(&ctx->inner);
    sha256_update(&ctx->inner, ipad, HMAC_SHA256_BLOCK_SIZE);

    /* Step 6: start outer hash with (K0 XOR opad) */
    sha256_init(&ctx->outer);
    sha256_update(&ctx->outer, opad, HMAC_SHA256_BLOCK_SIZE);

    /* Zeroize local key material */
    volatile uint8_t *p = (volatile uint8_t *)k0;
    for (size_t i = 0; i < sizeof(k0); i++) p[i] = 0;
}

void hmac_sha256_update(HMAC_SHA256_CTX *ctx,
                        const uint8_t *data, size_t len) {
    /* Step 5: feed data into inner hash */
    sha256_update(&ctx->inner, data, len);
}

void hmac_sha256_final(HMAC_SHA256_CTX *ctx,
                       uint8_t mac[HMAC_SHA256_DIGEST_SIZE]) {
    uint8_t inner_digest[SHA256_DIGEST_SIZE];

    /* Step 7: finalise inner hash */
    sha256_final(&ctx->inner, inner_digest);

    /* Step 8: feed inner digest into outer hash and finalise */
    sha256_update(&ctx->outer, inner_digest, SHA256_DIGEST_SIZE);
    sha256_final(&ctx->outer, mac);

    /* Zeroize */
    volatile uint8_t *p = (volatile uint8_t *)inner_digest;
    for (size_t i = 0; i < sizeof(inner_digest); i++) p[i] = 0;
}

void hmac_sha256(const uint8_t *key,  size_t key_len,
                 const uint8_t *data, size_t data_len,
                 uint8_t mac[HMAC_SHA256_DIGEST_SIZE]) {
    HMAC_SHA256_CTX ctx;
    hmac_sha256_init(&ctx, key, key_len);
    hmac_sha256_update(&ctx, data, data_len);
    hmac_sha256_final(&ctx, mac);
}
