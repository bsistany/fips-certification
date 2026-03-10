/*
 * Copyright (c) 2025 Bahman Sistany
 * SPDX-License-Identifier: MIT
 *
 * Part of fips-crypto — a minimal FIPS 140-3 cryptographic library.
 * https://github.com/bsistany/fips-certification
 */

#ifndef SELF_TEST_H
#define SELF_TEST_H

/*
 * self_test.c — Known Answer Tests (KATs) wired to module load
 *
 * Reference: FIPS 140-3 Section 10.3.1 (Power-On Self-Tests)
 * https://csrc.nist.gov/publications/detail/fips/140/3/final
 *
 * Required by FIPS 140-3:
 *   - A KAT must be run for every approved algorithm at module load
 *   - If any KAT fails, the module enters an error state
 *   - In error state, all cryptographic operations must be blocked
 *
 * Usage:
 *   Call fips_self_test_run() once at startup before any crypto operations.
 *   It sets the self-test passed flag in fips.c on success.
 */

/*
 * fips_self_test_run — run all KATs
 *
 * Returns  0 if all KATs pass (module is operational)
 *         -1 if any KAT fails (module enters error state)
 *
 * On failure, fips_mode_status() will return "error" and
 * fips_check_algorithm() will return FIPS_ERR_SELF_TEST for all algorithms.
 */
int fips_self_test_run(void);

#endif /* SELF_TEST_H */
