#include "handoff.h"
#include <stdio.h>
#define CHECK(x)                                                               \
  do {                                                                         \
    if (!(x)) {                                                                \
      printf("XI_CHECK:%d:%s\n", __LINE__, #x);                                \
      return 1;                                                                \
    }                                                                          \
  } while (0)
static struct xi_record record(unsigned id) {
  struct xi_record r = {
      .sequence = id, .timestamp = 17 * id, .original_length = 8, .copied = 8};
  for (unsigned j = 0; j < 8; ++j)
    r.bytes[j] = (unsigned char)(id + j);
  return r;
}
int main(void) {
  struct xi_queue q;
  struct xi_record out = record(999), r = record(1);
  CHECK(xi_init(&q, 0) == XI_INVALID);
  CHECK(xi_init(&q, 1) == XI_INVALID);
  CHECK(xi_init(&q, 3) == XI_INVALID);
  CHECK(xi_init(&q, 4) == XI_OK);
  CHECK(xi_take(&q, &out) == XI_EMPTY && out.sequence == 999);
  CHECK(xi_reserve(&q) == XI_OK);
  CHECK(xi_take(&q, &out) == XI_EMPTY);
  CHECK(xi_commit(&q) == XI_STATE);
  CHECK(xi_reserve(&q) == XI_BUSY);
  CHECK(xi_fill(&q, &r) == XI_OK);
  CHECK(xi_take(&q, &out) == XI_EMPTY);
  CHECK(xi_commit(&q) == XI_OK);
  for (unsigned i = 2; i <= 3; ++i) {
    r = record(i);
    CHECK(xi_push(&q, &r) == XI_OK);
  }
  CHECK(xi_take(&q, &out) == XI_OK && out.sequence == 1);
  CHECK(xi_take(&q, &out) == XI_BUSY);
  r = record(4);
  CHECK(xi_push(&q, &r) == XI_FULL);
  CHECK(xi_release(&q) == XI_OK);
  CHECK(xi_release(&q) == XI_STATE);
  r = record(5);
  CHECK(xi_push(&q, &r) == XI_OK);
  for (unsigned i = 0; i < 3; ++i) {
    unsigned expect = i < 2 ? i + 2 : 5;
    CHECK(xi_pop(&q, &out) == XI_OK && out.sequence == expect);
  }
  CHECK(xi_pop(&q, &out) == XI_EMPTY);
  CHECK(xi_reserve(&q) == XI_OK);
  CHECK(xi_cancel(&q) == XI_OK);
  CHECK(xi_pop(&q, &out) == XI_EMPTY);
  for (unsigned j = 0; j < 10000; ++j) {
    r = record(j);
    CHECK(xi_push(&q, &r) == XI_OK);
    CHECK(xi_pop(&q, &out) == XI_OK && out.sequence == j &&
          out.bytes[7] == (unsigned char)(j + 7));
  }
  struct xi_service service;
  CHECK(xi_service_init(&service, &q) == XI_OK);
  unsigned char bytes[8] = {1, 2, 3, 4, 5, 6, 7, 8};
  CHECK(xi_capture_begin(&service, 1, 5, bytes, UINT32_MAX) == XI_OK);
  CHECK(xi_capture_begin(&service, 99, 9, bytes, 1) == XI_BUSY &&
        service.acknowledged == 1);
  CHECK(xi_capture_end(&service) == XI_OK);
  for (unsigned i = 2; i <= 5; ++i) {
    int rc = xi_capture_begin(&service, i, i * 5, bytes, 8);
    CHECK(rc == (i <= 3 ? XI_OK : XI_FULL));
    if (!rc)
      CHECK(xi_capture_end(&service) == XI_OK);
  }
  CHECK(service.acknowledged == 5 && service.dropped == 2 &&
        service.reentrant == 1);
  CHECK(xi_pop(&q, &out) == XI_OK && out.sequence == 1 && out.copied == 8 &&
        out.original_length == UINT32_MAX && out.flags == XI_TRUNCATED &&
        out.bytes[7] == 8);
  r = record(7);
  CHECK(xi_push(&q, &r) == XI_OK);
  service.dropped = UINT32_MAX;
  CHECK(xi_capture_begin(&service, 8, 8, bytes, 8) == XI_FULL &&
        service.dropped == UINT32_MAX);
  const uint32_t lengths[] = {0, 1, 8, 9, UINT32_MAX};
  for (unsigned i = 0; i < 5; ++i) {
    CHECK(xi_init(&q, 4) == XI_OK && xi_service_init(&service, &q) == XI_OK);
    CHECK(xi_capture_begin(&service, i, 7, lengths[i] ? bytes : NULL,
                           lengths[i]) == XI_OK);
    CHECK(xi_capture_end(&service) == XI_OK && xi_pop(&q, &out) == XI_OK);
    CHECK(out.original_length == lengths[i] &&
          out.copied == (lengths[i] > 8 ? 8 : lengths[i]) &&
          out.flags == (lengths[i] > 8 ? XI_TRUNCATED : 0));
    for (unsigned j = 0; j < 8; ++j)
      CHECK(out.bytes[j] == (j < out.copied ? bytes[j] : 0));
  }
  CHECK(xi_capture_begin(&service, 9, 7, NULL, 1) == XI_INVALID &&
        service.acknowledged == 1 && !service.active);
  service.acknowledged = service.reentrant = UINT32_MAX;
  CHECK(xi_capture_begin(&service, 9, 7, bytes, 1) == XI_OK &&
        service.acknowledged == UINT32_MAX);
  CHECK(xi_capture_begin(&service, 10, 7, bytes, 1) == XI_BUSY &&
        service.reentrant == UINT32_MAX);
  CHECK(xi_capture_end(&service) == XI_OK);
  struct xi_latest latest;
  xi_latest_init(&latest);
  for (unsigned i = 1; i <= 5; ++i) {
    r = record(i);
    CHECK(xi_latest_push(&latest, &r) == XI_OK);
  }
  CHECK(latest.dropped == 2);
  for (unsigned i = 3; i <= 5; ++i)
    CHECK(xi_latest_pop(&latest, &out) == XI_OK && out.sequence == i);
  struct xi_mask mask;
  uint32_t outer, inner;
  for (unsigned prior = 0; prior < 2; ++prior) {
    CHECK(xi_mask_init(&mask, (struct xf_priority_config){4, 3}, 0, prior) == XI_OK);
    CHECK(xi_mask_enter(&mask, 1, &outer) == XI_OK);
    CHECK(xi_mask_enter(&mask, 1, &inner) == XI_OK);
    CHECK(xi_mask_leave(&mask, outer) == XI_STATE && mask.depth == 2);
    CHECK(xi_mask_leave(&mask, inner) == XI_OK && mask.value == 1);
    CHECK(xi_mask_leave(&mask, outer) == XI_OK && mask.value == prior);
    CHECK(xi_mask_leave(&mask, outer) == XI_STATE);
  }
  CHECK(xi_mask_init(&mask, (struct xf_priority_config){4, 3}, 1, 0x40) == XI_OK);
  CHECK(xi_mask_enter(&mask, 0x80, &outer) == XI_OK && mask.value == 0x40);
  CHECK(xi_mask_enter(&mask, 0x20, &inner) == XI_OK && mask.value == 0x20);
  CHECK(xi_mask_leave(&mask, inner) == XI_OK && mask.value == 0x40);
  CHECK(xi_mask_leave(&mask, outer) == XI_OK && mask.value == 0x40);
  CHECK(xi_mask_enter(&mask, 0, &outer) == XI_OK && mask.value == 0x40);
  CHECK(xi_mask_leave(&mask, outer) == XI_OK);
  mask.next = UINT32_MAX;
  CHECK(xi_mask_enter(&mask, 0x20, &outer) == XI_EXHAUSTED);
  CHECK(xi_mask_enter(&mask, 0x21, &outer) == XI_INVALID);
  CHECK(xi_mask_init(&mask, (struct xf_priority_config){4, 3}, 0, 0) == XI_OK);
  uint32_t tokens[8];
  for (unsigned i = 0; i < 8; ++i)
    CHECK(xi_mask_enter(&mask, 1, &tokens[i]) == XI_OK);
  outer = 999;
  CHECK(xi_mask_enter(&mask, 1, &outer) == XI_FULL && outer == 999 &&
        mask.depth == 8);
  for (unsigned i = 8; i > 0; --i)
    CHECK(xi_mask_leave(&mask, tokens[i - 1]) == XI_OK);
  CHECK(mask.value == 0);
  puts("XI_PASS ordinary_pause_burst_latest_mask");
  return 0;
}
