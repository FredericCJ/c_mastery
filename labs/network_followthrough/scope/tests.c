#include "scope.h"
#include <stdio.h>
#define CHECK(x)                  \
    do                            \
    {                             \
        if (!(x))                 \
        {                         \
            puts("NW_CHECK " #x); \
            return 1;             \
        }                         \
    } while (0)

int main(void)
{
    struct nw_lengths length;
    CHECK(nw_lengths(3, &length) == NW_OK && length.ip == 43 &&
          length.unpadded == 57 && length.padding == 3 &&
          length.pre_fcs == 60 && length.with_fcs == 64 &&
          length.dma_bytes == 12384);
    CHECK(nw_lengths(1461, &length) == NW_INVALID);
    unsigned char data[600];
    for (unsigned i = 0; i < sizeof data; ++i)
        data[i] = (unsigned char)i;
    struct nw_tx a, b;
    CHECK(nw_tx_init(&a, 7, 41) == NW_OK && nw_tx_init(&b, 7, 42) == NW_OK);
    CHECK(nw_offer(&a, data, sizeof data) == NW_OK &&
          nw_offer(&b, data, sizeof data) == NW_OK);
    data[0] = 255;
    struct nw_segment segment, other;
    CHECK(nw_prepare(&a, 256, 512, &segment) == NW_OK &&
          segment.length == 256 && segment.offset == 0 &&
          segment.bytes[0] == 0 && segment.bytes[255] == 255);
    CHECK(nw_submit(&a, segment.ticket, 0) == NW_OK && a.sent == 0 && !a.dma);
    CHECK(nw_prepare(&a, 256, 64, &segment) == NW_OK && segment.length == 64);
    CHECK(nw_submit(&a, segment.ticket, 1) == NW_OK && a.sent == 64 && a.dma);
    CHECK(nw_dma_done(&a, segment.ticket) == NW_OK && a.acknowledged == 0 &&
          a.flight);
    CHECK(nw_prepare(&a, 256, 512, &other) == NW_BUSY);
    CHECK(nw_ack(&a, segment.ticket, 32) == NW_OK && a.flight);
    CHECK(nw_ack(&a, segment.ticket, 64) == NW_OK && !a.flight);
    CHECK(nw_ack(&a, segment.ticket, 65) == NW_RANGE);
    struct nw_ticket old = segment.ticket;
    CHECK(nw_prepare(&a, 256, 512, &segment) == NW_OK && segment.offset == 64);
    CHECK(nw_submit(&a, segment.ticket, 1) == NW_OK);
    CHECK(nw_dma_done(&a, old) == NW_STALE && a.dma);
    CHECK(nw_ack(&a, segment.ticket, 320) == NW_OK && a.dma);
    CHECK(nw_offer(&a, data, 1) == NW_BUSY);
    CHECK(nw_dma_done(&a, segment.ticket) == NW_OK);
    CHECK(nw_prepare(&b, 256, 512, &other) == NW_OK);
    struct nw_ticket wrong = other.ticket;
    wrong.owner = a.owner;
    CHECK(nw_submit(&b, wrong, 1) == NW_STALE && b.prepared);
    CHECK(nw_submit(&b, other.ticket, 0) == NW_OK);
    b.serial = UINT32_MAX;
    CHECK(nw_prepare(&b, 256, 512, &other) == NW_RANGE);
    struct nw_ticket invalid = other.ticket;
    invalid.epoch++;
    CHECK(nw_ack(&b, invalid, 1) == NW_STALE);
    invalid = other.ticket;
    invalid.generation++;
    CHECK(nw_ack(&b, invalid, 1) == NW_STALE);
    b.generation = UINT32_MAX;
    b.acknowledged = b.length;
    CHECK(nw_offer(&b, data, 1) == NW_RANGE);
    CHECK(nw_offer(&b, data, 0) == NW_INVALID);
    CHECK(nw_ack(&a, segment.ticket, 320) == NW_DUPLICATE);
    CHECK(nw_dma_done(&a, segment.ticket) == NW_DUPLICATE);
    CHECK(nw_prepare(&a, 0, 1, &other) == NW_INVALID);
    CHECK(nw_prepare(&a, 1, 65536, &other) == NW_INVALID);
    struct nw_rx q;
    CHECK(nw_rx_init(&q, 8) == NW_OK);
    unsigned char bytes[6] = {0, 1, 2, 3, 4, 5};
    CHECK(nw_rx_offer(&q, bytes, 6) == NW_OK && q.count == 6 &&
          nw_rx_window(&q) == 2);
    bytes[0] = 99;
    const unsigned char *view;
    size_t n;
    CHECK(nw_rx_peek(&q, &view, &n) == NW_OK && n == 6 && view[0] == 0);
    CHECK(nw_rx_offer(&q, bytes, 3) == NW_BUSY && q.count == 6);
    CHECK(nw_rx_consume(&q, 5) == NW_OK && nw_rx_offer(&q, bytes, 6) == NW_OK);
    CHECK(nw_rx_peek(&q, &view, &n) == NW_OK && n == 3 && view[0] == 5 &&
          view[1] == 99);
    CHECK(nw_rx_consume(&q, 8) == NW_RANGE && q.count == 7);
    CHECK(nw_rx_consume(&q, 7) == NW_OK && nw_rx_window(&q) == 8);
    puts("NW_PASS length_ownership_admission_stream_capacity");
    return 0;
}
