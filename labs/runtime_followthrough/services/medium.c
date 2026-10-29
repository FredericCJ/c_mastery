#include "medium.h"
#include <string.h>
static void persist(struct vh_medium *m) {
  for (unsigned i = STORE_IMAGE_PAGES; i; i--)
    if (m->dirty[i - 1]) {
      memcpy(m->stable[i - 1], m->pending[i - 1], STORE_PAGE_SIZE);
      m->dirty[i - 1] = 0;
    }
}
static int boundary(struct vh_medium *m) {
  m->event++;
  if (!m->cut || m->event != m->cut)
    return 0;
  if (m->persist_on_failure)
    persist(m);
  m->dead = 1;
  return -1;
}
static int rd(void *p, uint32_t page, void *out) {
  struct vh_medium *m = p;
  if (m->dead || page >= STORE_IMAGE_PAGES)
    return -1;
  memcpy(out, m->dirty[page] ? m->pending[page] : m->stable[page],
         STORE_PAGE_SIZE);
  return 0;
}
static int wr(void *p, uint32_t page, const void *in) {
  struct vh_medium *m = p;
  if (m->dead || page >= STORE_IMAGE_PAGES)
    return -1;
  memcpy(m->pending[page], in, STORE_PAGE_SIZE);
  m->dirty[page] = 1;
  return boundary(m);
}
static int flush(void *p) {
  struct vh_medium *m = p;
  if (m->dead || boundary(m))
    return -1;
  persist(m);
  return 0;
}
store_io vh_medium_io(struct vh_medium *m) {
  return (store_io){m, rd, wr, flush, STORE_IMAGE_PAGES};
}
void vh_medium_restart(struct vh_medium *m) {
  memset(m->dirty, 0, sizeof m->dirty);
  m->event = m->cut = m->persist_on_failure = m->dead = 0;
}
