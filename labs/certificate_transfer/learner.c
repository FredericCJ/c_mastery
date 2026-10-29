/* Deliberately incomplete learner artifact. Keep author solutions separate. */
#include "transfer.h"

void ct_init(struct ct_stream *s)
{
    if (s)
    {
        *s = (struct ct_stream){0};
    }
}

enum ct_status ct_feed(struct ct_stream *s, const unsigned char *p, size_t n,
                       unsigned f, const struct ag_certificate *c,
                       struct cd_csr *out)
{
    (void)s;
    (void)p;
    (void)n;
    (void)f;
    (void)c;
    (void)out;
    return CT_INVALID;
}

int ct_tree(const struct at_certificate *c, unsigned p, struct ct_workspace *w,
            unsigned n, int *out)
{
    (void)c;
    (void)p;
    (void)w;
    (void)n;
    (void)out;
    return ALGO_BAD;
}
