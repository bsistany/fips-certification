/*
 * acvp_runner.c — ACVP Runner (Stage 2)
 *
 * Reads an ACVP-format request JSON, invokes the FIPS cryptographic library,
 * and writes an ACVP-format response JSON.
 *
 * Usage:
 *   acvp_runner <request.json> <response.json>
 *
 * Supported algorithms:
 *   ACVP-AES-CBC        (AES-256-CBC encrypt / decrypt)
 *   ACVP-SHA2-256       (SHA-256 AFT)
 *   ACVP-HMAC-SHA2-256  (HMAC-SHA-256, variable keyLen / macLen)
 *   ACVP-KDF-PBKDF2     (PBKDF2-HMAC-SHA-256, variable keyLen)
 *
 * Input JSON contains a local "expected" field per test case (a simulation
 * convention — not part of the real ACVP schema).  The runner compares its
 * output against "expected" and writes "passed": true/false per test case.
 * validate_responses.py performs an independent second check.
 *
 * Build:
 *   See acvp/Makefile — links against src/*.o from the repo root.
 *
 * FIPS notes:
 *   - aes_zeroize() is called after every AES test case (CSP zeroization).
 *   - All intermediate buffers that hold key material are zeroed before free.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <ctype.h>

#include <cjson/cJSON.h>

#include "aes.h"
#include "sha256.h"
#include "hmac.h"
#include "pbkdf2.h"

/* -------------------------------------------------------------------------
 * Constants
 * ---------------------------------------------------------------------- */

#define AES_BLOCK_SIZE      16
#define SHA256_DIGEST_BYTES 32
#define HMAC_DIGEST_BYTES   32   /* HMAC-SHA-256 full output */
#define MAX_PBKDF2_KEY_BYTES 64
#define MAX_PT_CT_BYTES     (256)  /* max AES plaintext/ciphertext per test */
#define MAX_MSG_BYTES       256    /* max SHA-256 / HMAC message per test */

/* -------------------------------------------------------------------------
 * Hex encoding / decoding helpers
 * ---------------------------------------------------------------------- */

/*
 * hex_decode — convert a lowercase hex string to bytes.
 * Returns the number of bytes written, or -1 on error.
 * out must be at least strlen(hex)/2 bytes.
 */
static int hex_decode(const char *hex, uint8_t *out, size_t out_max)
{
    size_t hex_len = strlen(hex);

    if (hex_len % 2 != 0)
        return -1;

    size_t byte_len = hex_len / 2;
    if (byte_len > out_max)
        return -1;

    for (size_t i = 0; i < byte_len; i++) {
        char hi = hex[i * 2];
        char lo = hex[i * 2 + 1];

        if (!isxdigit((unsigned char)hi) || !isxdigit((unsigned char)lo))
            return -1;

        uint8_t hval = (uint8_t)(isdigit((unsigned char)hi)
                       ? hi - '0' : tolower((unsigned char)hi) - 'a' + 10);
        uint8_t lval = (uint8_t)(isdigit((unsigned char)lo)
                       ? lo - '0' : tolower((unsigned char)lo) - 'a' + 10);

        out[i] = (uint8_t)((hval << 4) | lval);
    }

    return (int)byte_len;
}

/*
 * hex_encode — convert bytes to a lowercase hex string.
 * out must be at least len*2 + 1 bytes.
 */
static void hex_encode(const uint8_t *in, size_t len, char *out)
{
    static const char hex_chars[] = "0123456789abcdef";
    for (size_t i = 0; i < len; i++) {
        out[i * 2]     = hex_chars[(in[i] >> 4) & 0x0f];
        out[i * 2 + 1] = hex_chars[ in[i]       & 0x0f];
    }
    out[len * 2] = '\0';
}

/*
 * hex_cmp — case-insensitive comparison of two hex strings.
 * Returns 1 if equal, 0 if not.
 */
