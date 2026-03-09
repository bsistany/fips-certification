#!/bin/bash
# Run from inside your cloned fips-certification repo folder
# bash setup.sh

mkdir -p src include tests docs

for f in src/aes.c src/sha256.c src/hmac.c src/fips.c src/self_test.c src/pbkdf2.c; do echo "/* TODO */" > "$f"; done
for f in include/aes.h include/sha256.h include/hmac.h include/fips.h include/self_test.h include/pbkdf2.h; do echo "/* TODO */" > "$f"; done
for f in tests/test_aes.c tests/test_sha256.c tests/test_hmac.c tests/test_fips_mode.c tests/test_self_test.c tests/test_pbkdf2.c; do echo "/* TODO */" > "$f"; done
for f in docs/boundary.md docs/algorithm-inventory.md docs/security-policy.md; do echo "# TODO" > "$f"; done

echo "Done. Structure created."
