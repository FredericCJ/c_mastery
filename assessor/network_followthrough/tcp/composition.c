#include "test_wire.h"

int main(void)
{
    struct nc_stack s;
    struct nc_packet o;
    struct http_parser http;
    unsigned char packet[1500];
    const char request[] =
        "POST /echo HTTP/1.1\r\nHost: "
        "192.0.2.2:8080\r\nContent-Length: 4\r\n\r\nABCDNEXT";
    CHECK(!nc_establish(&s, 7, 1000, 30, 256));
    http_init(&http, "192.0.2.2:8080");
    size_t at = 0, consumed = 0, total = sizeof request - 1;
    int result = HTTP_NEED;
    while (at < total)
    {
        size_t n = total - at;
        if (n > 7)
        {
            n = 7;
        }
        size_t len = nc_wire(packet, s.rcv_nxt, s.snd_nxt, 0x18, 512,
                             request + at, n, 0);
        CHECK(nc_receive(&s, packet, len, 81 + at) == NC_OK);
        at += n;
        while (result == HTTP_NEED || result == HTTP_CONTINUE)
        {
            size_t used;
            result = nc_http_pump(&s, &http, 3, &used);
            consumed += used;
            if (!s.rx.count || !used || result == HTTP_DONE)
            {
                break;
            }
        }
    }
    CHECK(result == HTTP_DONE && http.request.body_length == 4 &&
          !memcmp(http.request.body, "ABCD", 4));
    CHECK(consumed == total - 4 && s.rx.count == 4);
    CHECK(nw_rx_window(&s.rx) == 3);
    /* DONE retains the next-message prefix; FIN is transport half-close only. */
    size_t n = nc_wire(packet, s.rcv_nxt, s.snd_nxt, 0x11, 512, 0, 0, 0);
    CHECK(nc_receive(&s, packet, n, 200) == NC_OK);
    CHECK(s.peer_fin && nc_http_eof(&s, &http) == HTTP_NEED);
    CHECK(nc_consume(&s, 4) == NC_OK);
    CHECK(nc_http_eof(&s, &http) == HTTP_DONE);
    CHECK(!nc_establish(&s, 128, 1000, 30, 256));
    http_init(&http, "192.0.2.2:8080");
    const char partial[] = "POST /echo HTTP/1.1\r\nHost: "
                           "192.0.2.2:8080\r\nContent-Length: 4\r\n\r\nAB";
    n = nc_wire(packet, s.rcv_nxt, s.snd_nxt, 0x19, 512, partial,
                sizeof partial - 1, 0);
    CHECK(nc_receive(&s, packet, n, 81) == NC_OK);
    size_t used;
    CHECK(nc_http_pump(&s, &http, 512, &used) == HTTP_NEED);
    CHECK(nc_http_eof(&s, &http) == HTTP_ERROR && http.request.status == 400);
    /* Same copied1024-byte application result, explicit deterministic RTT100
     * service model; neither value is a measured network throughput. */
    unsigned counts[2] = {0, 0};
    uint64_t finishes[2] = {0, 0};
    unsigned char data[1024];
    for (unsigned i = 0; i < 1024; i++)
    {
        data[i] = (unsigned char)i;
    }
    for (unsigned policy = 0; policy < 2; policy++)
    {
        CHECK(!nc_establish(&s, 512, 0xfffffff0, 30, policy ? 256 : 64));
        CHECK(nc_offer(&s, data, sizeof data) == NC_OK);
        size_t received = 0;
        uint64_t now = 100;
        while (received < sizeof data)
        {
            CHECK(nc_prepare(&s, &o) == NC_OK);
            size_t bytes = o.length - 40;
            CHECK(!memcmp(o.bytes + 40, data + received, bytes));
            CHECK(nc_submit(&s, o.ticket, 0, now) == NC_WAIT);
            CHECK(nc_submit(&s, o.ticket, 1, now + 3) == NC_OK);
            memset(data + received, 0, bytes);
            CHECK(nc_dma_done(&s, o.ticket) == NC_OK);
            CHECK(s.flight);
            received += bytes;
            counts[policy]++;
            now += 103;
            n = nc_wire(packet, s.rcv_nxt, s.snd_nxt, 16, 512, 0, 0, 0);
            CHECK(nc_receive(&s, packet, n, now) == NC_OK);
        }
        CHECK(!s.flight && s.tx_at == 1024);
        finishes[policy] = now;
        for (unsigned i = 0; i < 1024; i++)
        {
            data[i] = (unsigned char)i;
        }
    }
    CHECK(counts[0] == 16 && counts[1] == 4 && finishes[0] == 1748 &&
          finishes[1] == 512);
    puts("NC_COMPOSITION_PASS actual_Ni_SChecksum_NwRx_HTTP two_policies_16_4");
    return 0;
}
