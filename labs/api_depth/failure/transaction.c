#include "transaction.h"

struct ae_update ae_replace(struct ae_config *c, const unsigned *p, unsigned n,
                            unsigned z, enum ae_guarantee g, unsigned f,
                            unsigned e, ae_observe_fn o, void *x)
{
    (void)c;
    (void)p;
    (void)n;
    (void)z;
    (void)g;
    (void)f;
    (void)e;
    (void)o;
    (void)x;
    return (struct ae_update){AE_UNIMPLEMENTED, 0, 0};
}

enum ae_code ae_journal(struct ae_journal *j, unsigned i, unsigned d,
                        unsigned c)
{
    (void)j;
    (void)i;
    (void)d;
    (void)c;
    return AE_UNIMPLEMENTED;
}
