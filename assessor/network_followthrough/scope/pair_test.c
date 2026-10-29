#include "pair.h"
#include <stdio.h>
#include <string.h>
#define CHECK(x)                 \
    do                           \
    {                            \
        if (!(x))                \
        {                        \
            puts("NW_PAIR " #x); \
            return 1;            \
        }                        \
    } while (0)

static void w16(unsigned char *p, unsigned v)
{
    p[0] = (unsigned char)(v >> 8);
    p[1] = (unsigned char)v;
}

static void w32(unsigned char *p, uint32_t v)
{
    w16(p, v >> 16);
    w16(p + 2, v);
}

static unsigned sum(const unsigned char *p, size_t n, unsigned value)
{
    for (size_t i = 0; i < n; ++i)
    {
        value += (unsigned)p[i] << (i % 2 ? 0 : 8);
    }
    while (value > 65535)
    {
        value = (value & 65535) + (value >> 16);
    }
    return 65535 - value;
}

static void arp(struct net_packet *p)
{
    static const unsigned char packet[42] = {
        255, 255, 255, 255, 255, 255, 2, 0, 0, 0, 0,   1, 8, 6,
        0,   1,   8,   0,   6,   4,   0, 1, 2, 0, 0,   0, 0, 1,
        192, 0,   2,   1,   0,   0,   0, 0, 0, 0, 192, 0, 2, 2};
    memset(p, 0, sizeof *p);
    memcpy(p->bytes, packet, sizeof packet);
    p->length = 60;
}

static void packet(struct net_packet *p, uint16_t port, uint32_t seq,
                   uint32_t ack, unsigned flags, const void *data, size_t n)
{
    static const unsigned char mac[14] = {2, 0, 0, 0, 0, 2, 2,
                                          0, 0, 0, 0, 1, 8, 0};
    static const unsigned char ipaddr[8] = {192, 0, 2, 1, 192, 0, 2, 2};
    memset(p, 0, sizeof *p);
    memcpy(p->bytes, mac, 14);
    unsigned char *ip = p->bytes + 14, *tcp = p->bytes + 34;
    ip[0] = 0x45;
    w16(ip + 2, (unsigned)n + 40);
    ip[8] = 64;
    ip[9] = 6;
    memcpy(ip + 12, ipaddr, 8);
    w16(ip + 10, sum(ip, 20, 0));
    w16(tcp, port);
    w16(tcp + 2, 8080);
    w32(tcp + 4, seq);
    w32(tcp + 8, ack);
    tcp[12] = 0x50;
    tcp[13] = (unsigned char)flags;
    w16(tcp + 14, 512);
    if (n)
    {
        memcpy(tcp + 20, data, n);
    }
    w16(tcp + 16, sum(tcp, n + 20,
                      0xc000 + 0x201 + 0xc000 + 0x202 + 6 + (unsigned)n + 20));
    p->length = n + 54;
    if (p->length < 60)
    {
        p->length = 60;
    }
}
static struct nw_pair pair;
#ifdef NW_TARGET
int nw_pair_test(void)
{
#else
int main(void)
{
#endif
    struct net_packet p, out;
    CHECK(nw_pair_init(&pair, 1, 7, 40000, 40001) == NW_OK);
    packet(&p, 40001, 200, 0, 2, 0, 0);
    out.length = 999;
    CHECK(nw_pair_receive(&pair, p.bytes, p.length, 0, &out) == NW_BUSY &&
          out.length == 999);
    CHECK(nw_pair_init(&pair, 2, 8, 40000, 40001) == NW_OK);
    arp(&p);
    CHECK(nw_pair_receive(&pair, p.bytes, p.length, 0, &out) == NW_OK &&
          out.length == 60);
    for (unsigned i = 0; i < 2; ++i)
    {
        packet(&p, (uint16_t)(40000 + i), 100 + 100 * i, 0, 2, 0, 0);
        CHECK(nw_pair_receive(&pair, p.bytes, p.length, 1 + 20 * i, &out) ==
              NW_OK);
        CHECK(pair.flows[i].state == NET_SYN_RECEIVED &&
              pair.flows[i].peer_port == 40000 + i);
        packet(&p, (uint16_t)(40000 + i), 101 + 100 * i, pair.flows[i].snd_nxt,
               16, 0, 0);
        CHECK(nw_pair_receive(&pair, p.bytes, p.length, 20 + 20 * i, &out) ==
                  NW_OK &&
              pair.flows[i].state == NET_ESTABLISHED);
    }
    const char get[] = "GET /health HTTP/1.1\r\nHost: 192.0.2.2:8080\r\n\r\n";
    packet(&p, 40000, 101, pair.flows[0].snd_nxt, 24, get, sizeof get - 1);
    CHECK(nw_pair_receive(&pair, p.bytes, p.length, 50, &out) == NW_OK &&
          pair.flows[0].response_length > 0);
    struct net_packet retained = pair.flows[0].retained;
    CHECK(pair.flows[1].http.state == HTTP_LINE &&
          pair.flows[1].response_length == 0);
    const char post[] = "POST /echo HTTP/1.1\r\nHost: "
                        "192.0.2.2:8080\r\nContent-Length: 3\r\n\r\nXYZ";
    packet(&p, 40001, 201, pair.flows[1].snd_nxt, 24, post, sizeof post - 1);
    CHECK(nw_pair_receive(&pair, p.bytes, p.length, 51, &out) == NW_OK &&
          pair.flows[1].http.request.body_length == 3);
    CHECK(!memcmp(pair.flows[1].http.request.body, "XYZ", 3));
    CHECK(
        pair.flows[0].retained.length == retained.length &&
        !memcmp(pair.flows[0].retained.bytes, retained.bytes, retained.length));
#ifdef NW_TARGET
    puts(
        "NW_PAIR_PASS actual_handshakes=2 distinct_requests=2 one_slot_refusal "
        "target_sizes_in_object_map");
#else
    printf("NW_PAIR_PASS actual_handshakes=2 distinct_requests=2 "
           "one_slot_refusal sizeof_stack=%zu sizeof_pair=%zu\n",
           sizeof(struct net_stack), sizeof(struct nw_pair));
#endif
    return 0;
}
