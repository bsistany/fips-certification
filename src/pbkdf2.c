/*
 * Copyright (c) 2025 Bahman Sistany
 * SPDX-License-Identifier: MIT
 *
 * Part of fips-crypto — a minimal FIPS 140-3 cryptographic library.
 * https://github.com/bsistany/fips-certification
 */

/*
 * pbkdf2.c — PBKDF2-HMAC-SHA-256
 *
 * Reference: NIST SP 800-132, RFC 8018 Section 5.2
 *
 * Construction:
 *   DK = T1 || T2 || ... || Tdklen/hlen
 *   Ti = F(Password, Salt, c, i)
 *   F(Password, Salt, c, i) = U1 XOR U2 XOR ... XOR Uc
 *
 * Where:
 *   U1 = PRF(Password, Salt || INT(i))
 *   Uj = PRF(Password, U(j-1))
 *   PRF = HMAC-SHA-256
 *   c   = iteration count
 *   i   = block index (1-based, big-endian 32-bit)
 */

#include "pbkdf2.h"
#include "hmac.h"
#include "fips.h"
#include <string.h>

#define HLEN SHA256_DIGEST_SIZE  /* 32 bytes */

int pbkdf2_hmac_sha256(const uint8_t *password, size_t password_len,
                       const uint8_t *salt,     size_t salt_len,
                       uint32_t       iterations,
                       uint8_t       *out,      size_t out_len) {
    /* FIPS parameter checks */
    if (fips_mode_active()) {
        if (fips_check_algorithm(ALG_PBKDF2_SHA256) != FIPS_OK)
            return -1;
        if (salt_len < PBKDF2_MIN_SALT_LEN)
            return -1;
        if (iterations < PBKDF2_MIN_ITERATIONS)
            return -1;
    }

    if (iterations == 0 || out_len == 0)
        return -1;

    /* Temporary buffers */
    uint8_t U[HLEN];
    uint8_t T[HLEN];
    uint8_t salt_block[512 + 4];  /* salt || INT(i) — generous bound */

    if (salt_len > 512)
        return -1;  /* guard against overflow */

    memcpy(salt_block, salt, salt_len);

    size_t out_pos   = 0;
    uint32_t block_i = 1;

    while (out_pos < out_len) {
        /* Append big-endian block index INT(i) per RFC 8018 */
        salt_block[salt_len + 0] = (block_i >> 24) & 0xff;
        salt_block[salt_len + 1] = (block_i >> 16) & 0xff;
        salt_block[salt_len + 2] = (block_i >>  8) & 0xff;
        salt_block[salt_len + 3] =  block_i        & 0xff;

        /* U1 = HMAC(password, salt || INT(i)) */
        hmac_sha256(password, password_len,
                    salt_block, salt_len + 4, U);
        memcpy(T, U, HLEN);

        /* U2..Uc — each iteration feeds previous U back in */
        for (uint32_t j = 1; j < iterations; j++) {
            hmac_sha256(password, password_len, U, HLEN, U);
            for (int k = 0; k < HLEN; k++)
                T[k] ^= U[k];
        }

        /* Copy as many bytes as needed from T into output */
        size_t copy = out_len - out_pos;
        if (copy > HLEN) copy = HLEN;
        memcpy(out + out_pos, T, copy);
        out_pos += copy;
        block_i++;
    }

    /* Zeroize local key material */
    volatile uint8_t *p = (volatile uint8_t *)U;
    for (size_t i = 0; i < HLEN; i++) p[i] = 0;
    p = (volatile uint8_t *)T;
    for (size_t i = 0; i < HLEN; i++) p[i] = 0;

    return 0;
}
