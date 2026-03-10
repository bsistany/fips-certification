/*
 * Copyright (c) 2025 Bahman Sistany
 * SPDX-License-Identifier: MIT
 *
 * Part of fips-crypto — a minimal FIPS 140-3 cryptographic library.
 * https://github.com/bsistany/fips-certification
 */

/*
 * aes.c — AES-128/256 CBC mode, from scratch
 *
 * Reference: FIPS 197
 * https://csrc.nist.gov/publications/detail/fips/197/final
 */

#include "aes.h"
#include <string.h>

/* ------------------------------------------------------------------ */
/* FIPS 197 Figure 7 — S-box                                          */
/* ------------------------------------------------------------------ */
static const uint8_t SBOX[256] = {
    0x63,0x7c,0x77,0x7b,0xf2,0x6b,0x6f,0xc5,0x30,0x01,0x67,0x2b,0xfe,0xd7,0xab,0x76,
    0xca,0x82,0xc9,0x7d,0xfa,0x59,0x47,0xf0,0xad,0xd4,0xa2,0xaf,0x9c,0xa4,0x72,0xc0,
    0xb7,0xfd,0x93,0x26,0x36,0x3f,0xf7,0xcc,0x34,0xa5,0xe5,0xf1,0x71,0xd8,0x31,0x15,
    0x04,0xc7,0x23,0xc3,0x18,0x96,0x05,0x9a,0x07,0x12,0x80,0xe2,0xeb,0x27,0xb2,0x75,
    0x09,0x83,0x2c,0x1a,0x1b,0x6e,0x5a,0xa0,0x52,0x3b,0xd6,0xb3,0x29,0xe3,0x2f,0x84,
    0x53,0xd1,0x00,0xed,0x20,0xfc,0xb1,0x5b,0x6a,0xcb,0xbe,0x39,0x4a,0x4c,0x58,0xcf,
    0xd0,0xef,0xaa,0xfb,0x43,0x4d,0x33,0x85,0x45,0xf9,0x02,0x7f,0x50,0x3c,0x9f,0xa8,
    0x51,0xa3,0x40,0x8f,0x92,0x9d,0x38,0xf5,0xbc,0xb6,0xda,0x21,0x10,0xff,0xf3,0xd2,
    0xcd,0x0c,0x13,0xec,0x5f,0x97,0x44,0x17,0xc4,0xa7,0x7e,0x3d,0x64,0x5d,0x19,0x73,
    0x60,0x81,0x4f,0xdc,0x22,0x2a,0x90,0x88,0x46,0xee,0xb8,0x14,0xde,0x5e,0x0b,0xdb,
    0xe0,0x32,0x3a,0x0a,0x49,0x06,0x24,0x5c,0xc2,0xd3,0xac,0x62,0x91,0x95,0xe4,0x79,
    0xe7,0xc8,0x37,0x6d,0x8d,0xd5,0x4e,0xa9,0x6c,0x56,0xf4,0xea,0x65,0x7a,0xae,0x08,
    0xba,0x78,0x25,0x2e,0x1c,0xa6,0xb4,0xc6,0xe8,0xdd,0x74,0x1f,0x4b,0xbd,0x8b,0x8a,
    0x70,0x3e,0xb5,0x66,0x48,0x03,0xf6,0x0e,0x61,0x35,0x57,0xb9,0x86,0xc1,0x1d,0x9e,
    0xe1,0xf8,0x98,0x11,0x69,0xd9,0x8e,0x94,0x9b,0x1e,0x87,0xe9,0xce,0x55,0x28,0xdf,
    0x8c,0xa1,0x89,0x0d,0xbf,0xe6,0x42,0x68,0x41,0x99,0x2d,0x0f,0xb0,0x54,0xbb,0x16
};

