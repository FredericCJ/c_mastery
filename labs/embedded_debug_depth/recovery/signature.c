/* Hosted integration only. The deliberately public synthetic seed below is
   not a credential and is never linked into a target verifier image. */
#include "recovery.h"
#include "signature_adapter.h"
#include <openssl/evp.h>
#include <stdio.h>
#include <string.h>
#ifndef DU_SIGNATURE_MUTANT
#define DU_SIGNATURE_MUTANT 0
#endif
#define CHECK(x)                                                \
    do                                                          \
    {                                                           \
        if (!(x))                                               \
        {                                                       \
            printf("DU_SIGNATURE_CHECK:%u:%s\n", __LINE__, #x); \
            return 1;                                           \
        }                                                       \
    } while (0)

int main(void)
{
    unsigned char seed[32];
    for (unsigned i = 0; i < 32; i++)
    {
        seed[i] = (unsigned char)(i + 1);
    }
    EVP_PKEY *key =
        EVP_PKEY_new_raw_private_key(EVP_PKEY_ED25519, NULL, seed, 32);
    CHECK(key);
    struct lce_keyring ring = {{{0}}, 2};
    size_t z = 32;
    CHECK(EVP_PKEY_get_raw_public_key(key, ring.public_keys[0], &z) == 1 &&
          z == 32);
    EVP_MD_CTX *ctx = EVP_MD_CTX_new();
    CHECK(ctx);
    unsigned char old[9] = {3, 0, 0, 0, 1, 42, 43, 44, 45}, signature[64];
    z = sizeof signature;
    CHECK(EVP_DigestSignInit(ctx, NULL, NULL, NULL, key) == 1 &&
          EVP_DigestSign(ctx, signature, &z, old, sizeof old) == 1 && z == 64);
    EVP_MD_CTX_free(ctx);
    EVP_PKEY_free(key);
    uint32_t version = 99;
    CHECK(lce_admit(old, sizeof old, signature, sizeof signature, 2, 0, 3,
                    lce_signature_bridge, &ring, &version) &&
          version == 3);
    /* The old signature remains cryptographically valid. Current trusted floor
       is a distinct admission condition; teaching mutant deliberately drops it. */
    CHECK(lce_signature_bridge(&ring, 1, old, sizeof old, signature,
                               sizeof signature) == 1);
    version = 99;
    CHECK(!lce_admit(old, sizeof old, signature, sizeof signature, 2, 0,
                     DU_SIGNATURE_MUTANT == 1 ? 1 : 4, lce_signature_bridge,
                     &ring, &version) &&
          version == 99);
    old[5] ^= 1;
    CHECK(!lce_admit(old, sizeof old, signature, sizeof signature, 2, 0, 1,
                     lce_signature_bridge, &ring, &version) &&
          version == 99);
    old[5] ^= 1;
    CHECK(!lce_admit(old, sizeof old, signature, sizeof signature, 2, 2, 1,
                     lce_signature_bridge, &ring, &version) &&
          version == 99);
    CHECK(!lce_admit(old, sizeof old, signature, 63, 2, 0, 1,
                     lce_signature_bridge, &ring, &version) &&
          version == 99);
    printf("DU_SIGNATURE_PROVIDER:%s\n", OpenSSL_version(OPENSSL_VERSION));
    puts(
        "DU_SIGNATURE_PASS:actual-old-signature-valid-new-floor-rejects synthetic-only");
    return 0;
}