static int hex_cmp(const char *a, const char *b)
{
    while (*a && *b) {
        if (tolower((unsigned char)*a) != tolower((unsigned char)*b))
            return 0;
        a++;
        b++;
    }
    return (*a == '\0' && *b == '\0');
}

/* -------------------------------------------------------------------------
 * File I/O helpers
 * ---------------------------------------------------------------------- */

/*
 * read_file — read entire file into a heap-allocated buffer.
 * Caller must free() the returned pointer.
 * Returns NULL on error.
 */
static char *read_file(const char *path)
{
    FILE *f = fopen(path, "rb");
    if (!f) {
        fprintf(stderr, "ERROR: cannot open '%s'\n", path);
        return NULL;
    }

    if (fseek(f, 0, SEEK_END) != 0) {
        fclose(f);
        return NULL;
    }
    long size = ftell(f);
    if (size < 0) {
        fclose(f);
        return NULL;
    }
    rewind(f);

    char *buf = (char *)malloc((size_t)size + 1);
    if (!buf) {
        fclose(f);
        return NULL;
    }

    if (fread(buf, 1, (size_t)size, f) != (size_t)size) {
        free(buf);
        fclose(f);
        return NULL;
    }
    buf[size] = '\0';
    fclose(f);
    return buf;
}

/*
 * write_file — write a string to a file.
 * Returns 0 on success, -1 on error.
 */
static int write_file(const char *path, const char *content)
{
    FILE *f = fopen(path, "wb");
    if (!f) {
        fprintf(stderr, "ERROR: cannot write '%s'\n", path);
        return -1;
    }
    fputs(content, f);
    fclose(f);
    return 0;
}

/* -------------------------------------------------------------------------
 * AES-CBC handler
 * ---------------------------------------------------------------------- */

/*
 * handle_aes_cbc — process all testGroups for ACVP-AES-CBC.
 *
 * Per test:
 *   - Decode key, iv, and pt (encrypt) or ct (decrypt) from hex
 *   - Call aes_init / aes_cbc_encrypt or aes_cbc_decrypt
 *   - Call aes_zeroize (FIPS CSP zeroization — mandatory after each test)
 *   - Encode output to hex
 *   - Compare against "expected"; write "output" and "passed"
 */
static cJSON *handle_aes_cbc(const cJSON *req_groups)
{
    cJSON *resp_groups = cJSON_CreateArray();

    const cJSON *tg = NULL;
    cJSON_ArrayForEach(tg, req_groups) {
        int tg_id = cJSON_GetObjectItem(tg, "tgId")->valueint;
        const char *direction = cJSON_GetObjectItem(tg, "direction")->valuestring;
        int encrypt = (strcmp(direction, "encrypt") == 0);

        cJSON *resp_tg    = cJSON_CreateObject();
        cJSON *resp_tests = cJSON_CreateArray();
        cJSON_AddNumberToObject(resp_tg, "tgId", tg_id);

        const cJSON *tests = cJSON_GetObjectItem(tg, "tests");
        const cJSON *tc    = NULL;

        cJSON_ArrayForEach(tc, tests) {
            int tc_id = cJSON_GetObjectItem(tc, "tcId")->valueint;

            const char *key_hex = cJSON_GetObjectItem(tc, "key")->valuestring;
            const char *iv_hex  = cJSON_GetObjectItem(tc, "iv")->valuestring;
            const char *in_hex  = encrypt
                ? cJSON_GetObjectItem(tc, "pt")->valuestring
                : cJSON_GetObjectItem(tc, "ct")->valuestring;
            const char *expected = cJSON_GetObjectItem(tc, "expected")->valuestring;

            /* Decode inputs */
            uint8_t key[32];
            uint8_t iv[AES_BLOCK_SIZE];
            uint8_t in_buf[MAX_PT_CT_BYTES];
            uint8_t out_buf[MAX_PT_CT_BYTES];

            int key_len = hex_decode(key_hex, key, sizeof(key));
            int iv_len  = hex_decode(iv_hex,  iv,  sizeof(iv));
            int in_len  = hex_decode(in_hex,  in_buf, sizeof(in_buf));

            char out_hex[MAX_PT_CT_BYTES * 2 + 1];
            int  passed = 0;

            if (key_len <= 0 || iv_len != AES_BLOCK_SIZE || in_len <= 0) {
                fprintf(stderr, "  [AES-CBC] tgId=%d tcId=%d: decode error\n",
                        tg_id, tc_id);
                snprintf(out_hex, sizeof(out_hex), "");
            } else {
                AES_CTX ctx;
                aes_init(&ctx, key, (size_t)key_len);

                if (encrypt)
                    aes_cbc_encrypt(&ctx, iv, in_buf, out_buf, (size_t)in_len);
                else
                    aes_cbc_decrypt(&ctx, iv, in_buf, out_buf, (size_t)in_len);

                /* FIPS 140-3: zeroize CSP immediately after use */
                aes_zeroize(&ctx);

                hex_encode(out_buf, (size_t)in_len, out_hex);
                passed = hex_cmp(out_hex, expected);
            }

            /* Zero sensitive stack buffers */
            memset(key,     0, sizeof(key));
            memset(iv,      0, sizeof(iv));
            memset(in_buf,  0, sizeof(in_buf));
            memset(out_buf, 0, sizeof(out_buf));

            cJSON *resp_test = cJSON_CreateObject();
            cJSON_AddNumberToObject(resp_test, "tcId",   tc_id);
            cJSON_AddStringToObject(resp_test, "output", out_hex);
            cJSON_AddBoolToObject  (resp_test, "passed", passed);
            cJSON_AddItemToArray(resp_tests, resp_test);
        }

        cJSON_AddItemToObject(resp_tg, "tests", resp_tests);
        cJSON_AddItemToArray(resp_groups, resp_tg);
    }

    return resp_groups;
}

