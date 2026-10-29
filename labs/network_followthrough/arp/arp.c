#include "arp.h"

int na_init(struct na_arp *s, struct na_config c, uint32_t epoch)
{
    (void)s;
    (void)c;
    (void)epoch;
    return NA_UNIMPLEMENTED;
}

int na_queue(struct na_arp *s, const void *p, size_t n, uint64_t now)
{
    (void)s;
    (void)p;
    (void)n;
    (void)now;
    return NA_UNIMPLEMENTED;
}

int na_receive(struct na_arp *s, const void *p, size_t n, uint64_t now)
{
    (void)s;
    (void)p;
    (void)n;
    (void)now;
    return NA_UNIMPLEMENTED;
}

int na_prepare(struct na_arp *s, uint64_t now, struct net_packet *p,
               struct na_ticket *t)
{
    (void)s;
    (void)now;
    (void)p;
    (void)t;
    return NA_UNIMPLEMENTED;
}

int na_submit(struct na_arp *s, struct na_ticket t, unsigned accepted)
{
    (void)s;
    (void)t;
    (void)accepted;
    return NA_UNIMPLEMENTED;
}
