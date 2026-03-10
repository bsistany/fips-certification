/*
 * Copyright (c) 2025 Bahman Sistany
 * SPDX-License-Identifier: MIT
 *
 * Part of fips-crypto — a minimal FIPS 140-3 cryptographic library.
 * https://github.com/bsistany/fips-certification
 */

#ifndef HMAC_H
#define HMAC_H

/*
 * hmac.h — HMAC-SHA-256
 *
 * Reference: FIPS 198-1 (The Keyed-Hash Message Authentication Code)
 * https://csrc.nist.gov/publications/detail/fips/198/1/final
 *
 * Output: 32 bytes (256 bits)
 * Minimum key length for FIPS 140-3: 14 bytes (112 bits)
 */

#include "sha256.h"
#include <stdint.h>
#include <stddef.h>

#define HMAC_SHA256_DIGEST_SIZE  SHA256_DIGEST_SIZE  /* 32 bytes */
#define HMAC_SHA256_BLOCK_SIZE   SHA256_BLOCK_SIZE   /* 64 bytes */

typedef struct {
    SHA256_CTX inner;
    SHA256_CTX outer;
} HMAC_SHA256_CTX;

/*
 * hmac_sha256_init   — initialise with key
 * hmac_sha256_update — feed data
 * hmac_sha256_final  — produce 32-byte MAC and zeroize context
 */
void hmac_sha256_init  (HMAC_SHA256_CTX *ctx,
                        const uint8_t *key, size_t key_len);
void hmac_sha256_update(HMAC_SHA256_CTX *ctx,
                        const uint8_t *data, size_t len);
void hmac_sha256_final (HMAC_SHA256_CTX *ctx,
                        uint8_t mac[HMAC_SHA256_DIGEST_SIZE]);

/*
 * hmac_sha256 — one-shot convenience wrapper
 */
void hmac_sha256(const uint8_t *key,  size_t key_len,
                 const uint8_t *data, size_t data_len,
                 uint8_t mac[HMAC_SHA256_DIGEST_SIZE]);

#endif /* HMAC_H */
