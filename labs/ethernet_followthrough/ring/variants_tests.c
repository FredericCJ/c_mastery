#include "variants.h"
void en_note(const char *);
#define CHECK(x)                               \
    do                                         \
    {                                          \
        if (!(x))                              \
        {                                      \
            en_note("EN_VARIANT_ASSERT: " #x); \
            return 41;                         \
        }                                      \
    } while (0)

int en_order(void)
{
    unsigned char good[] = {0, 1, 2, 3, 4, 5, 6, 7},
                  bad[] = {0, 2, 3, 1, 7, 4, 5, 6};
    uint32_t mask = 99;
    CHECK(ev_order(good, &mask) == ED_OK && mask == 0 &&
          ev_order(bad, &mask) == ED_INVALID && mask == 98);
    bad[0] = 2;
    CHECK(ev_order(bad, &mask) == ED_INVALID && mask == 98);
    CHECK(ed_reachable(0x20000000, 1536) == ED_OK &&
          ed_reachable(0x20000000, 1537) == ED_INVALID);
    en_note("OBS ring_order 0 98 7 8");
    return 0;
}

int en_chain(void)
{
    unsigned char bytes[1536] = {0};
    struct ed_rx rx = {0};
    struct ed_borrow out = {0};
    struct ed_descriptor pieces[2] = {{bytes, 32, 0x200}, {bytes + 32, 32, 0}};
    CHECK(ed_rx_acquire(&rx, pieces, 2, &out) == ED_BUSY && rx.dropping &&
          !out.first);
    struct ed_descriptor late = {bytes + 32, 64, (64u << 16) | 0x100u};
    CHECK(ed_rx_acquire(&rx, &late, 1, &out) == ED_IO && !out.first);
    unsigned tested = 0;
    for (unsigned first = 4; first <= 96; first += 4)
    {
        struct ed_rx fresh = {0};
        rx = fresh;
        pieces[0] = (struct ed_descriptor){bytes, (uint16_t)first,
                                           0x200u | 0x3fff0000u};
        pieces[1] = (struct ed_descriptor){
            bytes + first, (uint16_t)(100 - first), (100u << 16) | 0x100u};
        CHECK(ed_rx_acquire(&rx, pieces, 2, &out) == ED_OK &&
              out.first_length + out.second_length == 96 &&
              out.first_length == (first < 96 ? first : 96));
        CHECK(ed_rx_release(&rx, out.token) == ED_OK &&
              ed_rx_release(&rx, out.token) == ED_STALE);
        tested++;
    }
    struct ed_rx fresh = {0};
    rx = fresh;
    struct ed_borrow sentinel = {bytes, 0, 9, 8, 77};
    out = sentinel;
    pieces[0] = (struct ed_descriptor){bytes, 32, 0x200};
    pieces[1] = (struct ed_descriptor){bytes + 32, 32, (100u << 16) | 0x100};
    CHECK(ed_rx_acquire(&rx, pieces, 2, &out) == ED_IO && out.token == 77 &&
          out.first_length == 9);
    pieces[1].status |= 0x80000000u;
    CHECK(ed_rx_acquire(&rx, pieces, 2, &out) == ED_BUSY && out.token == 77);
    bytes[12] = 8;
    size_t length = 99;
    CHECK(ev_span(bytes, 64, 0, &length) == ED_OK && length == 60 &&
          ev_span(bytes, 60, 1, &length) == ED_OK && length == 60);
    bytes[12] = 0;
    bytes[13] = 46;
    CHECK(ev_span(bytes, 64, 1, &length) == ED_OK && length == 60);
    bytes[12] = 6;
    bytes[13] = 0;
    CHECK(ev_span(bytes, 64, 1, &length) == ED_OK && length == 60);
    bytes[13] = 1;
    CHECK(ev_span(bytes, 64, 1, &length) == ED_OK && length == 64);
    CHECK(ev_span(bytes, 1518, 1, &length) == ED_INVALID && length == 64);
    CHECK(tested == 24);
    en_note("OBS ring_chain 24 96 60 64");
    return 0;
}

static void count_barrier(void *p)
{
    unsigned *n = p;
    (*n)++;
}

int en_publication(void)
{
    unsigned char bytes[64] = {9};
    unsigned histories = 0;
    struct ed_tx retained = {0};
    unsigned barriers = 0;
    CHECK(ed_tx_prepare(&retained, 0, 1, bytes, 64) == ED_OK &&
          ed_tx_publish(&retained, 0, 1, count_barrier, &barriers) == ED_BUSY &&
          !barriers);
    CHECK(ed_tx_tail(&retained, 0, 1) == ED_OK &&
          ed_tx_publish(&retained, 0, 1, count_barrier, &barriers) == ED_OK &&
          barriers == 1);
    CHECK(ed_tx_prepare(&retained, 1, 2, bytes, 64) == ED_OK &&
          ed_tx_tail(&retained, 1, 2) == ED_OK &&
          ed_tx_publish(&retained, 1, 2, count_barrier, &barriers) == ED_BUSY);
    CHECK(ed_tx_complete(&retained, 0, 1, 0, 0) == ED_BUSY &&
          ed_tx_complete(&retained, 0, 1, 1, 1) == ED_IO);
    CHECK(ed_tx_publish(&retained, 1, 2, count_barrier, &barriers) == ED_OK &&
          ed_tx_abort(&retained, 1, 2, 0) == ED_BUSY &&
          retained.slots[1].phase == 4 &&
          ed_tx_abort(&retained, 1, 2, 1) == ED_OK);
    for (unsigned n = 2; n <= 3; n++)
    {
        struct ev_chain s = {0};
        CHECK(ev_prepare(&s, bytes, 64, n, 1) == ED_OK);
        CHECK(ev_own(&s, 0) == ED_BUSY && s.owned == 0 && s.borrow == bytes);
        for (unsigned i = n - 1; i > 0; i--)
        {
            CHECK(ev_own(&s, i) == ED_OK);
            if (i > 1)
                CHECK(ev_own(&s, 0) == ED_BUSY);
            histories++;
        }
        CHECK(ev_own(&s, 0) == ED_OK && ev_complete(&s, 0, 0) == ED_BUSY &&
              s.borrow == bytes);
        CHECK(ev_complete(&s, 1, 1) == ED_IO && !s.borrow &&
              ev_prepare(&s, bytes, 64, n, 1) == ED_STALE);
        CHECK(ev_prepare(&s, bytes, 64, n, 2) == ED_OK &&
              ev_own(&s, n - 1) == ED_OK && ev_abort(&s, 0) == ED_BUSY &&
              s.borrow == bytes);
        CHECK(ev_own(&s, 0) == ED_BUSY &&
              ev_prepare(&s, bytes, 64, n, 3) == ED_BUSY &&
              ev_complete(&s, 1, 0) == ED_STALE);
        CHECK(ev_abort(&s, 1) == ED_OK && !s.borrow &&
              ev_prepare(&s, bytes, 64, n, 3) == ED_OK);
    }
    CHECK(histories == 3);
    en_note("OBS ring_publication 2 3 1 0");
    return 0;
}

int en_visibility(void)
{
    struct ev_visibility s = {0};
    CHECK(ev_store(&s, 0, 7) == ED_OK && s.device[0] == 0 &&
          ev_publish(&s, 0) == ED_BUSY);
    CHECK(ev_clean(&s, 0) == ED_OK && s.device[0] == 7 &&
          ev_publish(&s, 0) == ED_OK);
    CHECK(ev_store(&s, 31, 3) == ED_BUSY && ev_store(&s, 32, 5) == ED_OK &&
          ev_clean(&s, 0) == ED_BUSY);
    CHECK(ev_device_store(&s, 0, 9) == ED_OK && s.cpu[0] == 7 &&
          ev_invalidate(&s, 0) == ED_BUSY && ev_return(&s, 0, 0) == ED_BUSY);
    CHECK(ev_return(&s, 0, 1) == ED_OK && ev_invalidate(&s, 0) == ED_OK &&
          s.cpu[0] == 9 && s.cpu[32] == 5);
    /* Harmless counterexample in separate original arrays: cleaning a stale CPU
     * line after device return loses its write. No live DMA or C data race. */
    unsigned char stale = 7, device = 9;
    device = stale;
    CHECK(device == 7);
    en_note("OBS ring_visibility 7 9 5 7");
    return 0;
}