/* FIPS 197 Figure 14 — Inverse S-box */
static const uint8_t INV_SBOX[256] = {
    0x52,0x09,0x6a,0xd5,0x30,0x36,0xa5,0x38,0xbf,0x40,0xa3,0x9e,0x81,0xf3,0xd7,0xfb,
    0x7c,0xe3,0x39,0x82,0x9b,0x2f,0xff,0x87,0x34,0x8e,0x43,0x44,0xc4,0xde,0xe9,0xcb,
    0x54,0x7b,0x94,0x32,0xa6,0xc2,0x23,0x3d,0xee,0x4c,0x95,0x0b,0x42,0xfa,0xc3,0x4e,
    0x08,0x2e,0xa1,0x66,0x28,0xd9,0x24,0xb2,0x76,0x5b,0xa2,0x49,0x6d,0x8b,0xd1,0x25,
    0x72,0xf8,0xf6,0x64,0x86,0x68,0x98,0x16,0xd4,0xa4,0x5c,0xcc,0x5d,0x65,0xb6,0x92,
    0x6c,0x70,0x48,0x50,0xfd,0xed,0xb9,0xda,0x5e,0x15,0x46,0x57,0xa7,0x8d,0x9d,0x84,
    0x90,0xd8,0xab,0x00,0x8c,0xbc,0xd3,0x0a,0xf7,0xe4,0x58,0x05,0xb8,0xb3,0x45,0x06,
    0xd0,0x2c,0x1e,0x8f,0xca,0x3f,0x0f,0x02,0xc1,0xaf,0xbd,0x03,0x01,0x13,0x8a,0x6b,
    0x3a,0x91,0x11,0x41,0x4f,0x67,0xdc,0xea,0x97,0xf2,0xcf,0xce,0xf0,0xb4,0xe6,0x73,
    0x96,0xac,0x74,0x22,0xe7,0xad,0x35,0x85,0xe2,0xf9,0x37,0xe8,0x1c,0x75,0xdf,0x6e,
    0x47,0xf1,0x1a,0x71,0x1d,0x29,0xc5,0x89,0x6f,0xb7,0x62,0x0e,0xaa,0x18,0xbe,0x1b,
    0xfc,0x56,0x3e,0x4b,0xc6,0xd2,0x79,0x20,0x9a,0xdb,0xc0,0xfe,0x78,0xcd,0x5a,0xf4,
    0x1f,0xdd,0xa8,0x33,0x88,0x07,0xc7,0x31,0xb1,0x12,0x10,0x59,0x27,0x80,0xec,0x5f,
    0x60,0x51,0x7f,0xa9,0x19,0xb5,0x4a,0x0d,0x2d,0xe5,0x7a,0x9f,0x93,0xc9,0x9c,0xef,
    0xa0,0xe0,0x3b,0x4d,0xae,0x2a,0xf5,0xb0,0xc8,0xeb,0xbb,0x3c,0x83,0x53,0x99,0x61,
    0x17,0x2b,0x04,0x7e,0xba,0x77,0xd6,0x26,0xe1,0x69,0x14,0x63,0x55,0x21,0x0c,0x7d
};

/* Round constants for key schedule (FIPS 197 Section 5.2) */
static const uint8_t RCON[11] = {
    0x00, 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0x1b, 0x36
};

/* ------------------------------------------------------------------ */
/* Internal helpers                                                    */
/* ------------------------------------------------------------------ */

/* XTime: multiply by 2 in GF(2^8) per FIPS 197 Section 4.2 */
static inline uint8_t xtime(uint8_t x) {
    return (x << 1) ^ ((x >> 7) ? 0x1b : 0x00);
}

/* GF multiply */
static inline uint8_t gmul(uint8_t a, uint8_t b) {
    uint8_t p = 0;
    for (int i = 0; i < 8; i++) {
        if (b & 1) p ^= a;
        uint8_t hi = a & 0x80;
        a <<= 1;
        if (hi) a ^= 0x1b;
        b >>= 1;
    }
    return p;
}

/* Load 4 bytes as a big-endian 32-bit word */
static inline uint32_t load32(const uint8_t *b) {
    return ((uint32_t)b[0] << 24) | ((uint32_t)b[1] << 16) |
           ((uint32_t)b[2] <<  8) |  (uint32_t)b[3];
}

/* Store 32-bit word as 4 big-endian bytes */
static inline void store32(uint8_t *b, uint32_t w) {
    b[0] = w >> 24; b[1] = w >> 16; b[2] = w >> 8; b[3] = w;
}

/* SubWord: apply S-box to each byte of a word */
static inline uint32_t sub_word(uint32_t w) {
    return ((uint32_t)SBOX[w >> 24]         << 24) |
           ((uint32_t)SBOX[(w >> 16) & 0xff] << 16) |
           ((uint32_t)SBOX[(w >>  8) & 0xff] <<  8) |
            (uint32_t)SBOX[w & 0xff];
}

/* RotWord: rotate word left by 8 bits */
static inline uint32_t rot_word(uint32_t w) {
    return (w << 8) | (w >> 24);
}

