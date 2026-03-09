/*
 * test_sha256.c — SHA-256 test vectors
 *
 * Vectors from FIPS 180-4 Appendix B and NIST CAVS examples.
 */

#include <stdio.h>
#include <string.h>
#include "sha256.h"

static int tests_run    = 0;
static int tests_passed = 0;

static void check(const char *label,
                  const uint8_t *got, const uint8_t *expected) {
    tests_run++;
    if (memcmp(got, expected, SHA256_DIGEST_SIZE) == 0) {
        printf("  PASS  %s\n", label);
        tests_passed++;
    } else {
        printf("  FAIL  %s\n", label);
        printf("        got:      ");
        for (int i = 0; i < SHA256_DIGEST_SIZE; i++) printf("%02x", got[i]);
        printf("\n        expected: ");
        for (int i = 0; i < SHA256_DIGEST_SIZE; i++) printf("%02x", expected[i]);
        printf("\n");
    }
}

/* FIPS 180-4 Appendix B.1 — SHA-256("abc") */
static void test_abc(void) {
    const uint8_t expected[32] = {
        0xba,0x78,0x16,0xbf,0x8f,0x01,0xcf,0xea,
        0x41,0x41,0x40,0xde,0x5d,0xae,0x22,0x23,
        0xb0,0x03,0x61,0xa3,0x96,0x17,0x7a,0x9c,
        0xb4,0x10,0xff,0x61,0xf2,0x00,0x15,0xad
    };
    uint8_t digest[32];
    sha256((const uint8_t *)"abc", 3, digest);
    check("SHA-256(\"abc\") FIPS180-4 B.1", digest, expected);
}

/* FIPS 180-4 Appendix B.2 — SHA-256("abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq") */
static void test_448bit(void) {
    const uint8_t expected[32] = {
        0x24,0x8d,0x6a,0x61,0xd2,0x06,0x38,0xb8,
        0xe5,0xc0,0x26,0x93,0x0c,0x3e,0x60,0x39,
        0xa3,0x3c,0xe4,0x59,0x64,0xff,0x21,0x67,
        0xf6,0xec,0xed,0xd4,0x19,0xdb,0x06,0xc1
    };
    uint8_t digest[32];
    const char *msg = "abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq";
    sha256((const uint8_t *)msg, 56, digest);
    check("SHA-256(448-bit msg) FIPS180-4 B.2", digest, expected);
}

/* Empty string */
static void test_empty(void) {
    const uint8_t expected[32] = {
        0xe3,0xb0,0xc4,0x42,0x98,0xfc,0x1c,0x14,
        0x9a,0xfb,0xf4,0xc8,0x99,0x6f,0xb9,0x24,
        0x27,0xae,0x41,0xe4,0x64,0x9b,0x93,0x4c,
        0xa4,0x95,0x99,0x1b,0x78,0x52,0xb8,0x55
    };
    uint8_t digest[32];
    sha256((const uint8_t *)"", 0, digest);
    check("SHA-256(\"\") empty string", digest, expected);
}

/* Multi-block: "a" repeated 1000000 times (FIPS 180-4 Appendix B.3) */
static void test_million_a(void) {
    const uint8_t expected[32] = {
        0xcd,0xc7,0x6e,0x5c,0x99,0x14,0xfb,0x92,
        0x81,0xa1,0xc7,0xe2,0x84,0xd7,0x3e,0x67,
        0xf1,0x80,0x9a,0x48,0xa4,0x97,0x20,0x0e,
        0x04,0x6d,0x39,0xcc,0xc7,0x11,0x2c,0xd0
    };
    uint8_t digest[32];
    SHA256_CTX ctx;
    sha256_init(&ctx);
    uint8_t block[1000];
    memset(block, 'a', sizeof(block));
    for (int i = 0; i < 1000; i++)
        sha256_update(&ctx, block, 1000);
    sha256_final(&ctx, digest);
    check("SHA-256(\"a\" x 1000000) FIPS180-4 B.3", digest, expected);
}

int main(void) {
    printf("=== SHA-256 Tests ===\n");
    test_empty();
    test_abc();
    test_448bit();
    test_million_a();
    printf("\n%d/%d tests passed\n", tests_passed, tests_run);
    return (tests_passed == tests_run) ? 0 : 1;
}
