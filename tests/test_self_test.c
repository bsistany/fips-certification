/*
 * test_self_test.c — verify KATs pass and error state works correctly
 *
 * Reference: FIPS 140-3 Section 10.3.1
 */

#include <stdio.h>
#include <string.h>
#include "self_test.h"
#include "fips.h"
#include "aes.h"

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
    printf("=== Self-Test / KAT Tests ===\n");

    /* Enable FIPS mode before running KATs */
    fips_mode_enable();

    /* Before KATs: self-test flag is clear, module is in error state */
    check_int("Self-test not passed before KATs",  fips_self_test_passed(), 0);
    check_str("Status is error before KATs",       fips_mode_status(), "error");
    check_int("AES blocked before KATs",
              fips_check_algorithm(ALG_AES_128_CBC), FIPS_ERR_SELF_TEST);

    /* Run KATs */
    int result = fips_self_test_run();
    check_int("fips_self_test_run() returns 0",    result, 0);
    check_int("Self-test passed after KATs",       fips_self_test_passed(), 1);
    check_str("Status is approved after KATs",     fips_mode_status(), "approved");

    /* Approved algorithms now unblocked */
    check_int("AES-128 unblocked after KATs",
              fips_check_algorithm(ALG_AES_128_CBC), FIPS_OK);
    check_int("SHA-256 unblocked after KATs",
              fips_check_algorithm(ALG_SHA256), FIPS_OK);
    check_int("HMAC-SHA-256 unblocked after KATs",
              fips_check_algorithm(ALG_HMAC_SHA256), FIPS_OK);

    /* Non-approved algorithms still blocked */
    check_int("MD5 still blocked after KATs",
              fips_check_algorithm(ALG_MD5), FIPS_ERR_NOT_APPROVED);

    /* Simulate a KAT failure by manually clearing the flag */
    fips_set_self_test_passed(0);
    check_str("Status is error after flag cleared", fips_mode_status(), "error");
    check_int("AES blocked again after flag cleared",
              fips_check_algorithm(ALG_AES_128_CBC), FIPS_ERR_SELF_TEST);

    printf("\n%d/%d tests passed\n", tests_passed, tests_run);
    return (tests_passed == tests_run) ? 0 : 1;
}
