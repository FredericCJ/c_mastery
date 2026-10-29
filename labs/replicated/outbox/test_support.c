#include "test_support.h"
#include <string.h>
static void persist(struct rpo_medium *m) {
  for (unsigned i = 0; i < STORE_IMAGE_PAGES; ++i)
    if (m->dirty[i]) {
      memcpy(m->stable[i], m->pending[i], STORE_PAGE_SIZE);
      m->dirty[i] = 0;
    }
}
static int fault(struct rpo_medium *m) {
  if (++m->event != m->cut || !m->cut)
    return 0;
  if (m->persist)
    persist(m);
  m->dead = 1;
  return 1;
}
static ptrdiff_t rd(void *c, size_t at, unsigned char *out, size_t n) {
  struct rpo_medium *m = c;
  if (m->dead || at >= sizeof m->stable || n > sizeof m->stable - at)
    return -1;
  unsigned p = (unsigned)(at / STORE_PAGE_SIZE);
  size_t off = at % STORE_PAGE_SIZE;
  if (n > STORE_PAGE_SIZE - off)
    n = STORE_PAGE_SIZE - off;
  memcpy(out, (m->dirty[p] ? m->pending[p] : m->stable[p]) + off, n);
  return (ptrdiff_t)n;
}
static ptrdiff_t wr(void *c, size_t at, const unsigned char *in, size_t n) {
  struct rpo_medium *m = c;
  if (m->dead || at >= sizeof m->stable || n > sizeof m->stable - at)
    return -1;
  unsigned p = (unsigned)(at / STORE_PAGE_SIZE);
  size_t off = at % STORE_PAGE_SIZE;
  if (n > STORE_PAGE_SIZE - off)
    n = STORE_PAGE_SIZE - off;
  if (!m->dirty[p])
    memcpy(m->pending[p], m->stable[p], STORE_PAGE_SIZE);
  memcpy(m->pending[p] + off, in, n);
  m->dirty[p] = 1;
  return fault(m) ? -1 : (ptrdiff_t)n;
}
static int flush(void *c) {
  struct rpo_medium *m = c;
  if (m->dead || fault(m))
    return -1;
  persist(m);
  return 0;
}
static int attach(struct rpo_fixture *f, store_io *out) {
  return rpl_pages(
      &f->adapter,
      (struct rpl_bytes){&f->disk, rd, wr, flush, sizeof f->disk.stable, 4}, out);
}
int rpo_fixture(struct rpo_fixture *f, uint64_t uuid, uint32_t schema) {
  memset(f, 0, sizeof *f);
  store_io io;
  CHECK(attach(f, &io) == RPL_OK);
  CHECK(store_format(io) == STORE_OK);
  CHECK(store_open(&f->pages, io) == STORE_OK);
  CHECK(rpl_create(&f->pages, uuid, schema, &f->log) == RPL_OK);
  CHECK(store_checkpoint(&f->pages) == STORE_OK);
  f->disk.event = 0;
  return 0;
}
int rpo_reopen(struct rpo_fixture *f) {
  uint64_t uuid = f->log.uuid;
  uint32_t schema = f->log.schema;
  memset(f->disk.dirty, 0, sizeof f->disk.dirty);
  f->disk.cut = f->disk.dead = f->disk.event = 0;
  store_io io;
  CHECK(attach(f, &io) == RPL_OK);
  CHECK(store_open(&f->pages, io) == STORE_OK);
  CHECK(rpl_open(&f->pages, uuid, schema, &f->log) == RPL_OK);
  return 0;
}
int rpo_sync(struct rpo_fixture *f) {
  CHECK(store_checkpoint(&f->pages) == STORE_OK);
  f->disk.event = 0;
  return 0;
}
void rpo_cut(struct rpo_fixture *f, unsigned cut, unsigned mode) {
  f->disk.event = 0;
  f->disk.cut = cut;
  f->disk.persist = mode;
}
