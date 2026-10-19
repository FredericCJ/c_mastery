#include "arp.h"
#include <stdio.h>
#include <string.h>
#define CHECK(x)                                                               \
  do {                                                                         \
    if (!(x)) {                                                                \
      puts("NA_CHECK " #x);                                                    \
      return 1;                                                                \
    }                                                                          \
  } while (0)
static const NaConfig config = {
    {2, 0, 0, 0, 0, 2}, {192, 0, 2, 2}, {192, 0, 2, 1}};
static void reply(unsigned char p[60], unsigned tail) {
  const unsigned char bytes[42] = {
      2, 0, 0, 0, 0, 2, 2, 0,   0, 0, 0, 1, 8, 6, 0, 1, 8, 0,   6, 4, 0,
      2, 2, 0, 0, 0, 0, 1, 192, 0, 2, 1, 2, 0, 0, 0, 0, 2, 192, 0, 2, 2};
  memset(p, 0, 60);
  memcpy(p, bytes, 42);
  p[11] = p[27] = (unsigned char)tail;
}
int main(void) {
  NaArp s;
  NetPacket frame;
  NaTicket ticket, old;
  unsigned char bytes[60];
  const unsigned char payload[5] = {1, 0, 2, 255, 4};
  CHECK(na_init(&s, config, 9) == NA_OK);
  CHECK(na_queue(&s, payload, 5, 0) == NA_OK &&
        na_queue(&s, payload, 5, 0) == NA_BUSY);
  CHECK(na_prepare(&s, 0, &frame, &ticket) == NA_OK &&
        ticket.kind == NA_REQUEST && frame.length == 60);
  CHECK(frame.bytes[0] == 255 && frame.bytes[13] == 6 && frame.bytes[21] == 1 &&
        frame.bytes[41] == 1);
  CHECK(na_submit(&s, ticket, 0) == NA_OK && s.requests == 0 && s.pending);
  CHECK(na_prepare(&s, 0, &frame, &ticket) == NA_OK &&
        na_submit(&s, ticket, 1) == NA_OK);
  old = ticket;
  CHECK(na_submit(&s, old, 1) == NA_STALE);
  CHECK(na_prepare(&s, 999, &frame, &ticket) == NA_WAIT && s.requests == 1);
  CHECK(na_prepare(&s, 1000, &frame, &ticket) == NA_OK &&
        na_submit(&s, ticket, 1) == NA_OK);
  CHECK(na_prepare(&s, 2000, &frame, &ticket) == NA_OK &&
        na_submit(&s, ticket, 1) == NA_OK);
  CHECK(na_prepare(&s, 3000, &frame, &ticket) == NA_EXPIRED && !s.pending &&
        !s.length && s.requests == 3);
  CHECK(na_init(&s, config, 10) == NA_OK);
  reply(bytes, 7);
  CHECK(na_receive(&s, bytes, 60, 0) == NA_OK && s.peer_mac[5] == 7);
  CHECK(na_queue(&s, payload, 5, 29999) == NA_OK);
  CHECK(na_prepare(&s, 29999, &frame, &ticket) == NA_OK &&
        ticket.kind == NA_DATA && frame.bytes[5] == 7);
  CHECK(!memcmp(frame.bytes + 14, payload, 5));
  CHECK(na_submit(&s, ticket, 0) == NA_OK && s.pending);
  CHECK(na_prepare(&s, 30000, &frame, &ticket) == NA_OK &&
        ticket.kind == NA_REQUEST && !s.known);
  CHECK(na_submit(&s, ticket, 1) == NA_OK);
  reply(bytes, 99);
  CHECK(na_receive(&s, bytes, 60, 30001) == NA_OK && s.peer_mac[5] == 99);
  CHECK(na_prepare(&s, 30001, &frame, &ticket) == NA_OK &&
        ticket.kind == NA_DATA && frame.bytes[5] == 99);
  CHECK(na_receive(&s, bytes, 60, 30002) == NA_BUSY);
  NaTicket wrong = ticket;
  wrong.epoch++;
  CHECK(na_submit(&s, wrong, 1) == NA_STALE && s.pending);
  CHECK(na_submit(&s, ticket, 1) == NA_OK && !s.pending && s.resolved);
  CHECK(na_prepare(&s, 30000, &frame, &ticket) == NA_INVALID);
  CHECK(na_queue(&s, payload, 5, UINT64_MAX - 2999) == NA_RANGE);
  CHECK(na_init(&s, config, 11) == NA_OK);
  CHECK(na_queue(&s, payload, 5, 0) == NA_OK);
  /* Late service sends one request in the current slot, no catch-up burst. */
  CHECK(na_prepare(&s, 2500, &frame, &ticket) == NA_OK &&
        na_submit(&s, ticket, 1) == NA_OK);
  CHECK(na_prepare(&s, 2500, &frame, &ticket) == NA_WAIT && s.requests == 1);
  CHECK(na_init(&s, config, 12) == NA_OK);
  reply(bytes, 37);
  bytes[21] = 1;
  memset(bytes, 255, 6);
  memset(bytes + 32, 0, 6);
  CHECK(na_receive(&s, bytes, 60, 0) == NA_OK && s.reply_pending);
  CHECK(na_queue(&s, payload, 5, 0) == NA_OK);
  CHECK(na_prepare(&s, 0, &frame, &ticket) == NA_OK && ticket.kind == NA_REPLY);
  const unsigned char expected_reply[42] = {
      2, 0, 0, 0, 0, 37, 2, 0,   0, 0, 0, 2, 8, 6, 0, 1, 8,  0,   6, 4, 0,
      2, 2, 0, 0, 0, 0,  2, 192, 0, 2, 2, 2, 0, 0, 0, 0, 37, 192, 0, 2, 1};
  CHECK(!memcmp(frame.bytes, expected_reply, 42) && frame.length == 60);
  for (unsigned i = 42; i < 60; ++i)
    CHECK(frame.bytes[i] == 0);
  CHECK(na_submit(&s, ticket, 0) == NA_OK && s.reply_pending && s.pending);
  CHECK(na_prepare(&s, 0, &frame, &ticket) == NA_OK && ticket.kind == NA_REPLY);
  CHECK(na_submit(&s, ticket, 1) == NA_OK && !s.reply_pending && s.pending);
  CHECK(na_prepare(&s, 2999, &frame, &ticket) == NA_OK &&
        ticket.kind == NA_DATA);
  CHECK(na_submit(&s, ticket, 0) == NA_OK);
  CHECK(na_prepare(&s, 3000, &frame, &ticket) == NA_EXPIRED && !s.pending);
  CHECK(na_queue(&s, payload, 5, 3000) == NA_OK);
  s.serial = UINT32_MAX;
  CHECK(na_prepare(&s, 3000, &frame, &ticket) == NA_RANGE && s.pending &&
        !s.prepared);
  puts("NA_PASS retry_ms learned_mac rejection_retention expiry late_service");
  return 0;
}
