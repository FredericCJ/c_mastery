#ifndef BF_MAP_H
#define BF_MAP_H
#include "depth.h"
#include "containers.h"
/* scratch is a distinct actual B2Entry[capacity], capacity >= 7.
 * Successful rebuild removes tombstones without changing capacity or keys.
 * fail_after 0 means no injected failure; 1..7 refuses after that many
 * staged live entries. Old map remains unchanged; scratch may be changed. */
int bf_map_rehash(struct b2_map *, struct b2_entry *, size_t capacity, unsigned fail_after);
/* Deliberately borrowed teaching target: live key bytes must be immutable.
 * Initialize to zero; at most four entries, each length <= 8. */
struct bf_borrow_map {
  const unsigned char *bytes[4];
  size_t length[4];
  int value[4];
  unsigned live[4];
};
int bf_borrow_put(struct bf_borrow_map *, const unsigned char *, size_t, int);
int bf_borrow_get(const struct bf_borrow_map *, const unsigned char *, size_t, int *);
#endif
