#include "arp.h"
#include "scope.h"
#include <stdio.h>
#include <string.h>
#define CHECK(x)                    \
    do                              \
    {                               \
        if (!(x))                   \
        {                           \
            puts("NA_COMPOSE " #x); \
            return 1;               \
        }                           \
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

static void tcp(struct net_packet *p, uint32_t seq, uint32_t ack,
                unsigned flags, const void *data, size_t n)
{
    static const unsigned char mac[14] = {2, 0, 0, 0, 0, 2, 2,
                                          0, 0, 0, 0, 1, 8, 0};
    static const unsigned char ipaddr[8] = {192, 0, 2, 1, 192, 0, 2, 2};
    memset(p, 0, sizeof *p);
    memcpy(p->bytes, mac, 14);
    unsigned char *ip = p->bytes + 14, *t = p->bytes + 34;
    ip[0] = 0x45;
    w16(ip + 2, (unsigned)n + 40);
    ip[8] = 64;
    ip[9] = 6;
    memcpy(ip + 12, ipaddr, 8);
    w16(ip + 10, sum(ip, 20, 0));
    w16(t, 40000);
    w16(t + 2, 8080);
    w32(t + 4, seq);
    w32(t + 8, ack);
    t[12] = 0x50;
    t[13] = (unsigned char)flags;
    w16(t + 14, 512);
    if (n)
    {
        memcpy(t + 20, data, n);
    }
    w16(t + 16,
        sum(t, n + 20, 0xc000 + 0x201 + 0xc000 + 0x202 + 6 + (unsigned)n + 20));
    p->length = n + 54;
    if (p->length < 60)
    {
        p->length = 60;
    }
}
static struct net_stack stack;

int main(void)
{
    struct net_packet input, output, lower;
    struct na_arp arp;
    struct na_ticket ticket;
    const struct na_config config = {
        {2, 0, 0, 0, 0, 2}, {192, 0, 2, 2}, {192, 0, 2, 1}};
    const unsigned char request[42] = {
        255, 255, 255, 255, 255, 255, 2, 0, 0, 0, 0,   1, 8, 6,
        0,   1,   8,   0,   6,   4,   0, 1, 2, 0, 0,   0, 0, 1,
        192, 0,   2,   1,   0,   0,   0, 0, 0, 0, 192, 0, 2, 2};
    net_init(&stack, 1000);
    memset(&input, 0, sizeof input);
    memcpy(input.bytes, request, 42);
    input.length = 60;
    CHECK(net_receive(&stack, input.bytes, input.length, 0, &output) >= 0);
    tcp(&input, 100, 0, 2, 0, 0);
    CHECK(net_receive(&stack, input.bytes, input.length, 1, &output) >= 0 &&
          stack.state == NET_SYN_RECEIVED);
    tcp(&input, 101, stack.snd_nxt, 16, 0, 0);
    CHECK(net_receive(&stack, input.bytes, input.length, 2, &output) >= 0 &&
          stack.state == NET_ESTABLISHED);
    const char get[] = "GET /health HTTP/1.1\r\nHost: 192.0.2.2:8080\r\n\r\n";
    tcp(&input, 101, stack.snd_nxt, 24, get, sizeof get - 1);
    CHECK(net_receive(&stack, input.bytes, input.length, 3, &output) >= 0 &&
          stack.response_length);
    unsigned total = (unsigned)output.bytes[16] * 256 + output.bytes[17];
    struct nw_lengths lengths;
    CHECK(nw_lengths(total - 40, &lengths) == NW_OK &&
          lengths.pre_fcs == output.length);
    unsigned char saved[1500];
    memcpy(saved, output.bytes + 14, total);
    CHECK(na_init(&arp, config, 7) == NA_OK &&
          na_queue(&arp, output.bytes + 14, total, 0) == NA_OK);
    memset(&output, 0, sizeof output);
    CHECK(na_prepare(&arp, 0, &output, &ticket) == NA_OK &&
          ticket.kind == NA_REQUEST && na_submit(&arp, ticket, 1) == NA_OK);
    /* A legitimate changed association is synthesized; no packet leaves process.
     */
    memset(&input, 0, sizeof input);
    memcpy(input.bytes, request, 42);
    input.bytes[11] = input.bytes[27] = 88;
    input.bytes[21] = 2;
    memcpy(input.bytes, config.local_mac, 6);
    memcpy(input.bytes + 32, config.local_mac, 6);
    CHECK(na_receive(&arp, input.bytes, 60, 1) == NA_OK);
    for (unsigned refusal = 0; refusal < 3; ++refusal)
    {
        CHECK(na_prepare(&arp, 1, &output, &ticket) == NA_OK &&
              ticket.kind == NA_DATA);
        CHECK(output.bytes[5] == 88 &&
              !memcmp(output.bytes + 14, saved, total));
        CHECK(na_submit(&arp, ticket, 0) == NA_OK && arp.pending);
    }
    CHECK(na_prepare(&arp, 2, &output, &ticket) == NA_OK);
    lower = output;
    CHECK(na_submit(&arp, ticket, 1) == NA_OK && !arp.pending);
    memset(&output, 0, sizeof output);
    CHECK(lower.bytes[5] == 88 && !memcmp(lower.bytes + 14, saved, total));
    CHECK(net_checksum(lower.bytes + 14, 20, 0) == 0);
    puts("NA_COMPOSITION_PASS actual_HTTP_TCP_IP retained_on_three_refusals "
         "learned_MAC_lower_copy");
    return 0;
}
