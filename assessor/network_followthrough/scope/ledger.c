#include "scope.h"
#include <stdio.h>
#define CHECK(x)                                                               \
  do {                                                                         \
    if (!(x)) {                                                                \
      puts("NW_LEDGER " #x);                                                   \
      return 1;                                                                \
    }                                                                          \
  } while (0)
int main(void) {
  const unsigned lengths[] = {3, 256, 257, 600, 1600}, msses[] = {1, 7, 256},
                 windows[] = {1, 17, 512};
  for (unsigned a = 0; a < 5; ++a)
    for (unsigned b = 0; b < 3; ++b)
      for (unsigned c = 0; c < 3; ++c)
        for (unsigned order = 0; order < 2; ++order) {
          NwTx tx[2];
          unsigned char source[1600];
          unsigned segments[2] = {0, 0}, sum[2] = {0, 0};
          for (unsigned owner = 0; owner < 2; ++owner) {
            for (unsigned i = 0; i < lengths[a]; ++i)
              source[i] = (unsigned char)(17 * i + owner);
            CHECK(nw_tx_init(&tx[owner], 9, owner + 40000) == NW_OK);
            CHECK(nw_offer(&tx[owner], source, lengths[a]) == NW_OK);
          }
          for (unsigned i = 0; i < 1600; ++i)
            source[i] = 0;
          while (tx[0].acknowledged < lengths[a] ||
                 tx[1].acknowledged < lengths[a]) {
            for (unsigned owner = 0; owner < 2; ++owner) {
              NwTx *s = &tx[owner];
              if (s->acknowledged == s->length)
                continue;
              NwSegment segment;
              CHECK(nw_prepare(s, msses[b], 0, &segment) == NW_WAIT);
              CHECK(nw_prepare(s, msses[b], windows[c], &segment) == NW_OK);
              uint16_t before = s->sent;
              CHECK(nw_submit(s, segment.ticket, 0) == NW_OK &&
                    s->sent == before);
              CHECK(nw_prepare(s, msses[b], windows[c], &segment) == NW_OK);
              for (unsigned i = 0; i < segment.length; ++i) {
                CHECK(segment.bytes[i] ==
                      (unsigned char)(17 * (segment.offset + i) + owner));
                sum[owner] += segment.bytes[i];
              }
              CHECK(nw_submit(s, segment.ticket, 1) == NW_OK);
              NwTicket wrong = segment.ticket;
              wrong.owner = 40000 + !owner;
              CHECK(nw_ack(s, wrong, s->sent) == NW_STALE);
              if (order == 0)
                CHECK(nw_dma_done(s, segment.ticket) == NW_OK && s->flight);
              if (segment.length > 1)
                CHECK(nw_ack(s, segment.ticket, s->sent - 1) == NW_OK &&
                      s->flight);
              CHECK(nw_ack(s, segment.ticket, s->sent) == NW_OK);
              if (order == 1) {
                CHECK(nw_prepare(s, msses[b], windows[c], &segment) == NW_BUSY);
                CHECK(nw_dma_done(s, segment.ticket) == NW_OK);
              }
              ++segments[owner];
            }
          }
          for (unsigned owner = 0; owner < 2; ++owner) {
            printf("TX %u %u %u %u %u %u %u %u\n", lengths[a], msses[b],
                   windows[c], order, owner, segments[owner], sum[owner],
                   tx[owner].acknowledged);
            CHECK(nw_offer(&tx[owner], source, 1) == NW_OK &&
                  tx[owner].generation == 2);
          }
        }
  return 0;
}
