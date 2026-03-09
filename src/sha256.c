/*
 * sha256.c — SHA-256
 *
 * Reference: FIPS 180-4
 * https://csrc.nist.gov/publications/detail/fips/180/4/final
 */

#include "sha256.h"
#include <string.h>

/* ------------------------------------------------------------------ */
/* FIPS 180-4 Section 4.2.2 — SHA-256 constants                      */
/* First 32 bits of fractional parts of cube roots of first 64 primes */
/* ------------------------------------------------------------------ */
static const uint32_t K[64] = {
    0x428a2f98,0x71374491,0xb5c0fbcf,0xe9b5dba5,
    0x3956c25b,0x59f111f1,0x923f82a4,0xab1c5ed5,
    0xd807aa98,0x12835b01,0x243185be,0x550c7dc3,
    0x72be5d74,0x80deb1fe,0x9bdc06a7,0xc19bf174,
    0xe49b69c1,0xefbe4786,0x0fc19dc6,0x240ca1cc,
    0x2de92c6f,0x4a7484aa,0x5cb0a9dc,0x76f988da,
    0x983e5152,0xa831c66d,0xb00327c8,0xbf597fc7,
    0xc6e00bf3,0xd5a79147,0x06ca6351,0x14292967,
    0x27b70a85,0x2e1b2138,0x4d2c6dfc,0x53380d13,
    0x650a7354,0x766a0abb,0x81c2c92e,0x92722c85,
    0xa2bfe8a1,0xa81a664b,0xc24b8b70,0xc76c51a3,
    0xd192e819,0xd6990624,0xf40e3585,0x106aa070,
    0x19a4c116,0x1e376c08,0x2748774c,0x34b0bcb5,
    0x391c0cb3,0x4ed8aa4a,0x5b9cca4f,0x682e6ff3,
    0x748f82ee,0x78a5636f,0x84c87814,0x8cc70208,
    0x90befffa,0xa4506ceb,0xbef9a3f7,0xc67178f2
};

/* ------------------------------------------------------------------ */
/* FIPS 180-4 Section 4.1.2 — SHA-256 functions and operations        */
/* ------------------------------------------------------------------ */
#define ROTR(x,n)  (((x) >> (n)) | ((x) << (32-(n))))
#define CH(x,y,z)  (((x) & (y)) ^ (~(x) & (z)))
#define MAJ(x,y,z) (((x) & (y)) ^ ((x) & (z)) ^ ((y) & (z)))
#define EP0(x)     (ROTR(x,2)  ^ ROTR(x,13) ^ ROTR(x,22))
#define EP1(x)     (ROTR(x,6)  ^ ROTR(x,11) ^ ROTR(x,25))
#define SIG0(x)    (ROTR(x,7)  ^ ROTR(x,18) ^ ((x) >> 3))
#define SIG1(x)    (ROTR(x,17) ^ ROTR(x,19) ^ ((x) >> 10))

/* Load big-endian 32-bit word */
static inline uint32_t load32be(const uint8_t *p) {
    return ((uint32_t)p[0] << 24) | ((uint32_t)p[1] << 16) |
           ((uint32_t)p[2] <<  8) |  (uint32_t)p[3];
}

/* Store big-endian 32-bit word */
static inline void store32be(uint8_t *p, uint32_t w) {
    p[0]=w>>24; p[1]=w>>16; p[2]=w>>8; p[3]=w;
}

/* Store big-endian 64-bit word */
static inline void store64be(uint8_t *p, uint64_t w) {
    p[0]=w>>56; p[1]=w>>48; p[2]=w>>40; p[3]=w>>32;
    p[4]=w>>24; p[5]=w>>16; p[6]=w>>8;  p[7]=w;
}

