#include "publication.h"

struct ae_publication *ae_publication_new(const struct ae_config *c)
{
    (void)c;
    return 0;
}

enum ae_code ae_publication_replace(struct ae_publication *p,
                                    const struct ae_config *c, unsigned f)
{
    (void)p;
    (void)c;
    (void)f;
    return AE_UNIMPLEMENTED;
}

enum ae_code ae_publication_acquire(struct ae_publication *p,
                                    struct ae_read_lease *l)
{
    (void)p;
    (void)l;
    return AE_UNIMPLEMENTED;
}

enum ae_code ae_publication_read(const struct ae_read_lease *l,
                                 struct ae_config *c)
{
    (void)l;
    (void)c;
    return AE_UNIMPLEMENTED;
}

enum ae_code ae_publication_release(struct ae_read_lease *l)
{
    (void)l;
    return AE_UNIMPLEMENTED;
}

enum ae_code ae_publication_counts(struct ae_publication *p, unsigned *m,
                                   unsigned *f, unsigned *l)
{
    (void)p;
    (void)m;
    (void)f;
    (void)l;
    return AE_UNIMPLEMENTED;
}

enum ae_code ae_publication_close(struct ae_publication *p)
{
    (void)p;
    return AE_UNIMPLEMENTED;
}

enum ae_code ae_publication_destroy(struct ae_publication **p)
{
    (void)p;
    return AE_UNIMPLEMENTED;
}
