#include "com_edu7.h"
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

static int link_test(void)
{
    CHECK(edu_crc("123456789", 9) == 0x29b1);
    struct edu_frame f = {0, 1, 2, 40, {0}}, out;
    for (unsigned i = 0; i < 40; i++)
        f.payload[i] = (uint8_t)(i % 2 ? 0x7e : 0x7d);
    uint8_t wire[96];
    size_t n;
    CHECK(!edu_frame_encode(&f, wire, sizeof wire, &n));
    CHECK(n <= 96);
    struct edu_framer p = {0};
    unsigned publications = 0;
    for (size_t i = 0; i < n; i++)
        if (edu_frame_byte(&p, wire[i], 0, &out) == 1)
            publications++;
    CHECK(publications == 1 && !memcmp(&out, &f, sizeof f));
    wire[3] ^= 1;
    memset(&p, 0, sizeof p);
    publications = 0;
    for (size_t i = 0; i < n; i++)
        if (edu_frame_byte(&p, wire[i], 0, &out) == 1)
            publications++;
    CHECK(!publications);
    f.length = 3;
    memcpy(f.payload, "abc", 3);
    CHECK(!edu_frame_encode(&f, wire, sizeof wire, &n));
    wire[6] ^= 1; /* Payload bit: all framing/header checks still succeed. */
    memset(&p, 0, sizeof p);
    publications = 0;
    for (size_t i = 0; i < n; i++)
        if (edu_frame_byte(&p, wire[i], 0, &out) == 1)
            publications++;
    CHECK(!publications);
    memset(&p, 0, sizeof p);
    CHECK(edu_frame_byte(&p, 0x7e, 0, &out) == 0);
    CHECK(edu_frame_byte(&p, 0x7d, 0, &out) == 0);
    CHECK(edu_frame_byte(&p, 0x7e, 0, &out) == -1);
    CHECK(edu_frame_byte(&p, 1, 1, &out) == -1);
    for (unsigned i = 0; i < 100; i++)
        edu_frame_byte(&p, 1, 0, &out);
    CHECK(edu_frame_byte(&p, 0x7e, 0, &out) == -1);
    return 0;
}

static int router(void)
{
    struct edu_datagram d = {17, 51, 4, 2, {0xde, 0xad}}, r;
    unsigned port;
    uint8_t next;
    CHECK(!edu_route(&d, 0, &r, &port, &next));
    CHECK(r.hop == 3 && port == 1 && next == 2 && r.source == 17 &&
          r.payload[1] == 0xad);
    d.payload[1] = 0;
    CHECK(r.payload[1] == 0xad);
    d.hop = 1;
    CHECK(edu_route(&d, 0, &r, &port, &next));
    d.hop = 4;
    d.destination = 17;
    CHECK(edu_route(&d, 0, &r, &port, &next));
    return 0;
}

static int transport(void)
{
    unsigned char bytes[55];
    for (unsigned i = 0; i < 55; i++)
        bytes[i] = (unsigned char)i;
    struct edu_tx t = {0};
    struct edu_rx r = {0};
    struct edu_fragment f, again, ack;
    CHECK(!edu_tx_begin(&t, 1, 7, 8, bytes, sizeof bytes, 0));
    CHECK(edu_tx_poll(&t, 0, &f) == 1);
    CHECK(edu_rx_accept(&r, &f, &ack) == 0);
    CHECK(edu_tx_poll(&t, 99, &again) == 0);
    CHECK(edu_tx_poll(&t, 100, &again) == 1);
    CHECK(f.length == again.length && !memcmp(f.data, again.data, f.length));
    CHECK(edu_rx_accept(&r, &again, &ack) == 0);
    CHECK(r.length == 24);
    CHECK(edu_tx_ack(&t, &ack, 100) == 0);
    CHECK(edu_tx_poll(&t, 100, &f) == 1);
    CHECK(f.index == 1);
    CHECK(edu_rx_accept(&r, &f, &ack) == 0);
    ack.credit = 0;
    CHECK(!edu_tx_ack(&t, &ack, 100));
    CHECK(edu_tx_poll(&t, 200, &f) == 1 && f.type == 3);
    CHECK(!edu_rx_accept(&r, &f, &ack));
    CHECK(!edu_tx_ack(&t, &ack, 200));
    CHECK(edu_tx_poll(&t, 200, &f) == 1 && f.index == 2 && f.type == 1);
    CHECK(edu_rx_accept(&r, &f, &ack) == 1);
    CHECK(ack.credit == 0);
    CHECK(r.ready && r.publications == 1 && !memcmp(r.bytes, bytes, 55));
    CHECK(edu_rx_accept(&r, &f, &ack) == 0 && r.publications == 1);
    CHECK(ack.credit == 0);
    CHECK(edu_tx_ack(&t, &ack, 200) == 1 && !t.active);
    f.data[0] ^= 1;
    CHECK(edu_rx_accept(&r, &f, &ack) == -1);
    f.data[0] ^= 1;
    edu_rx_release(&r);
    CHECK(edu_rx_accept(&r, &f, &ack) == 0 && r.publications == 1);
    CHECK(ack.credit == 1);
    memset(&t, 0, sizeof t);
    CHECK(!edu_tx_begin(&t, 2, 7, 8, bytes, 55, 0));
    for (unsigned i = 0; i < 4; i++)
        CHECK(edu_tx_poll(&t, i * 100, &f) == 1);
    CHECK(edu_tx_poll(&t, 400, &f) == -1);
    memset(&t, 0, sizeof t);
    CHECK(!edu_tx_begin(&t, 3, 7, 8, bytes, 55, 0));
    CHECK(edu_tx_poll(&t, 0, &f) == 1);
    ack = (struct edu_fragment){2, 8, 7, 0, 3, 0, 3, {0}, 0};
    CHECK(!edu_tx_ack(&t, &ack, 0));
    CHECK(edu_tx_poll(&t, 500, &f) == -1);
    return 0;
}

