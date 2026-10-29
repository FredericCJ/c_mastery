#include "service.h"

/* Implement each operation from CONTRACTS.md using the actual P3 producer. */
enum cs_status cs_init(struct cs_service *s, size_t n, unsigned g)
{
    (void)s;
    (void)n;
    (void)g;
    return CS_INVALID;
}

enum cs_status cs_offer(struct cs_service *s, const struct cp_message *m, int a,
                        int r, int o)
{
    (void)s;
    (void)m;
    (void)a;
    (void)r;
    (void)o;
    return CS_INVALID;
}

enum cs_status cs_feed(struct cs_service *s, unsigned i, const unsigned char *b,
                       size_t n, size_t *u, int a, int r)
{
    (void)s;
    (void)i;
    (void)b;
    (void)n;
    (void)u;
    (void)a;
    (void)r;
    return CS_INVALID;
}

enum cs_status cs_begin(struct cs_service *s)
{
    (void)s;
    return CS_INVALID;
}

enum cs_status cs_step(struct cs_service *s, cs_sink_fn f, void *c)
{
    (void)s;
    (void)f;
    (void)c;
    return CS_INVALID;
}

enum cs_status cs_recover(struct cs_service *s)
{
    (void)s;
    return CS_INVALID;
}

enum cs_status cs_lease(struct cs_service *s, const unsigned char **b,
                        size_t *n, unsigned *g)
{
    (void)s;
    (void)b;
    (void)n;
    (void)g;
    return CS_INVALID;
}

enum cs_status cs_complete(struct cs_service *s, unsigned g, int q, size_t n,
                           cs_done_fn f, void *c)
{
    (void)s;
    (void)g;
    (void)q;
    (void)n;
    (void)f;
    (void)c;
    return CS_INVALID;
}

enum cs_status cs_close(struct cs_service *s, int c)
{
    (void)s;
    (void)c;
    return CS_INVALID;
}

int cs_conserved(const struct cs_service *s)
{
    (void)s;
    return 0;
}
