/*
 * fips_internal.h — Internal FIPS interface
 *
 * NOT part of the public API. This header is for use only by modules
 * inside the cryptographic boundary that need to set or query the
 * self-test state directly.
 *
 * Do NOT include this file from outside src/.
 *
 * Current internal consumers:
 *   - src/self_test.c  (sets the flag after KATs run)
 */

#ifndef FIPS_INTERNAL_H
#define FIPS_INTERNAL_H

/*
 * fips_set_self_test_passed — set the self-test state flag
 *
 * Called only by self_test.c after all KATs complete.
 * passed = 1 : all KATs passed, module is operational
 * passed = 0 : one or more KATs failed, module enters error state
 */
void fips_set_self_test_passed(int passed);

/*
 * fips_self_test_passed — query the self-test state flag
 *
 * Returns 1 if self-tests have passed, 0 otherwise.
 * External callers should use fips_mode_status() instead.
 */
int fips_self_test_passed(void);

#endif /* FIPS_INTERNAL_H */