static int presentation(void)
{
    struct edu_record r = {2, 42, -1234, 1, 1, {0}, 0}, decoded, reply;
    uint8_t p[80];
    size_t n;
    CHECK(!edu_record_encode(&r, p, &n));
    CHECK(!edu_record_decode(p, n, &decoded) && decoded.value == -1234);
    for (size_t i = 0; i < n; i++)
        CHECK(edu_record_decode(p, i, &decoded));
    int16_t set = 2200;
    CHECK(!edu_application(&r, &set, &reply) && set == -1234 &&
          reply.value == -1234);
    r.value = 13000;
    CHECK(!edu_application(&r, &set, &reply) && reply.verb == 4 &&
          set == -1234 && reply.value == set);
    r.verb = 1;
    r.has_value = r.has_unit = 0;
    CHECK(!edu_application(&r, &set, &reply) && reply.value == 1800);
    r.label_length = 2;
    r.label[0] = 0xc0;
    r.label[1] = 0x80;
    CHECK(edu_record_encode(&r, p, &n));
    return 0;
}

static int session(void)
{
    struct edu_session s = {0};
    const uint8_t *d;
    size_t n;
    uint8_t open[] = {1, 0, 1, 0, 1, 0, 0, 1, 1, 17};
    CHECK(!edu_session_receive(&s, 17, open, sizeof open, &d, &n));
    uint8_t data[] = {3, 0, 1, 0, 1, 0, 1, 1, 2, 3};
    CHECK(edu_session_receive(&s, 51, data, sizeof data, &d, &n) == -1);
    CHECK(edu_session_receive(&s, 17, data, sizeof data, &d, &n) == 1);
    CHECK(edu_session_receive(&s, 17, data, sizeof data, &d, &n) == 2 &&
          s.effects == 1);
    data[9] = 4;
    CHECK(edu_session_receive(&s, 17, data, sizeof data, &d, &n) == -1);
    CHECK(!edu_session_token_begin(&s, 17));
    data[6] = 2;
    CHECK(edu_session_receive(&s, 17, data, sizeof data, &d, &n) == -1);
    edu_session_token_complete(&s, 51, 0);
    CHECK(s.token_pending);
    edu_session_token_complete(&s, 51, 1);
    CHECK(s.holder == 51 && !s.token_pending);
    uint8_t resume[] = {4, 0, 1, 0, 1, 0, 1};
    CHECK(!edu_session_receive(&s, 17, resume, 7, &d, &n));
    memset(&s, 0, sizeof s);
    CHECK(edu_session_receive(&s, 17, resume, 7, &d, &n) == -1);
    return 0;
}

static int physical(void)
{
    struct edu_physical p = {0};
    CHECK(!edu_phy_begin(&p, 0) && p.driving);
    CHECK(edu_phy_started(&p, 199));
    CHECK(!edu_phy_started(&p, 200));
    CHECK(edu_phy_complete(&p, 1000, 0) && p.driving);
    CHECK(!edu_phy_complete(&p, 1000, 1) && !p.driving);
    CHECK(!edu_phy_ready(&p, 1199));
    CHECK(edu_phy_ready(&p, 1200));
    return 0;
}