/* -------------------------------------------------------------------------
 * SHA-256 handler
 * ---------------------------------------------------------------------- */

/*
 * handle_sha256 — process all testGroups for ACVP-SHA2-256.
 *
 * Per test:
 *   - Decode message from hex (empty string → zero-length input)
 *   - Call sha256() one-shot wrapper
 *   - Encode digest to hex
 *   - Compare against "expected"; write "output" and "passed"
 */
static cJSON *handle_sha256(const cJSON *req_groups)
{
    cJSON *resp_groups = cJSON_CreateArray();

    const cJSON *tg = NULL;
    cJSON_ArrayForEach(tg, req_groups) {
        int tg_id = cJSON_GetObjectItem(tg, "tgId")->valueint;

        cJSON *resp_tg    = cJSON_CreateObject();
        cJSON *resp_tests = cJSON_CreateArray();
        cJSON_AddNumberToObject(resp_tg, "tgId", tg_id);

        const cJSON *tests = cJSON_GetObjectItem(tg, "tests");
        const cJSON *tc    = NULL;

        cJSON_ArrayForEach(tc, tests) {
            int tc_id       = cJSON_GetObjectItem(tc, "tcId")->valueint;
            int len_bits    = cJSON_GetObjectItem(tc, "len")->valueint;
            const char *msg_hex  = cJSON_GetObjectItem(tc, "msg")->valuestring;
            const char *expected = cJSON_GetObjectItem(tc, "expected")->valuestring;

            uint8_t msg_buf[MAX_MSG_BYTES];
            uint8_t digest[SHA256_DIGEST_BYTES];
            char    out_hex[SHA256_DIGEST_BYTES * 2 + 1];
            int     passed = 0;

            if (len_bits == 0) {
                /* Empty message — pass NULL/0 to one-shot wrapper */
                sha256(NULL, 0, digest);
                hex_encode(digest, SHA256_DIGEST_BYTES, out_hex);
                passed = hex_cmp(out_hex, expected);
            } else {
                int msg_len = hex_decode(msg_hex, msg_buf, sizeof(msg_buf));
                if (msg_len <= 0) {
                    fprintf(stderr, "  [SHA-256] tgId=%d tcId=%d: decode error\n",
                            tg_id, tc_id);
                    snprintf(out_hex, sizeof(out_hex), "");
                } else {
                    sha256(msg_buf, (size_t)msg_len, digest);
                    hex_encode(digest, SHA256_DIGEST_BYTES, out_hex);
                    passed = hex_cmp(out_hex, expected);
                }
            }

            cJSON *resp_test = cJSON_CreateObject();
            cJSON_AddNumberToObject(resp_test, "tcId",   tc_id);
            cJSON_AddStringToObject(resp_test, "output", out_hex);
            cJSON_AddBoolToObject  (resp_test, "passed", passed);
            cJSON_AddItemToArray(resp_tests, resp_test);
        }

        cJSON_AddItemToObject(resp_tg, "tests", resp_tests);
        cJSON_AddItemToArray(resp_groups, resp_tg);
    }

    return resp_groups;
}

