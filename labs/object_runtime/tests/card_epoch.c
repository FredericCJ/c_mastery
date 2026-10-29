#include "object_runtime_card_epoch.h"
#include <stdio.h>
#include <string.h>
#define CHECK(c)                                                               \
  do {                                                                         \
    if (!(c)) {                                                                \
      fprintf(stderr, "OR_CHECK:%d:%s\n", __LINE__, #c);                       \
      return 1;                                                                \
    }                                                                          \
  } while (0)
struct or_writer {
  struct orc_cards *cards;
  unsigned object;
  enum or_status result;
};
static void *late_writer(void *arg) {
  struct or_writer *w = arg;
  w->result = orcc_store(w->cards, w->object, 1, 7);
  return NULL;
}
static int concurrent(void) {
  unsigned width, padded, i;
  uint16_t roots;
  unsigned scanned;
  for (width = 1; width <= 8; width *= 2)
    for (padded = 0; padded < 2; padded++) {
      struct orc_cards c;
      struct orcc_snapshot s;
      pthread_t thread;
      struct or_writer w;
      CHECK(orcc_init(&c, width, (int)padded) == OR_OK);
      CHECK(orcc_store(&c, 0, 0, 1) == OR_OK);
      CHECK(orcc_store(&c, 0, 0, 0) == OR_OK);
      CHECK(orcc_prepare(&c, 0, &s) == OR_OK);
      w = (struct or_writer){.cards = &c, .object = width - 1};
      CHECK(pthread_create(&thread, NULL, late_writer, &w) == 0);
      CHECK(pthread_join(thread, NULL) == 0 && w.result == OR_OK);
      roots = 777;
      scanned = 888;
      CHECK(orcc_scan_commit(&c, &s, &roots, &scanned) == OR_STATE &&
            roots == 777 && scanned == 888);
      CHECK(orcc_prepare(&c, 0, &s) == OR_OK);
      CHECK(orcc_scan_commit(&c, &s, &roots, &scanned) == OR_OK &&
            roots == 64 && scanned == 2 * width);
      CHECK(orcc_store(&c, width - 1, 1, 0) == OR_OK);
      CHECK(orcc_prepare(&c, 0, &s) == OR_OK);
      CHECK(orcc_scan_commit(&c, &s, &roots, &scanned) == OR_OK && roots == 0 &&
            scanned == 2 * width);
      CHECK(orcc_prepare(&c, 0, &s) == OR_DONE);
      CHECK(orcc_close(&c) == OR_OK);
    }
  {
    struct orc_cards c;
    struct orcc_snapshot s;
    CHECK(orcc_init(&c, 2, 0) == OR_OK);
    for (i = 1; i < ORCC_REV_LIMIT; i++)
      CHECK(orcc_store(&c, 0, 0, 0) == OR_OK);
    CHECK(orcc_store(&c, 0, 0, 1) == OR_FULL &&
          orcc_prepare(&c, 0, &s) == OR_DONE);
    CHECK(orcc_close(&c) == OR_OK);
  }
  return 0;
}
static int oracle(void) {
  unsigned width, padded, object, field, target;
  for (width = 1; width <= 8; width *= 2)
    for (padded = 0; padded < 2; padded++)
      for (object = 0; object < 8; object++)
        for (field = 0; field < 2; field++)
          for (target = 0; target <= 16; target++) {
            struct orc_cards c;
            struct orcc_snapshot s;
            uint16_t roots = 777;
            unsigned scanned = 888;
            enum or_status status;
            CHECK(orcc_init(&c, width, (int)padded) == OR_OK);
            CHECK(orcc_store(&c, object, field, target) == OR_OK);
            status = orcc_prepare(&c, object / width, &s);
            if (status == OR_OK)
              status = orcc_scan_commit(&c, &s, &roots, &scanned);
            printf("C %u %u %u %u %u %d %u %u\n", width, padded, object, field,
                   target, status, roots, scanned);
            CHECK(orcc_close(&c) == OR_OK);
          }
  return 0;
}
int main(int argc, char **argv) {
  if (argc != 2)
    return 2;
  if (!strcmp(argv[1], "concurrent"))
    return concurrent();
  if (!strcmp(argv[1], "oracle"))
    return oracle();
  return 2;
}
