#include "com_net.h"
#include <string.h>

/* Buildable exercises: every operation still needs implementation. */
uint16_t net_checksum(const void *v, size_t n, uint32_t sum)
{
    return 0;
}

void net_init(struct net_stack *s, uint32_t isn)
{
    (void)isn;
    memset(s, 0, sizeof *s);
}

void net_link_down(struct net_stack *s)
{
    return;
}

void net_rtt(struct net_stack *s, uint32_t r)
{
    return;
}

int net_receive(struct net_stack *s, const void *v, size_t n, uint64_t now,
                struct net_packet *o)
{
    return -1;
}

int net_tick(struct net_stack *s, uint64_t now, struct net_packet *o)
{
    return -1;
}
