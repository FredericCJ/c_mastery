#include "modern.h"

er_progress cs_result_step(const unsigned char *b, size_t n, size_t i,
                           struct cs_result_sink *s)
{
    (void)b;
    (void)n;
    (void)s;
    return (er_progress){i, ER_NOT_IMPLEMENTED};
}

void cs_slabs_init(struct cs_slabs *s)
{
    (void)s;
}

enum cs_status cs_slabs_share(struct cs_slabs *s, const struct cp_message *m,
                              struct cs_slab_token t[2])
{
    (void)s;
    (void)m;
    (void)t;
    return CS_INVALID;
}

enum cs_status cs_slabs_view(struct cs_slabs *s, struct cs_slab_token t,
                             const unsigned char **b, size_t *n)
{
    (void)s;
    (void)t;
    (void)b;
    (void)n;
    return CS_INVALID;
}

enum cs_status cs_slabs_drop(struct cs_slabs *s, struct cs_slab_token t)
{
    (void)s;
    (void)t;
    return CS_INVALID;
}

enum cs_status cs_receipt(struct cs_receipts *r, unsigned e,
                          const struct cp_message *m)
{
    (void)r;
    (void)e;
    (void)m;
    return CS_INVALID;
}

int cs_resources_open(struct cs_resources *r, int e, int i, int o, unsigned f,
                      int m)
{
    (void)r;
    (void)e;
    (void)i;
    (void)o;
    (void)f;
    (void)m;
    return -1;
}

int cs_resources_close(struct cs_resources *r)
{
    (void)r;
    return -1;
}

int cs_resources_borrow(struct cs_resources *r)
{
    (void)r;
    return -1;
}

int cs_resources_ack(struct cs_resources *r)
{
    (void)r;
    return -1;
}
