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
struct rpo_medium {
  unsigned char stable[STORE_IMAGE_PAGES][STORE_PAGE_SIZE];
  unsigned char pending[STORE_IMAGE_PAGES][STORE_PAGE_SIZE];
  unsigned char dirty[STORE_IMAGE_PAGES];
  unsigned event, cut, persist, dead;
};
struct rpo_fixture {
  struct rpo_medium disk;
  struct rpl_pages adapter;
  store pages;
  struct rpl_log log;
};
int rpo_fixture(struct rpo_fixture *, uint64_t, uint32_t);
int rpo_reopen(struct rpo_fixture *);
int rpo_sync(struct rpo_fixture *);
void rpo_cut(struct rpo_fixture *, unsigned, unsigned);
#endif