/* A real byte pipeline crosses two independent simulated link segments.
   The router receives a decoded L3 value and never gets an L5/L6 API. */
static int routed_fragment(const struct edu_fragment *f,
                           struct edu_fragment *received)
{
    uint8_t transport_bytes[32], network_bytes[40], wire[96];
    size_t tn, nn, wn;
    CHECK(!edu_fragment_encode(f, transport_bytes, &tn));
    struct edu_datagram packet = {17, 51, 4, (uint8_t)tn, {0}};
    memcpy(packet.payload, transport_bytes, tn);
    CHECK(!edu_network_encode(&packet, network_bytes, &nn));
    struct edu_frame link = {0, 1, 2, (uint8_t)nn, {0}};
    memcpy(link.payload, network_bytes, nn);
    CHECK(!edu_frame_encode(&link, wire, sizeof wire, &wn));
    struct edu_framer left = {0};
    struct edu_frame decoded;
    int done = 0;
    for (size_t i = 0; i < wn; i++)
        if (edu_frame_byte(&left, wire[i], 0, &decoded) == 1)
            done++;
    CHECK(done == 1);
    struct edu_datagram in, out;
    CHECK(!edu_network_decode(decoded.payload, decoded.length, &in));
    unsigned segment;
    uint8_t next;
    CHECK(!edu_route(&in, 0, &out, &segment, &next));
    CHECK(segment == 1 && next == 2);
    CHECK(!edu_network_encode(&out, network_bytes, &nn));
    link = (struct edu_frame){0, 1, 2, (uint8_t)nn, {0}};
    memcpy(link.payload, network_bytes, nn);
    CHECK(!edu_frame_encode(&link, wire, sizeof wire, &wn));
    struct edu_framer right = {0};
    done = 0;
    for (size_t i = 0; i < wn; i++)
        if (edu_frame_byte(&right, wire[i], 0, &decoded) == 1)
            done++;
    CHECK(done == 1);
    CHECK(!edu_network_decode(decoded.payload, decoded.length, &in));
    CHECK(in.source == 17 && in.destination == 51 && in.hop == 3);
    CHECK(!edu_fragment_decode(in.payload, in.length, received));
    return 0;
}

static int pipeline(void)
{
    struct edu_record request = {2, 88, 2350, 1, 1, {0}, 16};
    memset(request.label, 'x', 16);
    uint8_t sdu[87] = {3, 0, 1, 0, 1, 0, 1};
    size_t n;
    CHECK(!edu_record_encode(&request, sdu + 7, &n));
    CHECK(n == 33);
    struct edu_tx tx = {0};
    struct edu_rx rx = {0};
    CHECK(!edu_tx_begin(&tx, 1, 7, 8, sdu, n + 7, 0));
    struct edu_fragment f, received, ack;
    uint64_t now = 0;
    while (tx.active)
    {
        CHECK(edu_tx_poll(&tx, now, &f) == 1);
        CHECK(!routed_fragment(&f, &received));
        int r = edu_rx_accept(&rx, &received, &ack);
        CHECK(r >= 0);
        CHECK(edu_tx_ack(&tx, &ack, now) >= 0);
        now++;
    }
    CHECK(rx.ready && rx.length == 40);
    struct edu_session session = {0};
    uint8_t open[] = {1, 0, 1, 0, 1, 0, 0, 1, 1, 17};
    const uint8_t *value;
    size_t size;
    CHECK(!edu_session_receive(&session, 17, open, sizeof open, &value, &size));
    CHECK(edu_session_receive(&session, 17, rx.bytes, rx.length, &value,
                              &size) == 1);
    struct edu_record decoded, reply;
    CHECK(!edu_record_decode(value, size, &decoded));
    int16_t setpoint = 2200;
    CHECK(!edu_application(&decoded, &setpoint, &reply) && setpoint == 2350 &&
          reply.request == 88);
    CHECK(edu_session_receive(&session, 17, rx.bytes, rx.length, &value,
                              &size) == 2 &&
          session.effects == 1);
    return 0;
}

int main(int argc, char **argv)
{
    if (argc != 2)
        return 2;
    int r = !strcmp(argv[1], "link")             ? link_test()
            : !strcmp(argv[1], "router")         ? router()
            : !strcmp(argv[1], "transport")      ? transport()
            : !strcmp(argv[1], "presentation")   ? presentation()
            : !strcmp(argv[1], "session")        ? session()
            : !strcmp(argv[1], "physical_model") ? physical()
            : !strcmp(argv[1], "pipeline")       ? pipeline()
                                                 : 2;
    if (!r)
        printf("PASS edu7_%s\n", argv[1]);
    return r;
}
