#include "provider.h"
#include "secrets.h"
#include <openssl/evp.h>
#include <openssl/core_names.h>
#include <openssl/params.h>

int ss_mac(const unsigned char key[4], const unsigned char *message,
           size_t length, unsigned fault, unsigned char out[32],
           struct ss_ledger *ledger)
{
    unsigned char working[4] = {0}, result[32] = {0};
    size_t written = 0;
    int status = -1;
    EVP_MAC *algorithm = NULL;
    EVP_MAC_CTX *context = NULL;
    struct ss_ledger l = {0};
    char digest[] = "SHA256";
    char absent[] = "CM_NOT_A_DIGEST";
    if (!key || (!message && length) || length > 64 || !out || !ledger ||
        fault > 7)
    {
        return -1;
    }
    for (unsigned i = 0; i < 4; ++i)
    {
        working[i] = key[i];
    }
    if (fault == 1)
    {
        goto done;
    }
    algorithm = EVP_MAC_fetch(NULL, "HMAC", NULL);
    if (!algorithm)
    {
        goto done;
    }
    ++l.fetched;
    if (fault == 2)
    {
        goto done;
    }
    context = EVP_MAC_CTX_new(algorithm);
    if (!context)
    {
        goto done;
    }
    ++l.contexts;
    if (fault == 3)
    {
        goto done;
    }
    OSSL_PARAM params[2];
    params[0] = OSSL_PARAM_construct_utf8_string(
        OSSL_MAC_PARAM_DIGEST, fault == 7 ? absent : digest, 0);
    params[1] = OSSL_PARAM_construct_end();
    if (EVP_MAC_init(context, working, 4, params) != 1)
    {
        goto done;
    }
    if (fault == 4)
    {
        goto done;
    }
    if (length && EVP_MAC_update(context, message, length) != 1)
    {
        goto done;
    }
    if (fault == 5)
    {
        goto done;
    }
    if (EVP_MAC_final(context, result, &written, sizeof result) != 1 ||
        written != 32)
    {
        goto done;
    }
    if (fault == 6)
    {
        goto done;
    }
    for (unsigned i = 0; i < 32; ++i)
    {
        out[i] = result[i];
    }
    status = 0;
done:
    if (context)
    {
        EVP_MAC_CTX_free(context);
        ++l.freed_contexts;
    }
    if (algorithm)
    {
        EVP_MAC_free(algorithm);
        ++l.freed_fetches;
    }
#ifndef SS_MUTANT_KEY
    ss_wipe(working, sizeof working);
#endif
    ss_wipe(result, sizeof result);
    l.key_cleared = 1;
    for (unsigned i = 0; i < 4; ++i)
    {
        if (working[i])
        {
            l.key_cleared = 0;
        }
    }
    *ledger = l;
    return status;
}
