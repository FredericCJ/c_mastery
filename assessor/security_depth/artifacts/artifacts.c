#include "artifacts.h"
#include <openssl/evp.h>
#include <string.h>
#ifndef SA_PROVIDER_FAILURE
#define SA_PROVIDER_FAILURE 0
#endif
static void be32(unsigned char *p, uint32_t value)
{
    for (unsigned i = 0; i < 4; ++i)
        p[i] = (unsigned char)(value >> (24 - 8 * i));
}

int sa_digest(const unsigned char *p, size_t n, unsigned char out[32])
{
    unsigned char result[32];
    size_t length = 0;
    if ((!p && n) || n > 32 || !out)
        return -1;
    if (SA_PROVIDER_FAILURE == 4)
        return -1;
    if (EVP_Q_digest(NULL, "SHA256", NULL, p, n, result, &length) != 1 ||
        length != 32)
        return -1;
    memcpy(out, result, 32);
    return 0;
}

void sa_manifest(const struct sa_artifact *a, unsigned char out[44])
{
    memcpy(out, "CM35", 4);
    be32(out + 4, a->version);
    be32(out + 8, a->epoch);
    memcpy(out + 12, a->digest, 32);
}

void sa_transition(const struct sa_trust *old, const unsigned char key[32],
                   uint32_t epoch, uint32_t minimum, unsigned char out[49])
{
    memcpy(out, "ROT35", 5);
    be32(out + 5, old->epoch);
    be32(out + 9, epoch);
    be32(out + 13, minimum);
    memcpy(out + 17, key, 32);
}

static int signature(const unsigned char pub[32], const unsigned char sig[64],
                     const unsigned char *message, size_t length)
{
    int result = -1;
    EVP_PKEY *key =
        EVP_PKEY_new_raw_public_key_ex(NULL, "ED25519", NULL, pub, 32);
    EVP_MD_CTX *ctx = NULL;
    if (!key)
        goto done;
    if (SA_PROVIDER_FAILURE == 1)
        goto done;
    ctx = EVP_MD_CTX_new();
    if (!ctx)
        goto done;
    if (SA_PROVIDER_FAILURE == 2)
        goto done;
    if (EVP_DigestVerifyInit(ctx, NULL, NULL, NULL, key) != 1)
        goto done;
    if (SA_PROVIDER_FAILURE == 3)
        goto done;
    result = EVP_DigestVerify(ctx, sig, 64, message, length);
done:
    EVP_MD_CTX_free(ctx);
    EVP_PKEY_free(key);
    return result;
}

int sa_verify(const struct sa_trust *trust, const struct sa_artifact *a,
              const unsigned char *bytes, size_t n, struct sa_report *out)
{
    unsigned char digest[32], message[44];
    struct sa_report r;
    int verified;
    if (!trust || !a || !out || a->committed > 1 || !trust->epoch ||
        !a->epoch || (!bytes && n) || n > 32)
        return -1;
    if (sa_digest(bytes, n, digest))
        return -1;
    sa_manifest(a, message);
    verified = signature(a->public_key, a->signature, message, sizeof message);
    if (verified < 0)
        return -1;
    r.digest_matches = memcmp(digest, a->digest, 32) == 0;
    r.signature_valid = verified == 1;
    r.trusted_signer = memcmp(trust->public_key, a->public_key, 32) == 0;
    r.compatible = a->version >= trust->minimum;
    r.generation_current = a->epoch == trust->epoch;
    r.committed = a->committed;
#ifdef SA_MUTANT_TRUST
    r.ready = r.digest_matches && r.signature_valid && r.compatible &&
              r.generation_current && r.committed;
#else
    r.ready = r.digest_matches && r.signature_valid && r.trusted_signer &&
              r.compatible && r.generation_current && r.committed;
#endif
    *out = r;
    return 0;
}

int sa_rotate(struct sa_trust *trust, const unsigned char key[32],
              uint32_t epoch, uint32_t minimum, const unsigned char sig[64])
{
    unsigned char message[49];
    struct sa_trust next;
    if (!trust || !key || !sig || !trust->epoch || trust->epoch == UINT32_MAX ||
        epoch != trust->epoch + 1 || minimum < trust->minimum ||
        !memcmp(key, trust->public_key, 32))
        return -1;
    sa_transition(trust, key, epoch, minimum, message);
#ifndef SA_MUTANT_ROTATE
    if (signature(trust->public_key, sig, message, sizeof message) != 1)
        return -1;
#endif
    memcpy(next.public_key, key, 32);
    next.epoch = epoch;
    next.minimum = minimum;
    *trust = next;
    return 0;
}