/* -------------------------------------------------------------------------
 * HMAC-SHA-256 handler
 * ---------------------------------------------------------------------- */

/*
 * handle_hmac_sha256 — process all testGroups for ACVP-HMAC-SHA2-256.
 *
 * Per group: macLen is in bits — convert to bytes for truncation.
 * Per test:
 *   - Decode key and msg from hex
 *   - Call hmac_sha256() one-shot wrapper into a 32-byte full-output buffer
 *   - Truncate to macLen bytes (caller's responsibility per the API)
 *   - Encode truncated output to hex
 *   - Compare against "expected"; write "output" and "passed"
 */
static cJSON *handle_hmac_sha256(const cJSON *req_groups)
{
    cJSON *resp_groups = cJSON_CreateArray();

    const cJSON *tg = NULL;
    cJSON_ArrayForEach(tg, req_groups) {
        int tg_id   = cJSON_GetObjectItem(tg, "tgId")->valueint;
        int mac_bits = cJSON_GetObjectItem(tg, "macLen")->valueint;
        int mac_bytes = mac_bits / 8;

        cJSON *resp_tg    = cJSON_CreateObject();
        cJSON *resp_tests = cJSON_CreateArray();
        cJSON_AddNumberToObject(resp_tg, "tgId", tg_id);

        const cJSON *tests = cJSON_GetObjectItem(tg, "tests");
        const cJSON *tc    = NULL;

        cJSON_ArrayForEach(tc, tests) {
            int tc_id        = cJSON_GetObjectItem(tc, "tcId")->valueint;
            const char *key_hex  = cJSON_GetObjectItem(tc, "key")->valuestring;
            const char *msg_hex  = cJSON_GetObjectItem(tc, "msg")->valuestring;
            const char *expected = cJSON_GetObjectItem(tc, "expected")->valuestring;

            uint8_t key_buf[64];
            uint8_t msg_buf[MAX_MSG_BYTES];
            uint8_t full_mac[HMAC_DIGEST_BYTES];
            char    out_hex[HMAC_DIGEST_BYTES * 2 + 1];
            int     passed = 0;

            int key_len = hex_decode(key_hex, key_buf, sizeof(key_buf));
            int msg_len = hex_decode(msg_hex, msg_buf, sizeof(msg_buf));

            if (key_len <= 0 || msg_len < 0) {
                fprintf(stderr, "  [HMAC-SHA-256] tgId=%d tcId=%d: decode error\n",
                        tg_id, tc_id);
                snprintf(out_hex, sizeof(out_hex), "");
            } else {
                hmac_sha256(key_buf, (size_t)key_len,
                            msg_buf, (size_t)msg_len,
                            full_mac);

                /* Truncate to macLen bytes — caller's responsibility */
                hex_encode(full_mac, (size_t)mac_bytes, out_hex);
                passed = hex_cmp(out_hex, expected);
            }

            /* Zero key material */
            memset(key_buf,  0, sizeof(key_buf));
            memset(full_mac, 0, sizeof(full_mac));

            cJSON *resp_test = cJSON_CreateObject();
            cJSON_AddNumberToObject(resp_test, "tcId",   tc_id);
            cJSON_AddStringToObject(resp_test, "output", out_hex);
            cJSON_AddBoolToObject  (resp_test, "passed", passed);
            cJSON_AddItemToArray(resp_tests, resp_test);
        }

        cJSON_AddItemToObject(resp_tg, "tests", resp_tests);
        cJSON_AddItemToArray(resp_groups, resp_tg);
    }

    return resp_groups;
}

