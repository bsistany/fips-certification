/*
 * test_hmac.c — HMAC-SHA-256 test vectors
 *
 * Vectors from FIPS 198-1 and RFC 4231.
 */

#include <stdio.h>
#include <string.h>
#include "hmac.h"

static int tests_run    = 0;
static int tests_passed = 0;

static void check(const char *label,
                  const uint8_t *got, const uint8_t *expected) {
    tests_run++;
    if (memcmp(got, expected, HMAC_SHA256_DIGEST_SIZE) == 0) {
        printf("  PASS  %s\n", label);
        tests_passed++;
    } else {
        printf("  FAIL  %s\n", label);
        printf("        got:      ");
        for (int i = 0; i < HMAC_SHA256_DIGEST_SIZE; i++) printf("%02x", got[i]);
        printf("\n        expected: ");
        for (int i = 0; i < HMAC_SHA256_DIGEST_SIZE; i++) printf("%02x", expected[i]);
        printf("\n");
    }
}

/* RFC 4231 Test Case 1 — key shorter than block size */
static void test_rfc4231_tc1(void) {
    const uint8_t key[20] = {
        0x0b,0x0b,0x0b,0x0b,0x0b,0x0b,0x0b,0x0b,
        0x0b,0x0b,0x0b,0x0b,0x0b,0x0b,0x0b,0x0b,
        0x0b,0x0b,0x0b,0x0b
    };
    const uint8_t data[] = "Hi There";
    const uint8_t expected[32] = {
        0xb0,0x34,0x4c,0x61,0xd8,0xdb,0x38,0x53,
        0x5c,0xa8,0xaf,0xce,0xaf,0x0b,0xf1,0x2b,
        0x88,0x1d,0xc2,0x00,0xc9,0x83,0x3d,0xa7,
        0x26,0xe9,0x37,0x6c,0x2e,0x32,0xcf,0xf7
    };
    uint8_t mac[32];
    hmac_sha256(key, 20, data, 8, mac);
    check("RFC4231 TC1 — short key", mac, expected);
}

/* RFC 4231 Test Case 2 — key and data are ASCII */
static void test_rfc4231_tc2(void) {
    const uint8_t key[] = "Jefe";
    const uint8_t data[] = "what do ya want for nothing?";
    const uint8_t expected[32] = {
        0x5b,0xdc,0xc1,0x46,0xbf,0x60,0x75,0x4e,
        0x6a,0x04,0x24,0x26,0x08,0x95,0x75,0xc7,
        0x5a,0x00,0x3f,0x08,0x9d,0x27,0x39,0x83,
        0x9d,0xec,0x58,0xb9,0x64,0xec,0x38,0x43
    };
    uint8_t mac[32];
    hmac_sha256(key, 4, data, 28, mac);
    check("RFC4231 TC2 — ASCII key/data", mac, expected);
}

/* RFC 4231 Test Case 4 — key longer than block size */
static void test_rfc4231_tc4(void) {
    uint8_t key[131];
    memset(key, 0xaa, sizeof(key));
    const uint8_t data[] =
        "This is a test using a larger than block-size key and a "
        "larger than block-size data. The key needs to be hashed "
        "before being used by the HMAC algorithm.";
    const uint8_t expected[32] = {
        0x9b,0x09,0xff,0xa7,0x1b,0x94,0x2f,0xcb,
        0x27,0x63,0x5f,0xbc,0xd5,0xb0,0xe9,0x44,
        0xbf,0xdc,0x63,0x64,0x4f,0x07,0x13,0x93,
        0x8a,0x7f,0x51,0x53,0x5c,0x3a,0x35,0xe2
    };
    uint8_t mac[32];
    hmac_sha256(key, sizeof(key), data, sizeof(data)-1, mac);
    check("RFC4231 TC4 — key longer than block size", mac, expected);
}

/* RFC 4231 Test Case 3 — key of 0xaa bytes, data of 0xdd bytes */
static void test_rfc4231_tc3(void) {
    uint8_t key[20];
    memset(key, 0xaa, sizeof(key));
    uint8_t data[50];
    memset(data, 0xdd, sizeof(data));
    const uint8_t expected[32] = {
        0x77,0x3e,0xa9,0x1e,0x36,0x80,0x0e,0x46,
        0x85,0x4d,0xb8,0xeb,0xd0,0x91,0x81,0xa7,
        0x29,0x59,0x09,0x8b,0x3e,0xf8,0xc1,0x22,
        0xd9,0x63,0x55,0x14,0xce,0xd5,0x65,0xfe
    };
    uint8_t mac[32];
    hmac_sha256(key, sizeof(key), data, sizeof(data), mac);
    check("RFC4231 TC3 — 0xdd data", mac, expected);
}

int main(void) {
    printf("=== HMAC-SHA-256 Tests ===\n");
    test_rfc4231_tc1();
    test_rfc4231_tc2();
    test_rfc4231_tc3();
    test_rfc4231_tc4();
    printf("\n%d/%d tests passed\n", tests_passed, tests_run);
    return (tests_passed == tests_run) ? 0 : 1;
}
