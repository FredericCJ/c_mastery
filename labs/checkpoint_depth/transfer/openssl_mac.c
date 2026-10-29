/* Supplied hosted primitive adapter: protocol composition, not SHA implementation. */
#include "auth.h"
#include <openssl/evp.h>
#include <openssl/crypto.h>

int cpx_openssl_mac(const unsigned char *key, size_t key_length,
                    const unsigned char *bytes, size_t n, unsigned char tag[32])
{
    size_t written = 0;
    return EVP_Q_mac(NULL, "HMAC", NULL, "SHA256", NULL, key, key_length, bytes,
                     n, tag, 32, &written) != NULL &&
           written == 32;
}

int cpx_tag_equal(const unsigned char a[32], const unsigned char b[32])
{
    return CRYPTO_memcmp(a, b, 32) == 0;
}
