#include "signature_adapter.h"
#include "lifecycle_extension.h"
#include "image_transfer.h"
#include <openssl/evp.h>
#include <openssl/crypto.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define C(x)                                                              \
    do                                                                    \
    {                                                                     \
        if (!(x))                                                         \
        {                                                                 \
            fprintf(stderr, "LCE_SIGNATURE_CHECK:%d:%s\n", __LINE__, #x); \
            exit(1);                                                      \
        }                                                                 \
    } while (0)

/* Deliberately public synthetic teaching seeds, not credentials. */
static EVP_PKEY *teaching_key(unsigned id, struct lce_keyring *r)
{
    unsigned char seed[32];
    for (unsigned i = 0; i < 32; i++)
        seed[i] = (unsigned char)(id * 31 + i);
    EVP_PKEY *k =
        EVP_PKEY_new_raw_private_key(EVP_PKEY_ED25519, NULL, seed, 32);
    C(k);
    size_t z = 32;
    C(EVP_PKEY_get_raw_public_key(k, r->public_keys[id - 1], &z) == 1 &&
      z == 32);
    return k;
}

static void sign_image(EVP_PKEY *k, const unsigned char *p, size_t n,
                       unsigned char sig[64])
{
    EVP_MD_CTX *c = EVP_MD_CTX_new();
    size_t z = 64;
    C(c && EVP_DigestSignInit(c, NULL, NULL, NULL, k) == 1 &&
      EVP_DigestSign(c, sig, &z, p, n) == 1 && z == 64);
    EVP_MD_CTX_free(c);
}

static int false_error(void *c, unsigned id, const unsigned char *p, size_t n,
                       const unsigned char *s, size_t z)
{
    (void)c;
    (void)id;
    (void)p;
    (void)n;
    (void)s;
    (void)z;
    return -1;
}

static int policy_commit(void *context, const struct fi_rotation *candidate)
{
    *(struct fi_rotation *)context = *candidate;
    return 1;
}

static void compose(EVP_PKEY *key, struct lce_keyring *r)
{
    unsigned char image[32] = {8, 0, 0, 0, 2, 2}, sig[64], record[24];
    for (unsigned i = 6; i < 32; i++)
        image[i] = (unsigned char)i;
    sign_image(key, image, 32, sig);
    uint32_t version = 0;
    C(lce_admit(image, 32, sig, 64, 6, 2, 8, lce_signature_bridge, r,
                &version) &&
      version == 8);
    struct fi_image flash;
    C(fi_init(&flash, 8));
    for (unsigned i = 0; i < 4; i++)
        C(fi_program(&flash, image, i, 64, 1));
    C(fi_commit(&flash, image, 2, record));
    C(fi_record_matches(flash.bytes, record) &&
      lce_admit(flash.bytes, 32, sig, 64, 6, 2, 8, lce_signature_bridge, r,
                &version));
    struct lce_signature_result verdict = lce_ed25519(r, 2, image, 32, sig, 64);
    struct fi_rotation policy = {1, 3, 3, 7, 1, 0}, durable = policy;
    C(fi_rotate(&policy, image[5], 2, version, verdict.verified_key,
                verdict.accepted, policy_commit, &durable) &&
      policy.epoch == 2 && policy.floor == 8 && policy.revoked == 2);
    flash.bytes[17] ^= 1;
    C(!fi_record_matches(flash.bytes, record));
    C(!lce_admit(flash.bytes, 32, sig, 64, 6, 2, 8, lce_signature_bridge, r,
                 &version));
    puts("LCE_IMAGE_SIGNATURE_ROTATION_COMPOSITION_PASS");
}

int main(void)
{
    struct lce_keyring r = {0};
    r.present = 6;
    EVP_PKEY *k1 = teaching_key(1, &r), *k2 = teaching_key(2, &r);
    unsigned char image[] = {7, 0, 0, 0, 1, 19, 27}, sig[64],
                  copy[sizeof image];
    sign_image(k1, image, sizeof image, sig);
    struct lce_signature_result v =
        lce_ed25519(&r, 1, image, sizeof image, sig, 64);
    C(v.accepted == 1 && v.verified_key == 1);
    uint32_t out = 91;
    C(lce_admit(image, sizeof image, sig, 64, 6, 0, 7, lce_signature_bridge, &r,
                &out) &&
      out == 7);
    for (size_t i = 0; i < sizeof image; i++)
    {
        memcpy(copy, image, sizeof image);
        copy[i] ^= 1;
        out = 91;
        C(!lce_admit(copy, sizeof copy, sig, 64, 6, 0, 0, lce_signature_bridge,
                     &r, &out) &&
          out == 91);
    }
    sig[3] ^= 1;
    C(!lce_ed25519(&r, 1, image, sizeof image, sig, 64).accepted);
    sig[3] ^= 1;
    C(!lce_ed25519(&r, 2, image, sizeof image, sig, 64).accepted);
    C(!lce_ed25519(&r, 1, image, sizeof image, sig, 63).accepted);
    out = 91;
    C(!lce_admit(image, sizeof image, sig, 64, 6, 2, 7, lce_signature_bridge,
                 &r, &out) &&
      out == 91);
    C(!lce_admit(image, sizeof image, sig, 64, 6, 0, 8, lce_signature_bridge,
                 &r, &out) &&
      out == 91);
    C(!lce_admit(image, sizeof image, sig, 64, 6, 0, 0, false_error, NULL,
                 &out) &&
      out == 91);
    image[0] = 8;
    image[4] = 2;
    sign_image(k2, image, sizeof image, sig);
    C(lce_admit(image, sizeof image, sig, 64, 6, 2, 8, lce_signature_bridge, &r,
                &out) &&
      out == 8);
    v = lce_ed25519(&r, 2, image, sizeof image, sig, 64);
    C(v.accepted == 1 && v.verified_key == 2);
    compose(k2, &r);
    EVP_PKEY_free(k1);
    EVP_PKEY_free(k2);
    printf("LCE_SIGNATURE_PASS %s\n", OpenSSL_version(OPENSSL_VERSION));
    return 0;
}
