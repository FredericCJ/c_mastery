#ifndef RPO_TEST_SUPPORT_H
#define RPO_TEST_SUPPORT_H
#include "transport.h"
#include <stdio.h>
#define CHECK(x)                                                               \
  do {                                                                         \
    if (!(x)) {                                                                \
      fprintf(stderr, "RP_CHECK %s:%d %s\n", __FILE__, __LINE__, #x);          \
      return 1;                                                                \
    }                                                                          \
  } while (0)
typedef struct {
  unsigned char stable[STORE_IMAGE_PAGES][STORE_PAGE_SIZE];
  unsigned char pending[STORE_IMAGE_PAGES][STORE_PAGE_SIZE];
  unsigned char dirty[STORE_IMAGE_PAGES];
  unsigned event, cut, persist, dead;
} RpoMedium;
typedef struct {
  RpoMedium disk;
  RplPages adapter;
  store pages;
  RplLog log;
} RpoFixture;
int rpo_fixture(RpoFixture *, uint64_t, uint32_t);
int rpo_reopen(RpoFixture *);
int rpo_sync(RpoFixture *);
void rpo_cut(RpoFixture *, unsigned, unsigned);
#endif