/* ------------------------------------------------------------------ */
/* Key Expansion (FIPS 197 Section 5.2)                               */
/* ------------------------------------------------------------------ */
int aes_init(AES_CTX *ctx, const uint8_t *key, size_t key_len) {
    int nk; /* key length in 32-bit words */

    if (key_len == AES_128_KEY_SIZE) {
        nk = 4; ctx->nr = 10;
    } else if (key_len == AES_256_KEY_SIZE) {
        nk = 8; ctx->nr = 14;
    } else {
        return -1;
    }

    int total = 4 * (ctx->nr + 1); /* total words in expanded key */

    /* Copy original key words */
    for (int i = 0; i < nk; i++)
        ctx->round_keys[i] = load32(key + 4 * i);

    /* Expand */
    for (int i = nk; i < total; i++) {
        uint32_t temp = ctx->round_keys[i - 1];
        if (i % nk == 0)
            temp = sub_word(rot_word(temp)) ^ ((uint32_t)RCON[i / nk] << 24);
        else if (nk > 6 && i % nk == 4)
            temp = sub_word(temp);
        ctx->round_keys[i] = ctx->round_keys[i - nk] ^ temp;
    }
    return 0;
}

/* ------------------------------------------------------------------ */
/* AES block encrypt (FIPS 197 Section 5.1)                           */
/* ------------------------------------------------------------------ */
static void aes_encrypt_block(const AES_CTX *ctx,
                               const uint8_t in[16], uint8_t out[16]) {
    uint8_t s[4][4];
    int r, c;

    /* Load state column-major */
    for (r = 0; r < 4; r++)
        for (c = 0; c < 4; c++)
            s[r][c] = in[r + 4 * c];

    /* AddRoundKey — round 0 */
    for (c = 0; c < 4; c++) {
        uint32_t rk = ctx->round_keys[c];
        s[0][c] ^= rk >> 24; s[1][c] ^= rk >> 16;
        s[2][c] ^= rk >> 8;  s[3][c] ^= rk;
    }

    for (int round = 1; round <= ctx->nr; round++) {
        /* SubBytes */
        for (r = 0; r < 4; r++)
            for (c = 0; c < 4; c++)
                s[r][c] = SBOX[s[r][c]];

        /* ShiftRows */
        uint8_t tmp;
        tmp = s[1][0]; s[1][0]=s[1][1]; s[1][1]=s[1][2]; s[1][2]=s[1][3]; s[1][3]=tmp;
        tmp = s[2][0]; s[2][0]=s[2][2]; s[2][2]=tmp; tmp=s[2][1]; s[2][1]=s[2][3]; s[2][3]=tmp;
        tmp = s[3][3]; s[3][3]=s[3][2]; s[3][2]=s[3][1]; s[3][1]=s[3][0]; s[3][0]=tmp;

        /* MixColumns (skip on final round) */
        if (round < ctx->nr) {
            for (c = 0; c < 4; c++) {
                uint8_t a0=s[0][c], a1=s[1][c], a2=s[2][c], a3=s[3][c];
                s[0][c] = gmul(a0,2)^gmul(a1,3)^a2^a3;
                s[1][c] = a0^gmul(a1,2)^gmul(a2,3)^a3;
                s[2][c] = a0^a1^gmul(a2,2)^gmul(a3,3);
                s[3][c] = gmul(a0,3)^a1^a2^gmul(a3,2);
            }
        }

        /* AddRoundKey */
        int base = round * 4;
        for (c = 0; c < 4; c++) {
            uint32_t rk = ctx->round_keys[base + c];
            s[0][c] ^= rk >> 24; s[1][c] ^= rk >> 16;
            s[2][c] ^= rk >> 8;  s[3][c] ^= rk;
        }
    }

    /* Store state */
    for (r = 0; r < 4; r++)
        for (c = 0; c < 4; c++)
            out[r + 4 * c] = s[r][c];
}

