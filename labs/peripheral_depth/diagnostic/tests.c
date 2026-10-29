#include "ethernet.h"
#if defined(DG_BIG_ENDIAN) || defined(DG_STANDALONE)
int pm_printf(const char *, ...);
int pm_puts(const char *);
#define printf pm_printf
#define puts pm_puts
#else
#include <stdio.h>
#endif
#define CHECK(x)                               \
    do                                         \
    {                                          \
        if (!(x))                              \
        {                                      \
            printf("DG_CHECK:%d\n", __LINE__); \
            return 1;                          \
        }                                      \
    } while (0)

static int same(const uint8_t *a, const uint8_t *b, size_t n)
{
    for (size_t i = 0; i < n; ++i)
    {
        if (a[i] != b[i])
        {
            return 0;
        }
    }
    return 1;
}

static void barrier(void *v)
{
    ++*(unsigned *)v;
}

int main(void)
{
    struct dg_message m = {1, 2, {1, 2}}, out = {99, 0, {0}};
    uint8_t wire[80];
    size_t n = 99;
    const uint8_t golden[] = {0, 1, 0, 2, 1, 2};
    CHECK(dg_encode(&m, wire, 5, &n) == DG_FULL && n == 99);
    CHECK(dg_encode(&m, wire, sizeof wire, &n) == DG_OK && n == 6 &&
          same(wire, golden, 6));
    for (size_t i = 0; i < 6; ++i)
    {
        CHECK(dg_decode(wire, i, &out) == DG_FRAME && out.sequence == 99);
    }
    CHECK(dg_decode(wire, 7, &out) == DG_FRAME && out.sequence == 99);
    CHECK(dg_decode(wire, 6, &out) == DG_OK && out.sequence == 1 &&
          out.length == 2);
    struct dg_session session = {0};
    CHECK(dg_accept(&session, wire, n, &out) == DG_OK);
    CHECK(dg_accept(&session, wire, n, &out) == DG_STALE &&
          session.accepted == 1);
    m.sequence = 65535;
    CHECK(dg_encode(&m, wire, sizeof wire, &n) == DG_OK);
    CHECK(dg_accept(&session, wire, n, &out) == DG_OK && session.accepted == 2);
    m.sequence = 1;
    CHECK(dg_encode(&m, wire, sizeof wire, &n) == DG_OK);
    CHECK(dg_accept(&session, wire, n, &out) == DG_STALE &&
          session.last == 65535);
    for (unsigned length = 0; length <= 32; ++length)
    {
        m.length = (uint16_t)length;
        for (unsigned i = 0; i < length; ++i)
        {
            m.payload[i] = (uint8_t)(i * 17);
        }
        CHECK(dg_encode(&m, wire + 2, 78, &n) == DG_OK);
        wire[0] = 0;
        wire[1] = (uint8_t)n;
        for (size_t cut = 0; cut <= n + 2; ++cut)
        {
            struct dg_stream stream = {0};
            struct dg_session s = {0};
            size_t used;
            CHECK(dg_stream_feed(&stream, wire, cut, &used) == DG_OK &&
                  used == cut);
            if (cut < n + 2)
            {
                CHECK(dg_stream_take(&stream, &s, &out) == DG_BUSY);
            }
            CHECK(dg_stream_feed(&stream, wire + cut, n + 2 - cut, &used) ==
                  (cut == n + 2 ? DG_BUSY : DG_OK));
            CHECK(dg_stream_take(&stream, &s, &out) == DG_OK &&
                  out.length == length && same(out.payload, m.payload, length));
            CHECK(dg_stream_end(&stream) == DG_END);
        }
    }
    struct dg_stream stream = {0};
    size_t used;
    const uint8_t bad[] = {0, 37, 0, 1};
    CHECK(dg_stream_feed(&stream, bad, 4, &used) == DG_FRAME && used == 2);
    CHECK(dg_stream_feed(&stream, wire, 6, &used) == DG_FRAME && used == 0);
    CHECK(dg_stream_end(&stream) == DG_FRAME);
    stream = (struct dg_stream){0};
    CHECK(dg_stream_feed(&stream, wire, 1, &used) == DG_OK);
    CHECK(dg_stream_end(&stream) == DG_FRAME);
    /* Two valid outer frames can arrive together. Reject a replay without losing
       the following frame; never consume that suffix while a frame is ready. */
    const uint8_t coalesced[] = {0, 4, 0, 1, 0, 0, 0, 4, 0, 2, 0, 0};
    stream = (struct dg_stream){0};
    session = (struct dg_session){0};
    CHECK(dg_stream_feed(&stream, coalesced, sizeof coalesced, &used) ==
              DG_OK &&
          used == 6);
    CHECK(dg_stream_end(&stream) == DG_BUSY);
    CHECK(dg_stream_feed(&stream, coalesced + 6, 6, &used) == DG_BUSY &&
          used == 0);
    CHECK(dg_stream_take(&stream, &session, &out) == DG_OK &&
          out.sequence == 1);
    CHECK(dg_stream_feed(&stream, coalesced, sizeof coalesced, &used) ==
              DG_OK &&
          used == 6);
    CHECK(dg_stream_take(&stream, &session, &out) == DG_STALE &&
          session.accepted == 1);
    CHECK(dg_stream_feed(&stream, coalesced + 6, 6, &used) == DG_OK &&
          used == 6);
    CHECK(dg_stream_take(&stream, &session, &out) == DG_OK &&
          out.sequence == 2 && session.accepted == 2);
    CHECK(dg_stream_end(&stream) == DG_END);
    m = (struct dg_message){7, 2, {1, 2}};
    CHECK(dg_ethernet_build(&m, wire, sizeof wire, &n) == DG_OK && n == 60);
    session = (struct dg_session){0};
    CHECK(dg_ethernet_accept(&session, wire, n, &out) == DG_OK &&
          out.sequence == 7);
    for (size_t split = 1; split < 64; ++split)
    {
        uint8_t fcs[4] = {
            0}; /* DMA status reports retained FCS; hardware already checked it. */
        uint8_t complete[64];
        for (size_t i = 0; i < 60; ++i)
        {
            complete[i] = wire[i];
        }
        for (size_t i = 0; i < 4; ++i)
        {
            complete[60 + i] = fcs[i];
        }
        struct ed_descriptor d[2] = {{complete, (uint16_t)split, 1u << 9},
                                     {complete + split, (uint16_t)(64 - split),
                                      (64u << 16) | (1u << 8)}};
        struct ed_rx rx = {0};
        session = (struct dg_session){0};
        CHECK(dg_descriptors(&session, &rx, d, 2, &out) == DG_OK &&
              out.sequence == 7 && !rx.held);
        CHECK(dg_descriptors(&session, &rx, d, 2, &out) == DG_STALE &&
              !rx.held && rx.generation == 2);
        d[0].status |= 1u << 31;
        CHECK(dg_descriptors(&session, &rx, d, 2, &out) == DG_BUSY && !rx.held);
    }
    struct ed_tx tx = {0};
    unsigned barriers = 0;
    CHECK(dg_publish(&tx, 0, 1, &m, barrier, &barriers) == DG_OK &&
          barriers == 1 && tx.slots[0].phase == 3);
    CHECK(ed_tx_abort(&tx, 0, 1, 0) == ED_BUSY && tx.slots[0].phase == 4);
    CHECK(dg_publish(&tx, 1, 1, &m, barrier, &barriers) == DG_BUSY &&
          tx.slots[1].phase == 2 && barriers == 1);
    CHECK(ed_tx_abort(&tx, 0, 1, 1) == ED_OK);
    CHECK(ed_tx_publish(&tx, 1, 1, barrier, &barriers) == ED_OK &&
          barriers == 2);
    CHECK(ed_tx_complete(&tx, 1, 1, 1, 0) == ED_OK);
    puts("DG_PASS");
    return 0;
}