/* -------------------------------------------------------------------------
 * PBKDF2 handler
 * ---------------------------------------------------------------------- */

/*
 * handle_pbkdf2 — process all testGroups for ACVP-KDF-PBKDF2.
 *
 * ACVP schema conventions observed here:
 *   - "password" and "salt" are ASCII strings — cast directly to uint8_t*,
 *     use strlen() for length (no hex decoding)
 *   - "keyLen" is in bits — divide by 8 for out_len
 *   - "hashAlg" is informational only (always SHA2-256 in this suite)
 *
 * Per test:
 *   - Extract password, salt, iterationCount, keyLen from JSON
 *   - Call pbkdf2_hmac_sha256()
 *   - Encode derived key to hex
 *   - Compare against "expected"; write "output" and "passed"
 */
static cJSON *handle_pbkdf2(const cJSON *req_groups)
{
    cJSON *resp_groups = cJSON_CreateArray();

    const cJSON *tg = NULL;
    cJSON_ArrayForEach(tg, req_groups) {
        int tg_id = cJSON_GetObjectItem(tg, "tgId")->valueint;

        cJSON *resp_tg    = cJSON_CreateObject();
        cJSON *resp_tests = cJSON_CreateArray();
        cJSON_AddNumberToObject(resp_tg, "tgId", tg_id);

        const cJSON *tests = cJSON_GetObjectItem(tg, "tests");
        const cJSON *tc    = NULL;

        cJSON_ArrayForEach(tc, tests) {
            int tc_id            = cJSON_GetObjectItem(tc, "tcId")->valueint;
            const char *password = cJSON_GetObjectItem(tc, "password")->valuestring;
            const char *salt     = cJSON_GetObjectItem(tc, "salt")->valuestring;
            int iterations       = cJSON_GetObjectItem(tc, "iterationCount")->valueint;
            int key_len_bits     = cJSON_GetObjectItem(tc, "keyLen")->valueint;
            const char *expected = cJSON_GetObjectItem(tc, "expected")->valuestring;

            int key_len_bytes = key_len_bits / 8;

            uint8_t dk[MAX_PBKDF2_KEY_BYTES];
            char    out_hex[MAX_PBKDF2_KEY_BYTES * 2 + 1];
            int     passed = 0;

            if (key_len_bytes <= 0 || key_len_bytes > MAX_PBKDF2_KEY_BYTES) {
                fprintf(stderr, "  [PBKDF2] tgId=%d tcId=%d: keyLen out of range\n",
                        tg_id, tc_id);
                snprintf(out_hex, sizeof(out_hex), "");
            } else {
                /*
                 * password and salt are ASCII strings per ACVP schema.
                 * Cast to uint8_t* and use strlen() — no hex decoding.
                 */
                int rc = pbkdf2_hmac_sha256(
                    (const uint8_t *)password, strlen(password),
                    (const uint8_t *)salt,     strlen(salt),
                    (uint32_t)iterations,
                    dk, (size_t)key_len_bytes
                );

                if (rc != 0) {
                    fprintf(stderr, "  [PBKDF2] tgId=%d tcId=%d: pbkdf2 error %d\n",
                            tg_id, tc_id, rc);
                    snprintf(out_hex, sizeof(out_hex), "");
                } else {
                    hex_encode(dk, (size_t)key_len_bytes, out_hex);
                    passed = hex_cmp(out_hex, expected);
                }
            }

            /* Zero derived key material */
            memset(dk, 0, sizeof(dk));

            cJSON *resp_test = cJSON_CreateObject();
            cJSON_AddNumberToObject(resp_test, "tcId",   tc_id);
            cJSON_AddStringToObject(resp_test, "output", out_hex);
            cJSON_AddBoolToObject  (resp_test, "passed", passed);
            cJSON_AddItemToArray(resp_tests, resp_test);
        }

        cJSON_AddItemToObject(resp_tg, "tests", resp_tests);
        cJSON_AddItemToArray(resp_groups, resp_tg);
    }

    return resp_groups;
}

