#include "ip.h"

int ni_expire(struct ni_reassembly *s, uint64_t now, struct ni_packet out[2],
              unsigned *count)
{
    (void)s;
    (void)now;
    (void)out;
    (void)count;
    return NI_STUB;
}

int ni_parse(const struct ni_config *a, const unsigned char *b, size_t c,
             unsigned d, struct ni_view *e)
{
    (void)a;
    (void)b;
    (void)c;
    (void)d;
    (void)e;
    return NI_STUB;
}

int ni_echo(const struct ni_config *a, const unsigned char *b, size_t c,
            struct ni_packet *d)
{
    (void)a;
    (void)b;
    (void)c;
    (void)d;
    return NI_STUB;
}

int ni_init(struct ni_reassembly *a, struct ni_config b, unsigned c, unsigned d,
            uint64_t e, uint64_t f)
{
    (void)a;
    (void)b;
    (void)c;
    (void)d;
    (void)e;
    (void)f;
    return NI_STUB;
}

int ni_tick(struct ni_reassembly *a, uint64_t b)
{
    (void)a;
    (void)b;
    return NI_STUB;
}

int ni_receive(struct ni_reassembly *a, const unsigned char *b, size_t c,
               uint64_t d, unsigned *e)
{
    (void)a;
    (void)b;
    (void)c;
    (void)d;
    (void)e;
    return NI_STUB;
}

int ni_take(struct ni_reassembly *a, unsigned b, struct ni_packet *c)
{
    (void)a;
    (void)b;
    (void)c;
    return NI_STUB;
}
