/*
 * Copyright (c) 2025 Bahman Sistany
 * SPDX-License-Identifier: MIT
 *
 * Part of fips-crypto — a minimal FIPS 140-3 cryptographic library.
 * https://github.com/bsistany/fips-certification
 */

/*
 * test_fips_mode.c — FIPS mode flag and algorithm blocking tests
 */

#include <stdio.h>
#include <string.h>
#include "fips.h"
#include "self_test.h"

static int tests_run    = 0;
static int tests_passed = 0;

static void check_int(const char *label, int got, int expected) {
    tests_run++;
    if (got == expected) {
        printf("  PASS  %s\n", label);
        tests_passed++;
    } else {
        printf("  FAIL  %s — got %d, expected %d\n", label, got, expected);
    }
}

static void check_str(const char *label, const char *got, const char *expected) {
    tests_run++;
    if (strcmp(got, expected) == 0) {
        printf("  PASS  %s\n", label);
        tests_passed++;
    } else {
        printf("  FAIL  %s — got \"%s\", expected \"%s\"\n", label, got, expected);
    }
}

int main(void) {
    printf("=== FIPS Mode Tests ===\n");

    /* --- Non-approved mode (default) -------------------------------- */
    check_int("Default mode is off",         fips_mode_active(), 0);
    check_str("Status is non-approved",      fips_mode_status(), "non-approved");

    /* All algorithms allowed in non-approved mode */
    check_int("AES-128 allowed (non-approved mode)", fips_check_algorithm(ALG_AES_128_CBC), FIPS_OK);
    check_int("MD5 allowed (non-approved mode)",     fips_check_algorithm(ALG_MD5),         FIPS_OK);
    check_int("RC4 allowed (non-approved mode)",     fips_check_algorithm(ALG_RC4),         FIPS_OK);

    /* --- FIPS mode on, self-tests not yet passed -------------------- */
    fips_mode_enable();
    check_int("FIPS mode is on",             fips_mode_active(), 1);
    check_str("Status is error (no KATs)",   fips_mode_status(), "error");

    /* All operations blocked until self-tests pass */
    check_int("AES blocked (no KATs)", fips_check_algorithm(ALG_AES_128_CBC), FIPS_ERR_SELF_TEST);
    check_int("MD5 blocked (no KATs)", fips_check_algorithm(ALG_MD5),         FIPS_ERR_SELF_TEST);

    /* --- FIPS mode on, self-tests passed ---------------------------- */
    fips_self_test_run();
    check_str("Status is approved after KATs",  fips_mode_status(), "approved");

    /* Approved algorithms allowed */
    check_int("AES-128-CBC approved",   fips_check_algorithm(ALG_AES_128_CBC),   FIPS_OK);
    check_int("AES-256-CBC approved",   fips_check_algorithm(ALG_AES_256_CBC),   FIPS_OK);
    check_int("SHA-256 approved",       fips_check_algorithm(ALG_SHA256),        FIPS_OK);
    check_int("HMAC-SHA-256 approved",  fips_check_algorithm(ALG_HMAC_SHA256),   FIPS_OK);
    check_int("PBKDF2-SHA-256 approved",fips_check_algorithm(ALG_PBKDF2_SHA256), FIPS_OK);

    /* Non-approved algorithms blocked */
    check_int("MD5 blocked",      fips_check_algorithm(ALG_MD5),      FIPS_ERR_NOT_APPROVED);
    check_int("SHA-1 blocked",    fips_check_algorithm(ALG_SHA1),     FIPS_ERR_NOT_APPROVED);
    check_int("DES blocked",      fips_check_algorithm(ALG_DES),      FIPS_ERR_NOT_APPROVED);
    check_int("RC4 blocked",      fips_check_algorithm(ALG_RC4),      FIPS_ERR_NOT_APPROVED);
    check_int("Blowfish blocked", fips_check_algorithm(ALG_BLOWFISH), FIPS_ERR_NOT_APPROVED);

    /* --- Disable FIPS mode ------------------------------------------ */
    fips_mode_disable();
    check_int("FIPS mode off again",         fips_mode_active(), 0);
    check_int("MD5 allowed again",           fips_check_algorithm(ALG_MD5), FIPS_OK);

    printf("\n%d/%d tests passed\n", tests_passed, tests_run);
    return (tests_passed == tests_run) ? 0 : 1;
}