/* ------------------------------------------------------------------ */
/* Compress one 512-bit block (FIPS 180-4 Section 6.2.2)              */
/* ------------------------------------------------------------------ */
static void sha256_compress(SHA256_CTX *ctx, const uint8_t block[64]) {
    uint32_t W[64];
    uint32_t a,b,c,d,e,f,g,h,t1,t2;

    /* Prepare message schedule */
    for (int i = 0; i < 16; i++)
        W[i] = load32be(block + 4*i);
    for (int i = 16; i < 64; i++)
        W[i] = SIG1(W[i-2]) + W[i-7] + SIG0(W[i-15]) + W[i-16];

    /* Initialise working variables */
    a=ctx->state[0]; b=ctx->state[1]; c=ctx->state[2]; d=ctx->state[3];
    e=ctx->state[4]; f=ctx->state[5]; g=ctx->state[6]; h=ctx->state[7];

    /* 64 rounds */
    for (int i = 0; i < 64; i++) {
        t1 = h + EP1(e) + CH(e,f,g) + K[i] + W[i];
        t2 = EP0(a) + MAJ(a,b,c);
        h=g; g=f; f=e; e=d+t1;
        d=c; c=b; b=a; a=t1+t2;
    }

    /* Add compressed chunk to current hash */
    ctx->state[0]+=a; ctx->state[1]+=b; ctx->state[2]+=c; ctx->state[3]+=d;
    ctx->state[4]+=e; ctx->state[5]+=f; ctx->state[6]+=g; ctx->state[7]+=h;
}

/* ------------------------------------------------------------------ */
/* Public API                                                          */
/* ------------------------------------------------------------------ */

/* FIPS 180-4 Section 5.3.3 — initial hash values for SHA-256
 * First 32 bits of fractional parts of square roots of first 8 primes */
void sha256_init(SHA256_CTX *ctx) {
    ctx->state[0] = 0x6a09e667;
    ctx->state[1] = 0xbb67ae85;
    ctx->state[2] = 0x3c6ef372;
    ctx->state[3] = 0xa54ff53a;
    ctx->state[4] = 0x510e527f;
    ctx->state[5] = 0x9b05688c;
    ctx->state[6] = 0x1f83d9ab;
    ctx->state[7] = 0x5be0cd19;
    ctx->bit_count = 0;
    ctx->buf_len   = 0;
}

void sha256_update(SHA256_CTX *ctx, const uint8_t *data, size_t len) {
    ctx->bit_count += (uint64_t)len * 8;

    while (len > 0) {
        size_t space = SHA256_BLOCK_SIZE - ctx->buf_len;
        size_t copy  = len < space ? len : space;
        memcpy(ctx->buf + ctx->buf_len, data, copy);
        ctx->buf_len += copy;
        data += copy;
        len  -= copy;

        if (ctx->buf_len == SHA256_BLOCK_SIZE) {
            sha256_compress(ctx, ctx->buf);
            ctx->buf_len = 0;
        }
    }
}

void sha256_final(SHA256_CTX *ctx, uint8_t digest[SHA256_DIGEST_SIZE]) {
    /* Padding: append 0x80 then zeros then 64-bit big-endian bit count */
    ctx->buf[ctx->buf_len++] = 0x80;

    if (ctx->buf_len > 56) {
        /* Not enough room for length — pad to end of block and start new one */
        memset(ctx->buf + ctx->buf_len, 0, SHA256_BLOCK_SIZE - ctx->buf_len);
        sha256_compress(ctx, ctx->buf);
        ctx->buf_len = 0;
    }

    memset(ctx->buf + ctx->buf_len, 0, 56 - ctx->buf_len);
    store64be(ctx->buf + 56, ctx->bit_count);
    sha256_compress(ctx, ctx->buf);

    /* Produce final digest */
    for (int i = 0; i < 8; i++)
        store32be(digest + 4*i, ctx->state[i]);

    /* Zeroize context */
    volatile uint8_t *p = (volatile uint8_t *)ctx;
    for (size_t i = 0; i < sizeof(SHA256_CTX); i++) p[i] = 0;
}

void sha256(const uint8_t *data, size_t len,
            uint8_t digest[SHA256_DIGEST_SIZE]) {
    SHA256_CTX ctx;
    sha256_init(&ctx);
    sha256_update(&ctx, data, len);
    sha256_final(&ctx, digest);
}
