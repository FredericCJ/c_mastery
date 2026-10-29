#include "v4_com.h"
#include <stdio.h>
#include <string.h>
#define CHECK(x)                                       \
    do                                                 \
    {                                                  \
        if (!(x))                                      \
        {                                              \
            fprintf(stderr, "%d: %s\n", __LINE__, #x); \
            return 1;                                  \
        }                                              \
    } while (0)

struct edu_can
{
    uint32_t r[80];
    unsigned writes, bad, stuck;
};

static uint32_t rd(void *p, unsigned r)
{
    struct edu_can *c = p;
    if (c->stuck && r == 0)
        return 0;
    return c->r[r];
}

static void wr(void *p, unsigned r, uint32_t v)
{
    struct edu_can *c = p;
    if ((r == 1 || r == 2) && c->r[0] != 3)
        c->bad = 1;
    c->r[r] = v;
    c->writes++;
    if (r == 4)
        c->r[5] = 1;
}

static int drop(void *p, const struct net_packet *n)
{
    unsigned *count = p;
    (*count)++;
    return n->length == 3 ? 1 : -1;
}

int main(int argc, char **argv)
{
    if (argc != 2)
        return 2;
    if (!strcmp(argv[1], "reassembly"))
    {
        struct v4_reassembly r = {0};
        struct edu_fragment f = {.type = 1,
                                 .source_port = 7,
                                 .destination_port = 8,
                                 .id = 1,
                                 .count = 2,
                                 .length = 24},
                            ack;
        CHECK(v4_reassembly(&r, &f, &ack, 0, 0) < 0 && !r.rx.active);
        CHECK(v4_reassembly(&r, &f, &ack, 0, 1) == 0 && r.rx.active);
        CHECK(v4_reassembly(&r, &f, &ack, 99, 1) == 0 && r.due == 100);
        f.index = 1;
        f.length = 1;
        f.data[0] = 9;
        CHECK(v4_reassembly(&r, &f, &ack, 101, 1) < 0 && !r.rx.ready);
        f.index = 0;
        f.length = 24;
        CHECK(v4_reassembly(&r, &f, &ack, 102, 1) == 0);
        f.index = 1;
        f.length = 1;
        CHECK(v4_reassembly(&r, &f, &ack, 103, 1) == 1 &&
              r.rx.publications == 1 && r.rx.length == 25);
        CHECK(v4_reassembly(&r, &f, &ack, 104, 1) == 0 &&
              r.rx.publications == 1);
    }
    else if (!strcmp(argv[1], "arp"))
    {
        struct v4_arp a = {0};
        uint8_t p[] = {1, 2, 3};
        CHECK(v4_arp_queue(&a, 42, p, 3, 0) == 0);
        p[0] = 9;
        CHECK(a.bytes[0] == 1 && v4_arp_queue(&a, 2, p, 3, 0) < 0);
        CHECK(v4_arp_poll(&a, 0) == 1 && v4_arp_poll(&a, 99) == 0);
        CHECK(v4_arp_resolve(&a, 41) < 0);
        CHECK(v4_arp_resolve(&a, 42) == 0 && v4_arp_poll(&a, 100) == 2 &&
              !a.pending);
        CHECK(v4_arp_queue(&a, 42, p, 3, 0) == 0);
        for (unsigned i = 0; i < 3; i++)
            CHECK(v4_arp_poll(&a, 100 * i) == 1);
        CHECK(v4_arp_poll(&a, 300) < 0 && !a.pending);
    }
    else if (!strcmp(argv[1], "tcp_fault"))
    {
        struct net_stack s;
        net_init(&s, 7);
        s.state = NET_ESTABLISHED;
        s.flight = 1;
        s.retained.length = 3;
        s.retained.bytes[0] = 42;
        s.rto = 100;
        s.due = 100;
        unsigned drops = 0, calls = 0;
        CHECK(v4_tcp_delivery(&s, 99, drop, &calls, &drops) == 0 && calls == 0);
        CHECK(v4_tcp_delivery(&s, 100, drop, &calls, &drops) == 0 &&
              calls == 1 && drops == 1 && s.retained.bytes[0] == 42 &&
              s.retransmitted);
        CHECK(v4_tcp_delivery(&s, 300, drop, &calls, &drops) == 0);
        CHECK(v4_tcp_delivery(&s, 700, drop, &calls, &drops) == 0);
        CHECK(v4_tcp_delivery(&s, 1500, drop, &calls, &drops) < 0 &&
              !s.flight && s.state == NET_LISTEN && calls == 3);
    }
    else if (!strcmp(argv[1], "canfd"))
    {
        struct edu_can c = {0};
        struct v4_can_io io = {&c, rd, wr};
        CHECK(v4_can_init(io, 0x123, 0x45, 3) == 0 && !c.bad &&
              c.r[1] == 0x123 && c.r[2] == 0x45 && c.r[0] == 0);
        uint8_t in[64], out[64];
        for (unsigned i = 0; i < 64; i++)
            in[i] = (uint8_t)i;
        CHECK(v4_can_loopback(io, in, 64, out, 64, 3) == 0 &&
              !memcmp(in, out, 64));
        CHECK(v4_can_loopback(io, in, 9, out, 64, 3) < 0);
        c = (struct edu_can){.stuck = 1};
        CHECK(v4_can_init(io, 1, 1, 2) < 0 && c.r[1] == 0);
    }
    else
        return 2;
    return 0;
}
