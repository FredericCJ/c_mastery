#include "com_net.h"
#include "stream.h"
#include "wire.h"
#include <stdio.h>
#include <string.h>
#define CHECK(x)                      \
    do                                \
    {                                 \
        if (!(x))                     \
        {                             \
            puts("NW_INTEGRATE " #x); \
            return 1;                 \
        }                             \
    } while (0)
static const unsigned char request[] =
    "POST /echo HTTP/1.1\r\nHost: 192.0.2.2:8080\r\nContent-Length: "
    "4\r\n\r\nA\0B\xff";
static const unsigned char suffix[] = {'N', 'E', 'X', 'T'};

static void word(unsigned char *p, unsigned v)
{
    p[0] = (unsigned char)(v >> 8);
    p[1] = (unsigned char)v;
}

static unsigned checksum(const unsigned char *p, size_t n, unsigned sum)
{
    for (size_t i = 0; i < n; ++i)
        sum += (unsigned)p[i] << (i % 2 ? 0 : 8);
    while (sum > 65535)
        sum = (sum & 65535) + (sum >> 16);
    return 65535 - sum;
}

static int offer(struct nw_rx *q, uint32_t *seq, const unsigned char *data,
                 size_t n)
{
    unsigned char packet[600] = {2, 0, 0, 0, 0, 2, 2, 0, 0, 0, 0, 1, 8, 0};
    unsigned char *ip = packet + 14, *tcp = packet + 34;
    ip[0] = 0x45;
    word(ip + 2, (unsigned)n + 40);
    ip[8] = 64;
    ip[9] = 6;
    unsigned char addresses[8] = {192, 0, 2, 1, 192, 0, 2, 2};
    memcpy(ip + 12, addresses, 8);
    word(ip + 10, checksum(ip, 20, 0));
    word(tcp, 40000);
    word(tcp + 2, 8080);
    word(tcp + 4, *seq >> 16);
    word(tcp + 6, *seq);
    tcp[12] = 0x50;
    tcp[13] = 0x18;
    word(tcp + 14, 512);
    memcpy(tcp + 20, data, n);
    word(tcp + 16,
         checksum(tcp, n + 20,
                  0xc000 + 0x201 + 0xc000 + 0x202 + 6 + (unsigned)n + 20));
    size_t size = 54 + n;
    if (size < 60)
        size = 60;
    int rc = nw_ingress(q, seq, 40000, packet, size);
    memset(packet, 0, sizeof packet);
    return rc;
}
#ifdef NW_TARGET
int nw_integration(void)
{
    const size_t stride = 23;
#else
int main(void)
{
    const size_t stride = 1;
#endif
    unsigned partitions = 0;
    const size_t length = sizeof request - 1;
    for (size_t first = 1; first < length; first += stride)
    {
        for (size_t second = first; second < length; second += stride)
        {
            for (unsigned budget_case = 0; budget_case < 3; ++budget_case)
            {
                const unsigned budgets[3] = {1, 7, 512};
                struct nw_rx q;
                struct http_parser parser;
                uint32_t seq = UINT32_MAX - 10;
                CHECK(nw_rx_init(&q, 512) == NW_OK);
                http_init(&parser, "192.0.2.2:8080");
                CHECK(offer(&q, &seq, request, first) == NW_OK);
                CHECK(offer(&q, &seq, request + first, second - first) ==
                      NW_OK);
                unsigned char final[128];
                memcpy(final, request + second, length - second);
                memcpy(final + length - second, suffix, sizeof suffix);
                CHECK(offer(&q, &seq, final, length - second + sizeof suffix) ==
                      NW_OK);
                CHECK(seq == UINT32_MAX - 10 + (uint32_t)length + 4);
                memset(final, 0, sizeof final); /* Borrow expires on return. */
                int rc = HTTP_NEED;
                size_t consumed = 0;
                while (rc == HTTP_NEED)
                {
                    size_t used = 999;
                    rc = nw_http_pump(&q, &parser, budgets[budget_case], &used);
                    CHECK(used && used <= budgets[budget_case]);
                    consumed += used;
                }
                CHECK(rc == HTTP_DONE && consumed == length && q.count == 4);
                CHECK(parser.request.body_length == 4 &&
                      !memcmp(parser.request.body, request + length - 4, 4));
                const unsigned char *left;
                size_t n;
                CHECK(nw_rx_peek(&q, &left, &n) == NW_OK && n == 4 &&
                      !memcmp(left, suffix, 4));
                size_t used = 999;
                CHECK(nw_http_pump(&q, &parser, 512, &used) == HTTP_DONE &&
                      used == 0 && q.count == 4);
                ++partitions;
            }
        }
    }
    /* Retained provider parses exact IP total length before its HTTP consumer.
     * net_checksum's result is compared with a byte-oriented independent sum. */
    unsigned char odd[5] = {0, 1, 2, 3, 4};
    CHECK(net_checksum(odd, 5, 0) == (uint16_t)(65535 - (1 + 515 + 1024)));
    struct nw_rx small;
    uint32_t seq = 100;
    CHECK(nw_rx_init(&small, 1) == NW_OK);
    CHECK(offer(&small, &seq, odd, 5) == NW_BUSY && seq == 100 &&
          small.count == 0);
#ifdef NW_TARGET
    CHECK(partitions == 18);
    puts("NW_INTEGRATE_PASS target_three_segment_budget_cases=18 borrowed_copy "
         "leftover=4");
#else
    printf("NW_INTEGRATE_PASS three_segment_budget_cases=%u borrowed_copy "
           "leftover=4\n",
           partitions);
#endif
    return 0;
}
