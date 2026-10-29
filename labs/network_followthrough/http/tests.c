/* Original synthetic peer. It reconstructs TCP bytes by sequence and checks
 * literal HTTP expectations; it never calls the response encoder as oracle. */
#define main nb_predecessor_tests
#include "../driver/tests.c"
#undef main
#include "http.h"
#undef CHECK
#define CHECK(x)                  \
    do                            \
    {                             \
        if (!(x))                 \
        {                         \
            puts("NH_CHECK " #x); \
            return 1;             \
        }                         \
    } while (0)
static struct nh_http app;
static unsigned char request[1800], capture[4096], body[1024];
static size_t request_length, captured, sent;
static uint32_t server_next, client_next, client_ack;
static unsigned server_window, client_window, peer_fin, server_fin, date_calls;
static uint64_t time_ms;

static const char *date_value(void *p)
{
    date_calls++;
    return p;
}

static const char stamp[] = "Sun, 18 Oct 2026 01:02:03 GMT";

static uint32_t read32(const unsigned char *p)
{
    return (uint32_t)nc_u16(p) << 16 | nc_u16(p + 2);
}

static void append_request(const void *p, size_t n)
{
    memcpy(request + request_length, p, n);
    request_length += n;
}

static void text_request(const char *p)
{
    append_request(p, strlen(p));
}

static int start(unsigned mss, unsigned window, unsigned capacity,
                 unsigned date)
{
    CHECK(!boot());
    CHECK(nb_init(&net, &ring, config, 1, 1000, capacity, 0) == NB_OK);
    CHECK(nh_init(&app, &net, "mcu.test", date ? date_value : 0, (void *)stamp,
                  0) == NH_OK);
    captured = sent = date_calls = peer_fin = server_fin = 0;
    server_next = 1000;
    client_next = client_ack = 31;
    server_window = capacity;
    client_window = window;
    time_ms = 0;
    unsigned char p[300];
    size_t n = nc_wire(p, 30, 0, 2, window, 0, 0, mss);
    CHECK(inject(p, n) == ED_OK);
    return 0;
}

static int receive_output(void)
{
    for (unsigned i = 0; i < 2; i++)
    {
        int e = en_model_tx(&model, 0);
        if (e == ED_BUSY)
            break;
        CHECK(e == ED_OK);
        const unsigned char *f = model.wire;
        CHECK(model.wire_length >= 60);
        if (nc_u16(f + 12) == 0x806)
        {
            CHECK(arp_reply() == ED_OK);
            continue;
        }
        CHECK(nc_u16(f + 12) == 0x800);
        const unsigned char *p = f + 14;
        unsigned total = nc_u16(p + 2);
        CHECK(total >= 40 && total + 14 <= model.wire_length && p[0] == 0x45 &&
              p[9] == 6 && !nc_sum(p, 20, 0));
        unsigned pseudo = nc_u16(p + 12) + nc_u16(p + 14) + nc_u16(p + 16) +
                          nc_u16(p + 18) + 6 + total - 20;
        CHECK(!nc_sum(p + 20, total - 20, pseudo));
        unsigned h = (p[32] >> 4) * 4, flags = p[33];
        CHECK(h >= 20 && total >= 20 + h);
        size_t n = total - 20 - h;
        uint32_t seq = read32(p + 24), ack = read32(p + 28);
        CHECK(ack >= client_ack && ack <= client_next);
        client_ack = ack;
        server_window = nc_u16(p + 34);
        CHECK(n <= client_window);
        if (n || (flags & 3))
        {
            CHECK(seq == server_next && captured + n <= sizeof capture);
            memcpy(capture + captured, p + 20 + h, n);
            captured += n;
            server_next += (uint32_t)n;
            if (flags & 2)
                server_next++;
            if (flags & 1)
            {
                server_next++;
                server_fin = 1;
            }
            unsigned char reply[300];
            size_t z = nc_wire(reply, client_next, server_next, 16,
                               client_window, 0, 0, 0);
            CHECK(inject(reply, z) == ED_OK);
        }
    }
    return 0;
}

static int exchange(unsigned chunk, unsigned end_fin)
{
    for (unsigned steps = 0; steps < 30000; steps++)
    {
        int e = nh_step(&app, time_ms++);
        CHECK(e == NH_WAIT || e == NH_DONE);
        CHECK(!receive_output());
        if (e == NH_DONE)
        {
            CHECK(server_fin && app.requests == 1 && !model.violation &&
                  net.rx_frames == net.rx_released);
            return 0;
        }
        if (net.tcp.state == NC_ESTABLISHED && client_ack == client_next &&
            !server_fin)
        {
            if (sent < request_length && server_window)
            {
                size_t n = request_length - sent;
                if (n > chunk)
                    n = chunk;
                if (n > server_window)
                    n = server_window;
                unsigned char p[300];
                size_t z = nc_wire(p, client_next, server_next, 16,
                                   client_window, request + sent, n, 0);
                CHECK(inject(p, z) == ED_OK);
                sent += n;
                client_next += (uint32_t)n;
            }
            else if (sent == request_length && end_fin && !peer_fin &&
                     server_window)
            {
                unsigned char p[300];
                size_t z = nc_wire(p, client_next, server_next, 17,
                                   client_window, 0, 0, 0);
                CHECK(inject(p, z) == ED_OK);
                client_next++;
                peer_fin = 1;
            }
        }
        if (server_fin && !peer_fin)
        {
            unsigned char p[300];
            size_t z = nc_wire(p, client_next, server_next, 17, client_window,
                               0, 0, 0);
            CHECK(inject(p, z) == ED_OK);
            client_next++;
            peer_fin = 1;
        }
    }
    CHECK(0);
    return 1;
}

static void make_request(size_t n, unsigned expect, unsigned truncated,
                         unsigned pipeline)
{
    request_length = 0;
    text_request("POST /echo HTTP/1.1\r\nHost: mcu.test\r\nTransfer-Encoding: "
                 "chunked\r\n");
    if (expect)
        text_request("Expect: 100-continue\r\n");
    text_request("\r\n");
    for (size_t i = 0; i < n; i++)
        body[i] = (unsigned char)((i * 73 + 17) & 255);
    body[0] = 0;
    body[1] = '\r';
    body[2] = '\n';
    if (n == 12)
    {
        text_request("5\r\n");
        append_request(body, 5);
        text_request("\r\n7\r\n");
        append_request(body + 5, 7);
    }
    else
    {
        text_request("400\r\n");
        append_request(body, n);
    }
    text_request("\r\n0\r\nX-Synthetic: retained\r\n");
    if (!truncated)
        text_request("\r\n");
    if (pipeline)
        text_request("GET /health HTTP/1.1\r\nHost: mcu.test\r\n\r\n");
}

static int expected(size_t n, unsigned date, unsigned interim, unsigned error)
{
    unsigned char want[2000];
    size_t z = 0;
#define ADD(p)                  \
    do                          \
    {                           \
        const char *t = (p);    \
        size_t k = strlen(t);   \
        memcpy(want + z, t, k); \
        z += k;                 \
    } while (0)
    if (interim)
        ADD("HTTP/1.1 100 Continue\r\n\r\n");
    ADD(error ? "HTTP/1.1 400 Bad Request\r\n" : "HTTP/1.1 200 OK\r\n");
    if (date)
    {
        ADD("Date: ");
        ADD(stamp);
        ADD("\r\n");
    }
    ADD(error ? "Content-Type: text/plain; charset=utf-8\r\n"
              : "Content-Type: application/octet-stream\r\n");
    ADD(error || n == 12 ? "Content-Length: 12\r\n"
                         : "Content-Length: 1024\r\n");
    ADD("Connection: close\r\n\r\n");
    if (error)
        ADD("bad request\n");
    else
    {
        memcpy(want + z, body, n);
        z += n;
    }
#undef ADD
    CHECK(captured == z && !memcmp(capture, want, z));
    CHECK(date_calls == date);
    return 0;
}

static int streams(void)
{
    const unsigned chunks[] = {1, 7, 16}, mss[] = {1, 17, 256},
                   windows[] = {1, 9, 512};
    unsigned cases = 0;
    for (unsigned a = 0; a < 3; a++)
        for (unsigned b = 0; b < 3; b++)
            for (unsigned c = 0; c < 3; c++)
            {
                unsigned date = (a + b + c) % 2;
                make_request(12, 0, 0, 0);
                CHECK(!start(mss[b], windows[c], 16, date));
                CHECK(!exchange(chunks[a], 1));
                CHECK(!expected(12, date, 0, 0));
                cases++;
            }
    for (unsigned a = 0; a < 3; a++)
    {
        make_request(1024, a == 0, 0, 0);
        CHECK(!start(mss[a], windows[2 - a], 16, a == 1));
        CHECK(!exchange(chunks[a], 1));
        CHECK(!expected(1024, a == 1, a == 0, 0));
        cases++;
    }
    make_request(12, 0, 1, 0);
    CHECK(!start(17, 9, 16, 0));
    CHECK(!exchange(7, 1));
    CHECK(!expected(12, 0, 0, 1));
    cases++;
    make_request(12, 0, 0, 1);
    CHECK(!start(256, 512, 512, 0));
    CHECK(!exchange(256, 0));
    CHECK(!expected(12, 0, 0, 0));
    CHECK(sent == request_length && net.tcp.rx.count > 0);
    cases++;
    CHECK(cases == 32);
    puts("NH_STREAMS_PASS "
         "32_raw_peer_cases_MSS_window_binary_halfclose_no_pipeline");
    return 0;
}

static int failure(void)
{
    make_request(12, 0, 0, 0);
    CHECK(!start(17, 0, 16, 0));
    /* Complete handshake and request despite peer's zero response window. */
    for (unsigned i = 0; i < 1000 && app.state != NH_CLOSING; i++)
    {
        CHECK(nh_step(&app, time_ms++) == NH_WAIT);
        CHECK(!receive_output());
        if (net.tcp.state == NC_ESTABLISHED && client_ack == client_next &&
            sent < request_length && server_window)
        {
            size_t n = request_length - sent;
            if (n > 16)
                n = 16;
            if (n > server_window)
                n = server_window;
            unsigned char p[300];
            size_t z = nc_wire(p, client_next, server_next, 16, 0,
                               request + sent, n, 0);
            CHECK(inject(p, z) == ED_OK);
            client_next += (uint32_t)n;
            sent += n;
        }
    }
    CHECK(app.state == NH_CLOSING && net.tcp.tx_length > 0 && !net.tcp.flight);
    CHECK(nh_step(&app, app.output_started + 15000) == NH_ABORT &&
          net.phase == NB_STOPPING);
    /* Old application object must not mutate a freshly rebound transport. */
    CHECK(!boot());
    CHECK(nh_init(&app, &net, "mcu.test", 0, 0, 0) == NH_OK);
    net.tcp.epoch = 2;
    CHECK(nh_step(&app, 1) == NH_ABORT && net.tcp.state == NC_LISTEN &&
          net.phase == NB_RUNNING);
    puts("NH_FAILURE_PASS zero_window_absolute_abort_stale_application_epoch");
    return 0;
}

static int link_recovery(void)
{
    make_request(12, 0, 0, 0);
    CHECK(!start(17, 9, 16, 0));
    unsigned stopped = 0;
    for (unsigned step = 0; step < 1000; step++)
    {
        CHECK(nh_step(&app, time_ms++) == NH_WAIT);
        if (net.tcp.tx_length && net.tcp.dma && net.tcp.flight)
        {
            stopped = 1;
            break;
        }
        CHECK(!receive_output());
        if (net.tcp.state == NC_ESTABLISHED && client_ack == client_next &&
            sent < request_length && server_window)
        {
            size_t n = request_length - sent;
            if (n > 16)
                n = 16;
            if (n > server_window)
                n = server_window;
            unsigned char p[300];
            size_t z = nc_wire(p, client_next, server_next, 16, client_window,
                               request + sent, n, 0);
            CHECK(inject(p, z) == ED_OK);
            sent += n;
            client_next += (uint32_t)n;
        }
    }
    CHECK(stopped);
    uint32_t old_ack = net.tcp.snd_nxt;
    struct nc_ticket old = {net.tcp.epoch, net.tcp.dma_serial};
    model.stop_stuck = 1;
    phy_model.phy[0][1] = 0x20;
    CHECK(nh_step(&app, time_ms++) == NH_ABORT && net.tcp.state == NC_CLOSED &&
          net.tcp.dma);
    CHECK(nb_stop_step(&net, time_ms++) == NB_WAIT && net.tcp.dma);
    model.reg[0x1014 / 4] &= ~0x7e0000u;
    CHECK(nb_stop_step(&net, time_ms++) == NB_STOPPED && !net.tcp.dma);
    phy_model.phy[0][1] = 0x24;
    CHECK(ep_begin(&phy, 16, 1000) == ED_BUSY);
    int e = ED_BUSY;
    for (unsigned i = 0; i < 16 && e == ED_BUSY; i++)
        e = ep_step(&phy);
    CHECK(e == ED_OK);
    CHECK(en_reset_begin(&ring, 0, 4, 100) == ED_BUSY &&
          en_reset_step(&ring, 1) == ED_BUSY &&
          en_reset_step(&ring, 2) == ED_BUSY &&
          en_reset_step(&ring, 3) == ED_OK && en_start(&ring) == ED_OK);
    CHECK(nb_restart(&net, 2, 5000, 16, time_ms++) == NB_OK &&
          nc_dma_done(&net.tcp, old) == NC_STALE);
    unsigned char replay[300];
    size_t z = nc_wire(replay, client_next, old_ack, 16, 512, 0, 0, 0);
    CHECK(inject(replay, z) == ED_OK && nb_step(&net, time_ms++) == NB_OK &&
          net.tcp.state == NC_LISTEN && net.tcp.snd_nxt == 5000);
    CHECK(nh_step(&app, time_ms++) == NH_ABORT && net.tcp.state == NC_LISTEN);
    /* A new application and handshake use a new ISN. Replaying the old wire ACK
     * while SYN-ACK is outstanding cannot establish this incarnation. */
    CHECK(nh_init(&app, &net, "mcu.test", 0, 0, time_ms) == NH_OK);
    z = nc_wire(replay, 700, 0, 2, 512, 0, 0, 17);
    CHECK(inject(replay, z) == ED_OK && nb_step(&net, time_ms++) == NB_OK);
    CHECK(en_model_tx(&model, 0) == ED_OK && nc_u16(model.wire + 12) == 0x806);
    CHECK(arp_reply() == ED_OK && nb_step(&net, time_ms++) == NB_OK &&
          net.tcp.snd_nxt == 5001);
    CHECK(en_model_tx(&model, 0) == ED_OK && nb_step(&net, time_ms++) == NB_OK);
    z = nc_wire(replay, 701, old_ack, 16, 512, 0, 0, 0);
    CHECK(inject(replay, z) == ED_OK && nb_step(&net, time_ms++) == NB_OK &&
          net.tcp.state == NC_SYN_RECEIVED);
    /* The challenged old ACK may emit a separate ACK descriptor; reap it first.
     */
    if (en_model_tx(&model, 0) == ED_OK)
        CHECK(nb_step(&net, time_ms++) == NB_OK);
    z = nc_wire(replay, 701, 5001, 16, 512, 0, 0, 0);
    CHECK(inject(replay, z) == ED_OK && nb_step(&net, time_ms++) == NB_OK &&
          net.tcp.state == NC_ESTABLISHED);
    puts("NH_LINK_PASS "
         "app_abort_DMA_quiescence_new_handshake_old_wire_ACK_rejected");
    return 0;
}
#ifdef NB_TARGET
#define main nh_public_tests
#endif
int main(void)
{
    struct nc_stack old;
    CHECK(!nc_establish(&old, 16, 1000, 30, 100));
    CHECK(!streams());
    CHECK(!failure());
    CHECK(!link_recovery());
    puts("NH_PASS actual_driver_parser_encoder_raw_peer");
    return 0;
}
