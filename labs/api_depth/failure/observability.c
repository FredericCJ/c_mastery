#include "observability.h"

enum ae_code ae_record_encode(const struct ae_record *r, unsigned char *p,
                              size_t n)
{
    (void)r;
    (void)p;
    (void)n;
    return AE_UNIMPLEMENTED;
}

enum ae_code ae_record_decode(const unsigned char *p, size_t n,
                              struct ae_record *r)
{
    (void)p;
    (void)n;
    (void)r;
    return AE_UNIMPLEMENTED;
}

enum ae_code ae_tick_order(unsigned a, unsigned b, unsigned w, int *o)
{
    (void)a;
    (void)b;
    (void)w;
    (void)o;
    return AE_UNIMPLEMENTED;
}

enum ae_code ae_rate_refill(struct ae_rate *r, unsigned n)
{
    (void)r;
    (void)n;
    return AE_UNIMPLEMENTED;
}

enum ae_code ae_log(struct ae_log *l, const struct ae_record *r,
                    enum ae_log_policy p)
{
    (void)l;
    (void)r;
    (void)p;
    return AE_UNIMPLEMENTED;
}

enum ae_code ae_log_drain(struct ae_log *l, ae_sink_fn s, void *c)
{
    (void)l;
    (void)s;
    (void)c;
    return AE_UNIMPLEMENTED;
}

enum ae_code ae_report(struct ae_reporter *r, const struct ae_record *x,
                       ae_sink_fn s, void *c)
{
    (void)r;
    (void)x;
    (void)s;
    (void)c;
    return AE_UNIMPLEMENTED;
}
