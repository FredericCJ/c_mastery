#include "com_net.h"
#include <stdio.h>
#include <string.h>
#define CHECK(x)                                                    \
    do                                                              \
    {                                                               \
        if (!(x))                                                   \
        {                                                           \
            fprintf(stderr, "%s:%d: %s\n", __FILE__, __LINE__, #x); \
            return 1;                                               \
        }                                                           \
    } while (0)

static unsigned r16(const uint8_t *p)
{
    return (unsigned)p[0] * 256 + p[1];
}

static uint32_t r32(const uint8_t *p)
{
    return ((uint32_t)r16(p) << 16) | r16(p + 2);
}

static void w16(uint8_t *p, unsigned v)
{
    p[0] = (uint8_t)(v >> 8);
    p[1] = (uint8_t)v;
}

static void w32(uint8_t *p, uint32_t v)
{
    w16(p, v >> 16);
    w16(p + 2, v);
}

/* Independent byte-at-a-time checksum oracle, deliberately different folding.
 */
static uint16_t checksum(const uint8_t *p, size_t n, uint32_t sum)
{
    for (size_t i = 0; i < n; i++)
    {
        sum += (uint32_t)p[i] << (i % 2 ? 0 : 8);
    }
    while (sum > 65535)
    {
        sum = (sum & 65535) + (sum >> 16);
    }
    return (uint16_t)(65535 - sum);
}

static const uint8_t local_mac[6] = {2, 0, 0, 0, 0, 2},
                     peer_mac[6] = {2, 0, 0, 0, 0, 1};
static const uint8_t local_ip[4] = {192, 0, 2, 2}, peer_ip[4] = {192, 0, 2, 1};

static void arp(struct net_packet *p)
{
    memset(p, 0, sizeof *p);
    memset(p->bytes, 255, 6);
    memcpy(p->bytes + 6, peer_mac, 6);
    w16(p->bytes + 12, 0x806);
    uint8_t *a = p->bytes + 14;
    w16(a, 1);
    w16(a + 2, 0x800);
    a[4] = 6;
    a[5] = 4;
    w16(a + 6, 1);
    memcpy(a + 8, peer_mac, 6);
    memcpy(a + 14, peer_ip, 4);
    memcpy(a + 24, local_ip, 4);
    p->length = 60;
}

static void tcp(struct net_packet *p, uint32_t seq, uint32_t ack,
                unsigned flags, unsigned window, const void *data, size_t n)
{
    memset(p, 0, sizeof *p);
    memcpy(p->bytes, local_mac, 6);
    memcpy(p->bytes + 6, peer_mac, 6);
    w16(p->bytes + 12, 0x800);
    uint8_t *ip = p->bytes + 14;
    ip[0] = 0x45;
    w16(ip + 2, (unsigned)n + 40);
    w16(ip + 6, 0x4000);
    ip[8] = 64;
    ip[9] = 6;
    memcpy(ip + 12, peer_ip, 4);
    memcpy(ip + 16, local_ip, 4);
    w16(ip + 10, checksum(ip, 20, 0));
    uint8_t *t = ip + 20;
    w16(t, 40000);
    w16(t + 2, 8080);
    w32(t + 4, seq);
    w32(t + 8, ack);
    t[12] = 0x50;
    t[13] = (uint8_t)flags;
    w16(t + 14, window);
    if (n)
    {
        memcpy(t + 20, data, n);
    }
    uint32_t pseudo = r16(peer_ip) + r16(peer_ip + 2) + r16(local_ip) +
                      r16(local_ip + 2) + 6 + 20 + (uint32_t)n;
    w16(t + 16, checksum(t, n + 20, pseudo));
    p->length = 54 + n;
    if (p->length < 60)
    {
        p->length = 60;
    }
}

static int prepare(struct net_stack *s, uint32_t seq)
{
    struct net_packet p, o;
    net_init(s, UINT32_MAX - 20);
    arp(&p);
    CHECK(net_receive(s, p.bytes, p.length, 0, &o) == 1 &&
          r16(o.bytes + 20) == 2);
    tcp(&p, seq, 0, 2, 512, 0, 0);
    CHECK(net_receive(s, p.bytes, p.length, 1, &o) == 1 && o.bytes[47] == 0x12);
    CHECK(s->state == NET_SYN_RECEIVED);
    tcp(&p, seq + 1, s->snd_nxt, 16, 512, 0, 0);
    CHECK(net_receive(s, p.bytes, p.length, 20, &o) == 0 &&
          s->state == NET_ESTABLISHED);
    return 0;
}

static int packets(void)
{
    uint8_t odd[] = {0, 1, 2, 3, 4};
    CHECK(net_checksum(odd, 5, 0) == checksum(odd, 5, 0));
    struct net_stack s;
    struct net_packet p, o;
    CHECK(!prepare(&s, 100));
    tcp(&p, 101, s.snd_nxt, 16, 512, 0, 0);
    p.bytes[14] = 0x46;
    unsigned before = s.drops;
    CHECK(net_receive(&s, p.bytes, p.length, 21, &o) == 0 &&
          s.drops == before + 1);
    tcp(&p, 101, s.snd_nxt, 16, 512, 0, 0);
    p.bytes[54] ^= 1; /* Ethernet pad does not belong to IP/TCP. */
    CHECK(net_receive(&s, p.bytes, p.length, 22, &o) == 0 &&
          s.drops == before + 1);
    p.bytes[40] ^= 1;
    CHECK(net_receive(&s, p.bytes, p.length, 23, &o) == 0 &&
          s.drops == before + 2);
    return 0;
}

static int arp_test(void)
{
    struct net_stack s;
    struct net_packet p, o;
    net_init(&s, 1000);
    arp(&p);
    CHECK(net_receive(&s, p.bytes, p.length, 0, &o) == 1 && s.peer_known &&
          s.arp_expires == 30000);
    CHECK(o.length == 60 && r16(o.bytes + 20) == 2 &&
          !memcmp(o.bytes + 22, local_mac, 6));
    p.bytes[22] ^= 1;
    unsigned drops = s.drops;
    CHECK(net_receive(&s, p.bytes, p.length, 1, &o) == 0 &&
          s.drops == drops + 1 && s.arp_expires == 30000);
    return 0;
}

static int sequence(void)
{
    struct net_stack s;
    struct net_packet p, o;
    CHECK(!prepare(&s, UINT32_MAX - 10));
    uint32_t start = s.rcv_nxt;
    const char *a = "GET /";
    tcp(&p, start, s.snd_nxt, 24, 512, a, 5);
    CHECK(net_receive(&s, p.bytes, p.length, 21, &o) == 1 &&
          s.rcv_nxt == start + 5);
    tcp(&p, start, s.snd_nxt, 24, 512, a, 5);
    CHECK(net_receive(&s, p.bytes, p.length, 22, &o) == 1 &&
          s.rcv_nxt == start + 5);
    tcp(&p, s.rcv_nxt + 3, s.snd_nxt, 24, 512, "gap", 3);
    CHECK(net_receive(&s, p.bytes, p.length, 23, &o) == 1 &&
          s.rcv_nxt == start + 5);
    tcp(&p, s.rcv_nxt + 1, s.snd_nxt, 4, 512, 0, 0);
    CHECK(net_receive(&s, p.bytes, p.length, 24, &o) == 1 &&
          s.state == NET_ESTABLISHED);
    tcp(&p, s.rcv_nxt, s.snd_nxt + 99, 16, 0, 0, 0);
    CHECK(net_receive(&s, p.bytes, p.length, 25, &o) == 1 &&
          s.peer_window == 512);
    tcp(&p, s.rcv_nxt, s.snd_nxt, 4, 512, 0, 0);
    CHECK(net_receive(&s, p.bytes, p.length, 26, &o) == 0 &&
          s.state == NET_LISTEN && s.resets == 1);
    return 0;
}

static int timers(void)
{
    struct net_stack s;
    struct net_packet p, o;
    net_init(&s, 1000);
    net_rtt(&s, 1000);
    CHECK(s.srtt == 1000 && s.rttvar == 500 && s.rto == 3000);
    net_rtt(&s, 1200);
    CHECK(s.srtt == 1025 && s.rttvar == 425 && s.rto == 2725);
    net_init(&s, 1000);
    net_rtt(&s, UINT32_MAX);
    net_rtt(&s, UINT32_MAX);
    CHECK(s.srtt == UINT32_MAX && s.rttvar == 1610612735u && s.rto == 60000);
    net_init(&s, 1000);
    arp(&p);
    net_receive(&s, p.bytes, p.length, 0, &o);
    tcp(&p, 50, 0, 2, 512, 0, 0);
    CHECK(net_receive(&s, p.bytes, p.length, 0, &o) == 1);
    CHECK(net_tick(&s, 999, &o) == 0);
    CHECK(net_tick(&s, 1000, &o) == 1 && s.rto == 2000 && s.retransmitted);
    tcp(&p, 51, s.snd_nxt, 16, 512, 0, 0);
    CHECK(net_receive(&s, p.bytes, p.length, 1100, &o) == 0 && s.rto == 3000 &&
          s.rtt_samples == 0);
    net_link_down(&s);
    CHECK(s.state == NET_LISTEN && !s.peer_known && s.incarnation == 1);
    return 0;
}

static int http(void)
{
    struct net_stack s;
    struct net_packet p, o;
    CHECK(!prepare(&s, 100));
    const char *request =
        "GET /health HTTP/1.1\r\nHost: 192.0.2.2:8080\r\n\r\n";
    size_t n = strlen(request);
    uint32_t peer = 101;
    for (size_t i = 0; i < n; i++)
    {
        tcp(&p, peer++, s.snd_nxt, 24, 512, request + i, 1);
        CHECK(net_receive(&s, p.bytes, p.length, 21 + i, &o) == 1);
    }
    CHECK(s.response_length && s.flight);
    char response[1700] = {0};
    size_t have = 0;
    uint64_t now = 100;
    unsigned rounds = 0;
    while (s.state == NET_ESTABLISHED && rounds++ < 20)
    {
        CHECK(o.length >= 54);
        size_t total = r16(o.bytes + 16);
        size_t h = (o.bytes[46] >> 4) * 4;
        size_t bytes = total - 20 - h;
        CHECK(have + bytes < sizeof response);
        memcpy(response + have, o.bytes + 34 + h, bytes);
        have += bytes;
        uint32_t ack = r32(o.bytes + 38) + (uint32_t)bytes;
        tcp(&p, peer, ack, 16, 512, 0, 0);
        CHECK(net_receive(&s, p.bytes, p.length, now++, &o) >= 0);
    }
    CHECK(strstr(response, "HTTP/1.1 200 OK\r\n") &&
          strstr(response, "Content-Length: 3\r\n") && have >= 3 &&
          !memcmp(response + have - 3, "ok\n", 3));
    CHECK(s.state == NET_FIN_WAIT_1 && o.bytes[47] == 0x11);
    tcp(&p, peer, s.snd_nxt, 17, 512, 0, 0);
    CHECK(net_receive(&s, p.bytes, p.length, now++, &o) == 1 &&
          s.state == NET_TIME_WAIT);
    uint64_t expiry = s.timewait_until;
    tcp(&p, peer + 9, s.snd_nxt, 17, 512, 0, 0);
    CHECK(net_receive(&s, p.bytes, p.length, now++, &o) == 0 &&
          s.timewait_until == expiry);
    tcp(&p, peer, s.snd_nxt, 17, 512, 0, 0);
    CHECK(net_receive(&s, p.bytes, p.length, now++, &o) == 1 &&
          s.timewait_until > expiry);
    expiry = s.timewait_until;
    CHECK(net_tick(&s, expiry - 1, &o) == 0 && s.state == NET_TIME_WAIT);
    CHECK(net_tick(&s, expiry, &o) == 0 && s.state == NET_LISTEN);
    return 0;
}

static int zero_window(void)
{
    struct net_stack s;
    struct net_packet p, o;
    CHECK(!prepare(&s, 100));
    const char *r = "GET /health HTTP/1.1\r\nHost: 192.0.2.2:8080\r\n\r\n";
    tcp(&p, 101, s.snd_nxt, 24, 0, r, strlen(r));
    CHECK(net_receive(&s, p.bytes, p.length, 30, &o) == 1 && !s.flight &&
          s.response_length);
    uint32_t seq = s.snd_nxt;
    CHECK(net_tick(&s, s.due, &o) == 1 && s.snd_nxt == seq &&
          r32(o.bytes + 38) == seq - 1);
    tcp(&p, s.rcv_nxt, s.snd_nxt, 16, 256, 0, 0);
    CHECK(net_receive(&s, p.bytes, p.length, s.now + 1, &o) == 1 && s.flight);
    return 0;
}

static int icmp(void)
{
    struct net_stack s;
    struct net_packet p, o;
    net_init(&s, 1000);
    arp(&p);
    CHECK(net_receive(&s, p.bytes, p.length, 0, &o) == 1);
    tcp(&p, 0, 0, 16, 512, 0, 0);
    uint8_t *ip = p.bytes + 14, *msg = ip + 20;
    ip[9] = 1;
    w16(ip + 2, 29);
    w16(ip + 10, 0);
    w16(ip + 10, checksum(ip, 20, 0));
    memset(msg, 0, 9);
    msg[0] = 8;
    w16(msg + 4, 123);
    w16(msg + 6, 9);
    msg[8] = 0xab;
    w16(msg + 2, checksum(msg, 9, 0));
    p.length = 60;
    CHECK(net_receive(&s, p.bytes, p.length, 1, &o) == 1);
    CHECK(o.bytes[34] == 0 && r16(o.bytes + 38) == 123 &&
          r16(o.bytes + 40) == 9 && o.bytes[42] == 0xab &&
          checksum(o.bytes + 34, 9, 0) == 0);
    p.bytes[42] ^= 1;
    CHECK(net_receive(&s, p.bytes, p.length, 2, &o) == 0 && !o.length);
    return 0;
}

static int expect_continue(void)
{
    struct net_stack s;
    struct net_packet p, o;
    CHECK(!prepare(&s, 100));
    const char *head =
        "POST /echo HTTP/1.1\r\nHost: 192.0.2.2:8080\r\nContent-Length: "
        "3\r\nExpect: 100-continue\r\n\r\n";
    tcp(&p, 101, s.snd_nxt, 24, 512, head, strlen(head));
    CHECK(net_receive(&s, p.bytes, p.length, 30, &o) == 1);
    CHECK(r16(o.bytes + 16) == 65 &&
          !memcmp(o.bytes + 54, "HTTP/1.1 100 Continue\r\n\r\n", 25));
    tcp(&p, s.rcv_nxt, s.snd_nxt, 24, 512, "abc", 3);
    CHECK(net_receive(&s, p.bytes, p.length, 31, &o) == 1);
    CHECK(s.response_length && s.response_sent &&
          s.http.request.body_length == 3 &&
          !memcmp(s.http.request.body, "abc", 3));
    return 0;
}

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        return 2;
    }
    int r = !strcmp(argv[1], "packets")       ? packets()
            : !strcmp(argv[1], "arp")         ? arp_test()
            : !strcmp(argv[1], "sequence")    ? sequence()
            : !strcmp(argv[1], "timers")      ? timers()
            : !strcmp(argv[1], "http")        ? http()
            : !strcmp(argv[1], "zero_window") ? zero_window()
            : !strcmp(argv[1], "icmp")        ? icmp()
            : !strcmp(argv[1], "continue")    ? expect_continue()
                                              : 2;
    if (!r)
    {
        printf("PASS net_%s\n", argv[1]);
    }
    return r;
}
