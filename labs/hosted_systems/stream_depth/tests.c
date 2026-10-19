#include "stream.h"
#include <stdio.h>
#include <string.h>
#define CHECK(c)                                                               \
  do {                                                                         \
    if (!(c)) {                                                                \
      fprintf(stderr, "STREAM-CHECK:%s:%d\n", #c, __LINE__);                   \
      return 1;                                                                \
    }                                                                          \
  } while (0)
/* Complete-buffer decoder: inspect absolute frame extents, not parser state. */
static int decode(const unsigned char *wire, size_t n, unsigned width,
                  RfFrame out[2]) {
  size_t at = 0;
  unsigned count = 0;
  while (at < n) {
    if (count == 2 || n - at < width)
      return -1;
    unsigned len =
        width == 1 ? wire[at] : (unsigned)wire[at] * 256 + wire[at + 1];
    at += width;
    if (len > 8 || n - at < len)
      return -1;
    out[count].length = len;
    for (unsigned i = 0; i < len; ++i)
      out[count].bytes[i] = wire[at + i];
    ++count;
    at += len;
  }
  return (int)count;
}
static int equal(RfFrame a, RfFrame b) {
  return a.length == b.length && !memcmp(a.bytes, b.bytes, a.length);
}
static int framing(unsigned long *histories) {
  *histories = 0;
  for (unsigned width = 1; width <= 2; ++width)
    for (unsigned a = 0; a <= 8; ++a)
      for (unsigned b = 0; b <= 8; ++b) {
        unsigned char wire[20] = {0};
        size_t n = 0;
        unsigned lengths[2] = {a, b};
        for (unsigned f = 0; f < 2; ++f) {
          if (width == 2)
            wire[n++] = 0;
          wire[n++] = (unsigned char)lengths[f];
          for (unsigned i = 0; i < lengths[f]; ++i)
            wire[n++] = (unsigned char)(i % 3 == 0 ? 0 : 255 - i - f);
        }
        RfFrame expected[2] = {{0}};
        CHECK(decode(wire, n, width, expected) == 2);
        unsigned long partitions = 1UL << (n - 1);
        for (unsigned long mask = 0; mask < partitions; ++mask) {
          RfParser parser;
          CHECK(rf_parser_init(&parser, width) == 0);
          size_t first = 0;
          unsigned frames = 0;
          for (size_t end = 1; end <= n; ++end)
            if (end == n || (mask & (1UL << (end - 1)))) {
              size_t cursor = first;
              while (cursor < end) {
                RfFrame out = {.length = 99};
                size_t used = 999;
                int r =
                    rf_feed(&parser, wire + cursor, end - cursor, &used, &out);
                CHECK(r >= 0 && used > 0 && used <= end - cursor);
                cursor += used;
                if (r) {
                  CHECK(frames < 2 && equal(out, expected[frames]));
                  ++frames;
                } else
                  CHECK(out.length == 99);
              }
              first = end;
            }
          CHECK(frames == 2 && rf_eof(&parser) == 0);
          ++*histories;
        }
        /* Every proper prefix is independently classified as
         * boundary/truncation. */
        for (size_t cut = 0; cut <= n; ++cut) {
          RfParser p;
          CHECK(rf_parser_init(&p, width) == 0);
          RfFrame out;
          for (size_t i = 0; i < cut; ++i)
            CHECK(rf_push(&p, wire[i], &out) >= 0);
          int boundary = cut == 0 || cut == width + a || cut == n;
          CHECK((rf_eof(&p) == 0) == boundary);
          if (!boundary)
            CHECK(rf_push(&p, 0, &out) == -1);
        }
      }
  for (unsigned length = 0; length < 65536; ++length) {
    RfParser p;
    RfFrame f = {.length = 99};
    CHECK(rf_parser_init(&p, 2) == 0);
    CHECK(rf_push(&p, (unsigned char)(length >> 8), &f) == 0);
    int r = rf_push(&p, (unsigned char)length, &f);
    CHECK(length == 0 ? r == 1 : length <= 8 ? r == 0 : r == -1);
    if (length > 8)
      CHECK(p.failed == 1 && f.length == 99);
  }
  return 0;
}
static int queues(unsigned *cases) {
  *cases = 0;
  for (unsigned budget = 0; budget <= 256; ++budget)
    for (unsigned length = 0; length <= 8; ++length) {
      RfOutbox q;
      CHECK(rf_outbox_init(&q, budget) == 0);
      RfMessage messages[36];
      unsigned ids[4];
      for (unsigned i = 0; i < 4; ++i)
        CHECK(rf_open(&q, &ids[i]) == 1 && ids[i] == i);
      unsigned untouched = 99;
      CHECK(rf_open(&q, &untouched) == 0 && untouched == 99);
      size_t model_bytes = 0;
      unsigned model_count[4] = {0};
      RfFrame frame = {.length = length};
      for (unsigned j = 0; j < length; ++j)
        frame.bytes[j] = (unsigned char)j;
      for (unsigned i = 0; i < 36; ++i) {
        unsigned slot = i % 4;
        CHECK(rf_message_init(&messages[i], i + 1, 2, &frame) == 0);
        int expected = model_count[slot] < 8 && model_bytes + length <= budget;
        CHECK(rf_enqueue(&q, slot, &messages[i]) == expected);
        if (expected) {
          model_bytes += length;
          ++model_count[slot];
          CHECK(messages[i].owner == RF_OUTBOX);
        } else
          CHECK(messages[i].owner == RF_CALLER);
        CHECK(q.bytes == model_bytes && rf_outbox_invariant(&q));
      }
      for (unsigned slot = 0; slot < 4; ++slot) {
        for (unsigned j = 0; j < model_count[slot]; ++j) {
          size_t before = q.bytes;
          for (unsigned byte = 0; byte < length + 2; ++byte) {
            const unsigned char *wire = 0;
            size_t n = 0;
            CHECK(rf_peek(&q, slot, &wire, &n) == 1 && n == length + 2 - byte);
            CHECK(*wire == (byte == 0 ? 0 : byte == 1 ? length : byte - 2));
            CHECK(rf_advance(&q, slot, 1) == 0);
            CHECK(q.bytes == before - (byte + 1 == length + 2 ? length : 0));
          }
        }
        CHECK(rf_disconnect(&q, slot) == 0);
      }
      CHECK(q.bytes == 0 && rf_outbox_invariant(&q));
      ++*cases;
    }
  RfOutbox q;
  RfMessage message, duplicate;
  RfFrame frame = {.length = 8};
  unsigned slot;
  CHECK(rf_outbox_init(&q, 8) == 0 && rf_open(&q, &slot) == 1 &&
        rf_message_init(&message, 7, 1, &frame) == 0 &&
        rf_enqueue(&q, slot, &message) == 1);
  CHECK(rf_message_init(&duplicate, 7, 1, &frame) == 0 &&
        rf_enqueue(&q, slot, &duplicate) == -1 && duplicate.owner == RF_CALLER);
  CHECK(rf_advance(&q, slot, 1) == 0 && q.bytes == 8 &&
        message.owner == RF_OUTBOX);
  CHECK(rf_disconnect(&q, slot) == 0 && message.owner == RF_CANCELLED &&
        q.bytes == 0 && rf_disconnect(&q, slot) == 0);
  return 0;
}
typedef struct {
  unsigned calls, count, short_returns;
  unsigned char wire[10];
} ShortWriter;
static int short_writer(void *context, const unsigned char *bytes,
                        size_t length) {
  ShortWriter *s = context;
  ++s->calls;
  if (!(s->calls % 2))
    return 0;
  if (!length || s->count == 10)
    return -1;
  s->wire[s->count++] = bytes[0];
  if (length > 1)
    ++s->short_returns;
  return 1;
}
static int short_sends(void) {
  RfOutbox q;
  RfMessage m;
  RfFrame f = {.length = 3, .bytes = {7, 0, 9}};
  unsigned slot;
  CHECK(rf_outbox_init(&q, 3) == 0 && rf_open(&q, &slot) == 1 &&
        rf_message_init(&m, 1, 2, &f) == 0 && rf_enqueue(&q, slot, &m) == 1);
  ShortWriter w = {0};
  for (unsigned i = 0; i < 9; ++i) {
    int result = rf_send_once(&q, slot, short_writer, &w);
    CHECK(result == (i % 2 == 0));
    CHECK(m.offset == (i + 2) / 2 &&
          m.owner == (i == 8 ? RF_DELIVERED : RF_OUTBOX) &&
          q.bytes == (i == 8 ? 0 : 3));
  }
  const unsigned char expected[5] = {0, 3, 7, 0, 9};
  CHECK(w.count == 5 && w.short_returns == 4 && !memcmp(w.wire, expected, 5));
  return 0;
}
static int replay_and_sequences(void) {
  RfDedup d;
  uint32_t result = 99;
  CHECK(rf_dedup_init(&d, 4) == 0);
  CHECK(rf_effect(&d, 4, 7, 3, &result) == 1 && result == 3 && d.total == 3);
  CHECK(rf_effect(&d, 4, 7, 3, &result) == 0 && result == 3 && d.total == 3);
  CHECK(rf_effect(&d, 4, 7, 4, &result) == -1 && result == 3 && d.total == 3);
  for (unsigned id = 8; id <= 10; ++id)
    CHECK(rf_effect(&d, 4, id, 1, &result) == 1);
  CHECK(d.total == 6 && rf_effect(&d, 4, 7, 3, &result) == 0 && result == 3 &&
        d.total == 6);
  CHECK(rf_effect(&d, 4, 11, 1, &result) == -1 && d.count == 4 && d.total == 6);
  CHECK(rf_effect(&d, 5, 7, 3, &result) == -1 && d.total == 6);
  CHECK(rf_dedup_init(&d, 1) == 0);
  d.total = UINT32_MAX;
  CHECK(rf_effect(&d, 1, 1, 1, &result) == -1 && d.total == UINT32_MAX &&
        d.count == 0);
  RfSequence s;
  unsigned out[3], n;
  CHECK(rf_sequence_init(&s, 1, 7) == 0);
  unsigned arrivals[5] = {7, 9, 8, 9, 6};
  int expected[5] = {1, 1, 1, 0, -1};
  unsigned counts[5] = {1, 0, 2, 0, 0}, all[3] = {0}, at = 0;
  for (unsigned i = 0; i < 5; ++i) {
    CHECK(rf_arrive(&s, 1, arrivals[i], out, &n) == expected[i] &&
          n == counts[i]);
    for (unsigned j = 0; j < n; ++j)
      all[at++] = out[j];
  }
  CHECK(at == 3 && all[0] == 7 && all[1] == 8 && all[2] == 9);
  CHECK(rf_sequence_init(&s, 2, 0) == 0 &&
        rf_arrive(&s, 1, 0, out, &n) == RF_SEQ_SESSION && n == 0 &&
        s.next == 0);
  CHECK(rf_arrive(&s, 2, 0, out, &n) == 1 && n == 1 && out[0] == 0);
  CHECK(rf_sequence_init(&s, 1, 7) == 0 && rf_arrive(&s, 1, 9, out, &n) == 1 &&
        n == 0);
  unsigned missing = 99;
  CHECK(rf_expire_gap(&s, &missing, out, &n) == 1 && missing == 7 && n == 0);
  CHECK(rf_expire_gap(&s, &missing, out, &n) == 1 && missing == 8 && n == 1 &&
        out[0] == 9);
  CHECK(rf_sequence_init(&s, 3, 65535) == 0 &&
        rf_arrive(&s, 3, 65535, out, &n) == 1 && n == 1 && s.next == 65536 &&
        rf_arrive(&s, 3, 0, out, &n) == RF_SEQ_RETIRED);
  return 0;
}
int main(void) {
  unsigned long histories;
  unsigned cases;
  CHECK(framing(&histories) == 0 && queues(&cases) == 0 && short_sends() == 0 &&
        replay_and_sequences() == 0);
  printf("STREAM-PASS:all_chunk_partitions=%lu;queue_budget_cells=%u;all_65536_"
         "two_byte_lengths=1;replay_cache_bound=4;sequence_order=7,8,9;\n",
         histories, cases);
  return 0;
}