/* ------------------------------------------------------------------ */
/* AES block decrypt (FIPS 197 Section 5.3)                           */
/* ------------------------------------------------------------------ */
static void aes_decrypt_block(const AES_CTX *ctx,
                               const uint8_t in[16], uint8_t out[16]) {
    uint8_t s[4][4];
    int r, c;

    for (r = 0; r < 4; r++)
        for (c = 0; c < 4; c++)
            s[r][c] = in[r + 4 * c];

    /* AddRoundKey — last round key */
    int base = ctx->nr * 4;
    for (c = 0; c < 4; c++) {
        uint32_t rk = ctx->round_keys[base + c];
        s[0][c] ^= rk >> 24; s[1][c] ^= rk >> 16;
        s[2][c] ^= rk >> 8;  s[3][c] ^= rk;
    }

    for (int round = ctx->nr - 1; round >= 0; round--) {
        /* InvShiftRows */
        uint8_t tmp;
        tmp = s[1][3]; s[1][3]=s[1][2]; s[1][2]=s[1][1]; s[1][1]=s[1][0]; s[1][0]=tmp;
        tmp = s[2][0]; s[2][0]=s[2][2]; s[2][2]=tmp; tmp=s[2][1]; s[2][1]=s[2][3]; s[2][3]=tmp;
        tmp = s[3][0]; s[3][0]=s[3][1]; s[3][1]=s[3][2]; s[3][2]=s[3][3]; s[3][3]=tmp;

        /* InvSubBytes */
        for (r = 0; r < 4; r++)
            for (c = 0; c < 4; c++)
                s[r][c] = INV_SBOX[s[r][c]];

        /* AddRoundKey */
        base = round * 4;
        for (c = 0; c < 4; c++) {
            uint32_t rk = ctx->round_keys[base + c];
            s[0][c] ^= rk >> 24; s[1][c] ^= rk >> 16;
            s[2][c] ^= rk >> 8;  s[3][c] ^= rk;
        }

        /* InvMixColumns (skip on round 0) */
        if (round > 0) {
            for (c = 0; c < 4; c++) {
                uint8_t a0=s[0][c], a1=s[1][c], a2=s[2][c], a3=s[3][c];
                s[0][c] = gmul(a0,0x0e)^gmul(a1,0x0b)^gmul(a2,0x0d)^gmul(a3,0x09);
                s[1][c] = gmul(a0,0x09)^gmul(a1,0x0e)^gmul(a2,0x0b)^gmul(a3,0x0d);
                s[2][c] = gmul(a0,0x0d)^gmul(a1,0x09)^gmul(a2,0x0e)^gmul(a3,0x0b);
                s[3][c] = gmul(a0,0x0b)^gmul(a1,0x0d)^gmul(a2,0x09)^gmul(a3,0x0e);
            }
        }
    }

    for (r = 0; r < 4; r++)
        for (c = 0; c < 4; c++)
            out[r + 4 * c] = s[r][c];
}

/* ------------------------------------------------------------------ */
/* CBC mode                                                            */
/* ------------------------------------------------------------------ */
int aes_cbc_encrypt(const AES_CTX *ctx,
                    const uint8_t iv[AES_BLOCK_SIZE],
                    const uint8_t *in, uint8_t *out, size_t len) {
    if (len % AES_BLOCK_SIZE != 0) return -1;

    uint8_t prev[AES_BLOCK_SIZE];
    memcpy(prev, iv, AES_BLOCK_SIZE);

    for (size_t i = 0; i < len; i += AES_BLOCK_SIZE) {
        uint8_t tmp[AES_BLOCK_SIZE];
        for (int j = 0; j < AES_BLOCK_SIZE; j++)
            tmp[j] = in[i + j] ^ prev[j];
        aes_encrypt_block(ctx, tmp, out + i);
        memcpy(prev, out + i, AES_BLOCK_SIZE);
    }
    return 0;
}

int aes_cbc_decrypt(const AES_CTX *ctx,
                    const uint8_t iv[AES_BLOCK_SIZE],
                    const uint8_t *in, uint8_t *out, size_t len) {
    if (len % AES_BLOCK_SIZE != 0) return -1;

    uint8_t prev[AES_BLOCK_SIZE];
    memcpy(prev, iv, AES_BLOCK_SIZE);

    for (size_t i = 0; i < len; i += AES_BLOCK_SIZE) {
        uint8_t tmp[AES_BLOCK_SIZE];
        aes_decrypt_block(ctx, in + i, tmp);
        for (int j = 0; j < AES_BLOCK_SIZE; j++)
            out[i + j] = tmp[j] ^ prev[j];
        memcpy(prev, in + i, AES_BLOCK_SIZE);
    }
    return 0;
}

/* ------------------------------------------------------------------ */
/* Zeroization (FIPS 140-3 CSP requirement)                           */
/* ------------------------------------------------------------------ */
void aes_zeroize(AES_CTX *ctx) {
    volatile uint8_t *p = (volatile uint8_t *)ctx;
    for (size_t i = 0; i < sizeof(AES_CTX); i++)
        p[i] = 0;
}
