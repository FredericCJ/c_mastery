#include "allocation.h"
#include <stdio.h>
#define CHECK(x)                                                               \
  do {                                                                         \
    if (!(x)) {                                                                \
      fprintf(stderr, "ASSERT buddy line %d: %s\n", __LINE__, #x);             \
      return 1;                                                                \
    }                                                                          \
  } while (0)
int main(void) {
  struct al_buddy b, other;
  struct al_handle a, c, h[8];
  size_t off, charged;
  CHECK(al_buddy_init(&b) == AL_OK);
  CHECK(al_buddy_init(&other) == AL_OK);
  CHECK(al_buddy_take(&b, 9, &a, &off, &charged) == AL_OK && off == 0 &&
        charged == 16);
  CHECK(al_buddy_take(&b, 8, &c, &off, &charged) == AL_OK && off == 16 &&
        charged == 8);
  CHECK(al_buddy_release(&b, a) == AL_OK && al_buddy_check(&b) == AL_OK);
  CHECK(al_buddy_release(&other, c) == AL_STALE);
  CHECK(al_buddy_release(&b, c) == AL_OK && !b.state[0]);
  unsigned histories = 0;
  for (unsigned mask = 0; mask < 256; ++mask) {
    CHECK(al_buddy_init(&b) == AL_OK);
    for (unsigned i = 0; i < 8; ++i) {
      CHECK(al_buddy_take(&b, 8, &h[i], &off, &charged) == AL_OK &&
            off == i * 8 && charged == 8 && b.steps <= 15);
    }
    for (unsigned i = 0; i < 8; ++i)
      if (mask & (1u << i)) {
        CHECK(al_buddy_release(&b, h[i]) == AL_OK);
        CHECK(al_buddy_check(&b) == AL_OK && b.steps <= 4);
      }
    for (unsigned i = 0; i < 8; ++i)
      if (!(mask & (1u << i))) {
        CHECK(al_buddy_release(&b, h[i]) == AL_OK);
        CHECK(al_buddy_check(&b) == AL_OK);
      }
    CHECK(!b.state[0]);
    ++histories;
  }
  for (unsigned request = 1; request <= 64; ++request) {
    CHECK(al_buddy_init(&b) == AL_OK);
    CHECK(al_buddy_take(&b, request, &a, &off, &charged) == AL_OK &&
          charged >= request && charged <= 64 &&
          (charged & (charged - 1)) == 0);
    CHECK(al_buddy_release(&b, a) == AL_OK);
  }
  printf(
      "PASS buddy histories=%u relative-offset/coalescence/operation-bounds\n",
      histories);
  return 0;
}
