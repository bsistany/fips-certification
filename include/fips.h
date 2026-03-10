/*
 * Copyright (c) 2025 Bahman Sistany
 * SPDX-License-Identifier: MIT
 *
 * Part of fips-crypto — a minimal FIPS 140-3 cryptographic library.
 * https://github.com/bsistany/fips-certification
 */

#ifndef FIPS_H
#define FIPS_H

/*
 * fips.h — FIPS mode flag and error handling
 *
 * Reference: FIPS 140-3 Section 7.6 (Operational Environment)
 * https://csrc.nist.gov/publications/detail/fips/140/3/final
 *
 * When FIPS mode is enabled:
 *   - Approved algorithms proceed normally
 *   - Non-approved algorithms return FIPS_ERR_NOT_APPROVED
 *   - If self-tests have not passed, all operations return FIPS_ERR_SELF_TEST
 */

#include <stdint.h>

/* ------------------------------------------------------------------ */
/* Error codes                                                         */
/* ------------------------------------------------------------------ */
#define FIPS_OK                0   /* operation allowed                */
#define FIPS_ERR_NOT_APPROVED -1   /* algorithm not approved in FIPS mode */
#define FIPS_ERR_SELF_TEST    -2   /* self-tests not yet passed        */

/* ------------------------------------------------------------------ */
/* FIPS mode control                                                   */
/* ------------------------------------------------------------------ */

/*
 * fips_mode_enable  — turn FIPS mode on
 * fips_mode_disable — turn FIPS mode off (testing/development only)
 * fips_mode_active  — returns 1 if FIPS mode is on, 0 otherwise
 */
void fips_mode_enable (void);
void fips_mode_disable(void);
int  fips_mode_active (void);

/*
 * fips_mode_status — returns a human-readable string:
 *   "approved"      if FIPS mode is on and self-tests passed
 *   "non-approved"  if FIPS mode is off
 *   "error"         if self-tests failed
 */
const char *fips_mode_status(void);

/* ------------------------------------------------------------------ */
/* Algorithm approval check                                           */
/* ------------------------------------------------------------------ */

/* Algorithm identifiers */
typedef enum {
    /* Approved */
    ALG_AES_128_CBC = 0,
    ALG_AES_256_CBC,
    ALG_SHA256,
    ALG_HMAC_SHA256,
    ALG_PBKDF2_SHA256,

    /* Non-approved */
    ALG_MD5,
    ALG_SHA1,
    ALG_DES,
    ALG_RC4,
    ALG_BLOWFISH
} fips_algorithm_t;

/*
 * fips_check_algorithm — call before using any algorithm
 *
 * Returns FIPS_OK if the algorithm is allowed in the current mode.
 * Returns FIPS_ERR_NOT_APPROVED if FIPS mode is on and algorithm is not approved.
 * Returns FIPS_ERR_SELF_TEST if self-tests have not passed.
 */
int fips_check_algorithm(fips_algorithm_t alg);

#endif /* FIPS_H */