/* -------------------------------------------------------------------------
 * Main dispatch
 * ---------------------------------------------------------------------- */

int main(int argc, char *argv[])
{
    if (argc != 3) {
        fprintf(stderr, "Usage: acvp_runner <request.json> <response.json>\n");
        return 1;
    }

    const char *req_path  = argv[1];
    const char *resp_path = argv[2];

    /* Read and parse request */
    char *raw = read_file(req_path);
    if (!raw)
        return 1;

    cJSON *req = cJSON_Parse(raw);
    free(raw);
    if (!req) {
        fprintf(stderr, "ERROR: failed to parse JSON from '%s'\n", req_path);
        return 1;
    }

    /* Extract top-level fields */
    cJSON *vs_id_item = cJSON_GetObjectItem(req, "vsId");
    cJSON *algo_item  = cJSON_GetObjectItem(req, "algorithm");
    cJSON *groups     = cJSON_GetObjectItem(req, "testGroups");

    if (!vs_id_item || !algo_item || !groups) {
        fprintf(stderr, "ERROR: missing vsId, algorithm, or testGroups\n");
        cJSON_Delete(req);
        return 1;
    }

    int         vs_id     = vs_id_item->valueint;
    const char *algorithm = algo_item->valuestring;

    fprintf(stdout, "Processing [%s] vsId=%d from '%s'\n",
            algorithm, vs_id, req_path);

    /* Dispatch to algorithm handler */
    cJSON *resp_groups = NULL;

    if (strcmp(algorithm, "ACVP-AES-CBC") == 0) {
        resp_groups = handle_aes_cbc(groups);
    } else if (strcmp(algorithm, "ACVP-SHA2-256") == 0) {
        resp_groups = handle_sha256(groups);
    } else if (strcmp(algorithm, "ACVP-HMAC-SHA2-256") == 0) {
        resp_groups = handle_hmac_sha256(groups);
    } else if (strcmp(algorithm, "ACVP-KDF-PBKDF2") == 0) {
        resp_groups = handle_pbkdf2(groups);
    } else {
        fprintf(stderr, "ERROR: unsupported algorithm '%s'\n", algorithm);
        cJSON_Delete(req);
        return 1;
    }

    /* Build response JSON */
    cJSON *resp = cJSON_CreateObject();
    cJSON_AddNumberToObject(resp, "vsId",       vs_id);
    cJSON_AddStringToObject(resp, "algorithm",  algorithm);
    cJSON_AddItemToObject  (resp, "testGroups", resp_groups);

    /* Serialize and write */
    char *resp_str = cJSON_Print(resp);
    if (!resp_str) {
        fprintf(stderr, "ERROR: failed to serialize response JSON\n");
        cJSON_Delete(req);
        cJSON_Delete(resp);
        return 1;
    }

    int rc = write_file(resp_path, resp_str);

    /* Count pass/fail for summary */
    int total = 0, passed = 0;
    const cJSON *rg = NULL;
    cJSON_ArrayForEach(rg, resp_groups) {
        const cJSON *rt = NULL;
        cJSON_ArrayForEach(rt, cJSON_GetObjectItem(rg, "tests")) {
            total++;
            if (cJSON_IsTrue(cJSON_GetObjectItem(rt, "passed")))
                passed++;
        }
    }

    fprintf(stdout, "  %d/%d passed  →  '%s'\n", passed, total, resp_path);

    free(resp_str);
    cJSON_Delete(req);
    cJSON_Delete(resp);

    return (rc == 0 && passed == total) ? 0 : 1;
}
