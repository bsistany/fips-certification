/*
 * Copyright (c) 2025 Bahman Sistany
 * SPDX-License-Identifier: MIT
 *
 * Part of fips-crypto — a minimal FIPS 140-3 cryptographic library.
 * https://github.com/bsistany/fips-certification
 */

/*
 * fips.c — FIPS mode flag and algorithm approval
 *
 * Reference: FIPS 140-3 Section 7.6
 */

#include "fips.h"
#include "fips_internal.h"

/* ------------------------------------------------------------------ */
/* Module state                                                        */
/* ------------------------------------------------------------------ */
static int fips_mode_on       = 0;  /* 0 = non-approved, 1 = FIPS mode */
static int self_test_ok       = 0;  /* set to 1 after KATs pass         */

/* ------------------------------------------------------------------ */
/* FIPS mode control                                                   */
/* ------------------------------------------------------------------ */
void fips_mode_enable(void)  { fips_mode_on = 1; self_test_ok = 0; }
void fips_mode_disable(void) { fips_mode_on = 0; }
int  fips_mode_active(void)  { return fips_mode_on; }

const char *fips_mode_status(void) {
    if (!fips_mode_on)  return "non-approved";
    if (!self_test_ok)  return "error";
    return "approved";
}

/* ------------------------------------------------------------------ */
/* Self-test state                                                     */
/* ------------------------------------------------------------------ */
void fips_set_self_test_passed(int passed) { self_test_ok = passed; }
int  fips_self_test_passed(void)           { return self_test_ok; }

/* ------------------------------------------------------------------ */
/* Algorithm approval table                                            */
/* ------------------------------------------------------------------ */
static const int approved[] = {
    [ALG_AES_128_CBC]   = 1,
    [ALG_AES_256_CBC]   = 1,
    [ALG_SHA256]        = 1,
    [ALG_HMAC_SHA256]   = 1,
    [ALG_PBKDF2_SHA256] = 1,
    [ALG_MD5]           = 0,
    [ALG_SHA1]          = 0,
    [ALG_DES]           = 0,
    [ALG_RC4]           = 0,
    [ALG_BLOWFISH]      = 0
};

int fips_check_algorithm(fips_algorithm_t alg) {
    if (!fips_mode_on)
        return FIPS_OK;  /* non-approved mode: everything allowed */

    if (!self_test_ok)
        return FIPS_ERR_SELF_TEST;

    if (approved[alg])
        return FIPS_OK;

    return FIPS_ERR_NOT_APPROVED;
}
