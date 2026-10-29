#include "diagnostic.h"

/* Learner scaffold: implement the contracts without an assessor fallback. */
int dg_encode(const struct dg_message *m, uint8_t *p, size_t n, size_t *w)
{
    (void)m;
    (void)p;
    (void)n;
    (void)w;
    return DG_UNIMPLEMENTED;
}

int dg_decode(const uint8_t *p, size_t n, struct dg_message *m)
{
    (void)p;
    (void)n;
    (void)m;
    return DG_UNIMPLEMENTED;
}

int dg_accept(struct dg_session *s, const uint8_t *p, size_t n,
              struct dg_message *m)
{
    (void)s;
    (void)p;
    (void)n;
    (void)m;
    return DG_UNIMPLEMENTED;
}

int dg_stream_feed(struct dg_stream *s, const uint8_t *p, size_t n, size_t *u)
{
    (void)s;
    (void)p;
    (void)n;
    (void)u;
    return DG_UNIMPLEMENTED;
}

int dg_stream_take(struct dg_stream *s, struct dg_session *c,
                   struct dg_message *m)
{
    (void)s;
    (void)c;
    (void)m;
    return DG_UNIMPLEMENTED;
}

int dg_stream_end(const struct dg_stream *s)
{
    (void)s;
    return DG_UNIMPLEMENTED;
}
