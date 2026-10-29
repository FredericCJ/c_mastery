#include "wire.h"

int ew_feed(struct ew_stream *a, const unsigned char *b, size_t c, size_t *d)
{
    (void)a;
    (void)b;
    (void)c;
    (void)d;
    return EW_STUB;
}

int ew_take(struct ew_stream *a, unsigned char *b, size_t c, size_t *d)
{
    (void)a;
    (void)b;
    (void)c;
    (void)d;
    return EW_STUB;
}

int ew_end(struct ew_stream *a)
{
    (void)a;
    return EW_STUB;
}

int ew_vlan_build(const unsigned char *a, const unsigned char *b, unsigned c,
                  unsigned d, unsigned e, const unsigned char *f, size_t g,
                  unsigned char *h, size_t i, size_t *j)
{
    (void)a;
    (void)b;
    (void)c;
    (void)d;
    (void)e;
    (void)f;
    (void)g;
    (void)h;
    (void)i;
    (void)j;
    return EW_STUB;
}

int ew_vlan_parse(const unsigned char *a, size_t b, struct ew_vlan *c)
{
    (void)a;
    (void)b;
    (void)c;
    return EW_STUB;
}

int ew_vlan_pop(const unsigned char *a, size_t b, unsigned char *c, size_t d,
                size_t *e)
{
    (void)a;
    (void)b;
    (void)c;
    (void)d;
    (void)e;
    return EW_STUB;
}

int ew_bridge(struct ed_bridge *a, const unsigned char *b,
              const unsigned char *c, unsigned d, uint32_t e, unsigned f,
              unsigned *g)
{
    (void)a;
    (void)b;
    (void)c;
    (void)d;
    (void)e;
    (void)f;
    (void)g;
    return EW_STUB;
}
