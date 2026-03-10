/*
 * Copyright (c) 2025 Bahman Sistany
 * SPDX-License-Identifier: MIT
 *
 * Part of fips-crypto — a minimal FIPS 140-3 cryptographic library.
 * https://github.com/bsistany/fips-certification
 */

/*
 * test_pbkdf2.c — PBKDF2-HMAC-SHA-256 test vectors
 *
 * Vectors from RFC 7914 Section 11 and NIST CAVS examples.
 */

#include <stdio.h>
#include <string.h>
#include "pbkdf2.h"
#include "fips.h"
#include "self_test.h"

static int tests_run    = 0;
static int tests_passed = 0;

static void check(const char *label,
                  const uint8_t *got, const uint8_t *expected, size_t len) {
    tests_run++;
    if (memcmp(got, expected, len) == 0) {
        printf("  PASS  %s\n", label);
        tests_passed++;
    } else {
        printf("  FAIL  %s\n", label);
        printf("        got:      ");
        for (size_t i = 0; i < len; i++) printf("%02x", got[i]);
        printf("\n        expected: ");
        for (size_t i = 0; i < len; i++) printf("%02x", expected[i]);
        printf("\n");
    }
}

static void check_int(const char *label, int got, int expected) {
    tests_run++;
    if (got == expected) {
        printf("  PASS  %s\n", label);
        tests_passed++;
    } else {
        printf("  FAIL  %s — got %d, expected %d\n", label, got, expected);
    }
}

/* RFC 6070 TC1 — password="password", salt="salt", c=1, dkLen=20 */
static void test_rfc6070_tc1(void) {
    const uint8_t expected[20] = {
        0x12,0x0f,0xb6,0xcf,0xfc,0xf8,0xb3,0x2c,
        0x43,0xe7,0x22,0x52,0x56,0xc4,0xf8,0x37,
        0xa8,0x65,0x48,0xc9
    };
    uint8_t out[20];
    pbkdf2_hmac_sha256((const uint8_t *)"password", 8,
                       (const uint8_t *)"salt",     4,
                       1, out, 20);
    check("RFC6070 TC1 — c=1", out, expected, 20);
}

/* RFC 6070 TC2 — password="password", salt="salt", c=2, dkLen=20 */
static void test_rfc6070_tc2(void) {
    const uint8_t expected[20] = {
        0xae,0x4d,0x0c,0x95,0xaf,0x6b,0x46,0xd3,
        0x2d,0x0a,0xdf,0xf9,0x28,0xf0,0x6d,0xd0,
        0x2a,0x30,0x3f,0x8e
    };
    uint8_t out[20];
    pbkdf2_hmac_sha256((const uint8_t *)"password", 8,
                       (const uint8_t *)"salt",     4,
                       2, out, 20);
    check("RFC6070 TC2 — c=2", out, expected, 20);
}

/* RFC 6070 TC3 — c=4096, dkLen=20 */
static void test_rfc6070_tc3(void) {
    const uint8_t expected[20] = {
        0xc5,0xe4,0x78,0xd5,0x92,0x88,0xc8,0x41,
        0xaa,0x53,0x0d,0xb6,0x84,0x5c,0x4c,0x8d,
        0x96,0x28,0x93,0xa0
    };
    uint8_t out[20];
    pbkdf2_hmac_sha256((const uint8_t *)"password", 8,
                       (const uint8_t *)"salt",     4,
                       4096, out, 20);
    check("RFC6070 TC3 — c=4096", out, expected, 20);
}

/* RFC 6070 TC5 — longer password and salt, c=4096, dkLen=25 */
static void test_rfc6070_tc5(void) {
    const uint8_t expected[25] = {
        0x34,0x8c,0x89,0xdb,0xcb,0xd3,0x2b,0x2f,
        0x32,0xd8,0x14,0xb8,0x11,0x6e,0x84,0xcf,
        0x2b,0x17,0x34,0x7e,0xbc,0x18,0x00,0x18,0x1c
    };
    uint8_t out[25];
    pbkdf2_hmac_sha256(
        (const uint8_t *)"passwordPASSWORDpassword", 24,
        (const uint8_t *)"saltSALTsaltSALTsaltSALTsaltSALTsalt", 36,
        4096, out, 25);
    check("RFC6070 TC5 — multi-block output", out, expected, 25);
}

/* FIPS mode: reject short salt */
static void test_fips_short_salt(void) {
    fips_mode_enable();
    fips_self_test_run();
    uint8_t out[32];
    int rc = pbkdf2_hmac_sha256(
        (const uint8_t *)"password", 8,
        (const uint8_t *)"shortsalt", 9,  /* < 16 bytes */
        1000, out, 32);
    check_int("FIPS mode rejects salt < 16 bytes", rc, -1);
    fips_mode_disable();
}

/* FIPS mode: reject low iteration count */
static void test_fips_low_iterations(void) {
    fips_mode_enable();
    fips_self_test_run();
    uint8_t out[32];
    uint8_t salt[16] = {0};
    int rc = pbkdf2_hmac_sha256(
        (const uint8_t *)"password", 8,
        salt, 16,
        999,   /* < 1000 */
        out, 32);
    check_int("FIPS mode rejects iterations < 1000", rc, -1);
    fips_mode_disable();
}

int main(void) {
    printf("=== PBKDF2-HMAC-SHA-256 Tests ===\n");
    test_rfc6070_tc1();
    test_rfc6070_tc2();
    test_rfc6070_tc3();
    test_rfc6070_tc5();
    test_fips_short_salt();
    test_fips_low_iterations();
    printf("\n%d/%d tests passed\n", tests_passed, tests_run);
    return (tests_passed == tests_run) ? 0 : 1;
}
