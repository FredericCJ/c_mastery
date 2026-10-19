#include "cmx_advanced.h"
#include "identity.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define CHECK(x)                                                               \
  do {                                                                         \
    if (!(x)) {                                                                \
      fprintf(stderr, "AI_ASSERT:%s:%d:%s\n", __FILE__, __LINE__, #x);         \
      exit(41);                                                                \
    }                                                                          \
  } while (0)
static void pool(void) {
  AIPool p;
  uint32_t handles[16], out = 99;
  CHECK(ai_init(&p, 8) == AI_OK);
  for (unsigned i = 0; i < 16; i++) {
    CHECK(ai_take(&p, &handles[i]) == AI_OK);
    CHECK((handles[i] & 255u) == i + 1 && ai_valid(&p, handles[i]));
  }
  CHECK(ai_take(&p, &out) == AI_FULL && out == 99);
  AIPool before = p;
  CHECK(ai_init(&p, 7) == AI_INVALID && !memcmp(&p, &before, sizeof p));
  CHECK(ai_release(&p, handles[0]) == AI_OK);
  CHECK(!ai_valid(&p, handles[0]));
  CHECK(ai_release(&p, handles[0]) == AI_INVALID);
  CHECK(ai_take(&p, &out) == AI_OK && out != handles[0]);
  CHECK(!ai_valid(&p, handles[0]));
  CHECK(ai_valid(&p, out));
  CHECK(!ai_valid(&p, 0) && !ai_valid(&p, 273) && !ai_valid(&p, 1));
  CHECK(ai_take(&p, NULL) == AI_INVALID);
  puts("OBS pool status stale-generation double-release invalid-layout");
}
static void exhaust(void) {
  AIPool p;
  CHECK(ai_init(&p, 8) == AI_OK);
  unsigned count = 0;
  uint32_t h = 0;
  for (unsigned slot = 0; slot < 16; slot++)
    for (unsigned gen = 1; gen <= 255; gen++) {
      CHECK(ai_take(&p, &h) == AI_OK);
      CHECK((h & 255u) == slot + 1 && (h >> 8) == gen);
      CHECK(ai_valid(&p, h));
      CHECK(ai_release(&p, h) == AI_OK);
      CHECK(!ai_valid(&p, h));
      count++;
    }
  h = 99;
  CHECK(ai_take(&p, &h) == AI_FULL && h == 99);
  for (unsigned i = 0; i < 16; i++)
    CHECK(p.retired[i] && !p.used[i]);
  printf("OBS exhaust incarnations=%u permanently-retired=16\n", count);
}
static void compact(void) {
  AIPool p;
  uint32_t h = 0;
  CHECK(ai_init(&p, 24) == AI_OK);
  /* Boundary projection, no claim that all sixteen-million generations ran. */
  for (unsigned i = 0; i < 16; i++)
    p.generation[i] = UINT32_C(0xffffff);
  for (unsigned i = 0; i < 16; i++) {
    CHECK(ai_take(&p, &h) == AI_OK);
    CHECK(h == (UINT32_C(0xffffff00) | (i + 1)));
    CHECK(ai_release(&p, h) == AI_OK);
    CHECK(!ai_valid(&p, h));
  }
  CHECK(ai_take(&p, &h) == AI_FULL);
  puts("OBS compact 8-index-bits 24-generation-bits saturation-boundary");
}
static void retained(void) {
  cmx_identity_pool p;
  cmx_identity_init(&p);
  uint32_t stale = 257, h = 0;
  for (unsigned gen = 1; gen <= 65535; gen++) {
    h = cmx_identity_take(&p);
    CHECK(h == ((gen << 8) | 1u));
    CHECK(cmx_identity_valid(&p, h));
    CHECK(cmx_identity_put(&p, h) == CMX_OK);
    CHECK(!cmx_identity_valid(&p, stale));
  }
  CHECK(p.generation[0] == 65536 && !p.used[0]);
  CHECK(cmx_identity_take(&p) == 258);
  CHECK(cmx_identity_put(&p, stale) == CMX_INVALID);
  puts("OBS retained exact-CMX 65535-incarnations slot0-retired slot1-live");
}
static void aba(void) {
  _Atomic(uint32_t) root;
  atomic_init(&root, 0);
  uint32_t paused = atomic_load(&root);
  unsigned identity_at_read = 1;
  atomic_store(&root, 1);
  unsigned identity_now = 2;
  atomic_store(&root, 0);
  CHECK(identity_at_read != identity_now);
  CHECK(ai_word_cas(&root, &paused, 2));
  atomic_store(&root, 257);
  paused = atomic_load(&root);
  atomic_store(&root, 258);
  atomic_store(&root, 513);
  CHECK(!ai_word_cas(&root, &paused, 514) && paused == 513);
  puts("OBS aba equal-index-bits stale-success; tagged-incarnation-rejects; "
       "no-freed-pointer");
}
int main(int argc, char **argv) {
  if (argc != 2)
    return 2;
  if (!strcmp(argv[1], "pool"))
    pool();
  else if (!strcmp(argv[1], "exhaust"))
    exhaust();
  else if (!strcmp(argv[1], "compact"))
    compact();
  else if (!strcmp(argv[1], "retained"))
    retained();
  else if (!strcmp(argv[1], "aba"))
    aba();
  else
    return 2;
  return 0;
}
