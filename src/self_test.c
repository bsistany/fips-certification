/*
 * self_test.c — Known Answer Tests (KATs) for all approved algorithms
 *
 * Reference: FIPS 140-3 Section 10.3.1
 *
 * Each KAT uses a fixed input and compares output to a known-good value.
 * Any mismatch sets the module into error state and blocks all operations.
 */

#include "self_test.h"
#include "fips.h"
#include "aes.h"
#include "sha256.h"
#include "hmac.h"
#include <string.h>
#include <stdio.h>

/* ------------------------------------------------------------------ */
/* KAT: AES-128-CBC                                                   */
/* Vector: FIPS 197 Appendix B (single block, zero IV)                */
/* ------------------------------------------------------------------ */
static int kat_aes(void) {
    const uint8_t key[16] = {
        0x2b,0x7e,0x15,0x16,0x28,0xae,0xd2,0xa6,
        0xab,0xf7,0x15,0x88,0x09,0xcf,0x4f,0x3c
    };
    const uint8_t pt[16] = {
        0x32,0x43,0xf6,0xa8,0x88,0x5a,0x30,0x8d,
        0x31,0x31,0x98,0xa2,0xe0,0x37,0x07,0x34
    };
    const uint8_t expected[16] = {
        0x39,0x25,0x84,0x1d,0x02,0xdc,0x09,0xfb,
        0xdc,0x11,0x85,0x97,0x19,0x6a,0x0b,0x32
    };
    const uint8_t iv[16] = {0};

    AES_CTX ctx;
    uint8_t ct[16];

    if (aes_init(&ctx, key, 16) != 0) return -1;
    if (aes_cbc_encrypt(&ctx, iv, pt, ct, 16) != 0) return -1;
    aes_zeroize(&ctx);

    return memcmp(ct, expected, 16) == 0 ? 0 : -1;
}

/* ------------------------------------------------------------------ */
/* KAT: SHA-256                                                        */
/* Vector: FIPS 180-4 Appendix B.1 — SHA-256("abc")                  */
/* ------------------------------------------------------------------ */
static int kat_sha256(void) {
    const uint8_t expected[32] = {
        0xba,0x78,0x16,0xbf,0x8f,0x01,0xcf,0xea,
        0x41,0x41,0x40,0xde,0x5d,0xae,0x22,0x23,
        0xb0,0x03,0x61,0xa3,0x96,0x17,0x7a,0x9c,
        0xb4,0x10,0xff,0x61,0xf2,0x00,0x15,0xad
    };
    uint8_t digest[32];
    sha256((const uint8_t *)"abc", 3, digest);
    return memcmp(digest, expected, 32) == 0 ? 0 : -1;
}

/* ------------------------------------------------------------------ */
/* KAT: HMAC-SHA-256                                                   */
/* Vector: RFC 4231 Test Case 1                                        */
/* ------------------------------------------------------------------ */
static int kat_hmac_sha256(void) {
    const uint8_t key[20] = {
        0x0b,0x0b,0x0b,0x0b,0x0b,0x0b,0x0b,0x0b,
        0x0b,0x0b,0x0b,0x0b,0x0b,0x0b,0x0b,0x0b,
        0x0b,0x0b,0x0b,0x0b
    };
    const uint8_t expected[32] = {
        0xb0,0x34,0x4c,0x61,0xd8,0xdb,0x38,0x53,
        0x5c,0xa8,0xaf,0xce,0xaf,0x0b,0xf1,0x2b,
        0x88,0x1d,0xc2,0x00,0xc9,0x83,0x3d,0xa7,
        0x26,0xe9,0x37,0x6c,0x2e,0x32,0xcf,0xf7
    };
    uint8_t mac[32];
    hmac_sha256(key, 20, (const uint8_t *)"Hi There", 8, mac);
    return memcmp(mac, expected, 32) == 0 ? 0 : -1;
}

/* ------------------------------------------------------------------ */
/* Run all KATs                                                        */
/* ------------------------------------------------------------------ */
int fips_self_test_run(void) {
    int failed = 0;

    /* Mark self-tests as not passed while running */
    fips_set_self_test_passed(0);

    if (kat_aes() != 0) {
        fprintf(stderr, "FIPS KAT FAILED: AES-128-CBC\n");
        failed++;
    }

    if (kat_sha256() != 0) {
        fprintf(stderr, "FIPS KAT FAILED: SHA-256\n");
        failed++;
    }

    if (kat_hmac_sha256() != 0) {
        fprintf(stderr, "FIPS KAT FAILED: HMAC-SHA-256\n");
        failed++;
    }

    if (failed == 0) {
        fips_set_self_test_passed(1);
        return 0;
    }

    /* Module stays in error state — all operations will be blocked */
    return -1;
}
