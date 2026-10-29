#include "signature_adapter.h"

struct lce_signature_result lce_ed25519(const struct lce_keyring *k,
                                        unsigned id, const unsigned char *p,
                                        size_t n, const unsigned char *s,
                                        size_t z)
{
    (void)k;
    (void)id;
    (void)p;
    (void)n;
    (void)s;
    (void)z;
    struct lce_signature_result r = {0, 0};
    return r;
}

int lce_signature_bridge(void *c, unsigned id, const unsigned char *p, size_t n,
                         const unsigned char *s, size_t z)
{
    (void)c;
    (void)id;
    (void)p;
    (void)n;
    (void)s;
    (void)z;
    return 0;
}
