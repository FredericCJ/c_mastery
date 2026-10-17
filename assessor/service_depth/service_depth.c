#include "service_depth.h"
#include <limits.h>
static void copy_bytes(void *to, const void *from, size_t n) {
  unsigned char *d = to;
  const unsigned char *s = from;
  for (size_t i = 0; i < n; i++)
    d[i] = s[i];
}
static void zero_bytes(void *to, size_t n) {
  unsigned char *d = to;
  for (size_t i = 0; i < n; i++)
    d[i] = 0;
}
static unsigned take(SDMemory *m) {
  for (unsigned i = 0; i < 16; i++)
    if (!m->refs[i]) {
      m->refs[i] = 1;
      return i;
    }
  return 16;
}
int sd_release(SDMemory *m, SDImage *p) {
  if (!m || !p || !p->active || p->count > 4)
    return SD_INVALID;
  for (unsigned i = 0; i < p->count; i++)
    if (p->frames[i] >= 16 || !m->refs[p->frames[i]])
      return SD_INVALID;
  for (unsigned i = 0; i < p->count; i++) {
    unsigned f = p->frames[i];
    if (!--m->refs[f])
      zero_bytes(m->bytes[f], 64);
  }
  p->active = 0;
  return SD_OK;
}
int sd_load(SDMemory *m, const unsigned char *data, size_t n, int fail_after,
            SDImage *out) {
  if (!m || !data || !out || !n || n > 256 || fail_after < -1)
    return SD_INVALID;
  SDImage image = {0};
  image.active = 1;
  image.length = (unsigned)n;
  for (unsigned page = 0; page < (n + 63) / 64; page++) {
    unsigned f =
        (fail_after >= 0 && page == (unsigned)fail_after) ? 16 : take(m);
    if (f == 16) {
#ifndef SD_MUTATE_MEMORY
      (void)sd_release(m, &image);
#endif
      return SD_FULL;
    }
    image.frames[image.count++] = f;
    zero_bytes(m->bytes[f], 64);
    size_t at = (size_t)page * 64, k = n - at;
    if (k > 64)
      k = 64;
    copy_bytes(m->bytes[f], data + at, k);
  }
  *out = image;
  return SD_OK;
}
int sd_clone(SDMemory *m, SDImage *p, unsigned threads, SDImage *out) {
  if (!m || !p || !out || !p->active || !p->count || p->count > 4 ||
      threads != 1)
    return SD_INVALID;
  for (unsigned i = 0; i < p->count; i++) {
    if (p->frames[i] >= 16 || !m->refs[p->frames[i]] ||
        m->refs[p->frames[i]] >= 65535)
      return SD_FULL;
#ifndef SD_MUTATE_MEMORY2
    if (p->kind[i])
      return SD_DENIED;
#endif
  }
  SDImage child = *p;
  for (unsigned i = 0; i < p->count; i++) {
    m->refs[p->frames[i]]++;
    p->cow[i] = child.cow[i] = 1;
  }
  *out = child;
  return SD_OK;
}
int sd_write(SDMemory *m, SDImage *p, unsigned off, unsigned char value,
             int allowed) {
  if (!m || !p || !p->active || off >= p->length || allowed < 0 || allowed > 1)
    return SD_INVALID;
  unsigned i = off / 64, f = p->frames[i];
  if (f >= 16 || !m->refs[f])
    return SD_INVALID;
  if (m->refs[f] > 1) {
    if (!allowed)
      return SD_FULL;
    unsigned fresh = take(m);
    if (fresh == 16)
      return SD_FULL;
    copy_bytes(m->bytes[fresh], m->bytes[f], 64);
    m->refs[f]--;
    p->frames[i] = fresh;
    f = fresh;
  }
  p->cow[i] = 0;
  m->bytes[f][off % 64] = value;
  return SD_OK;
}
static int cap_check(const SDChannel *q, SDCap c, unsigned right) {
  if (!q)
    return SD_INVALID;
  if (q->retired || c.epoch != q->epoch)
    return SD_STALE;
  if (c.rights > 7 || (c.rights & right) != right)
    return SD_DENIED;
  return SD_OK;
}
int sd_channel_init(SDChannel *q) {
  if (!q)
    return SD_INVALID;
  *q = (SDChannel){0};
  q->epoch = 1;
  return SD_OK;
}
int sd_grant(const SDChannel *q, SDCap c, unsigned rights, SDCap *out) {
  if (!out || !rights || rights > 7)
    return SD_INVALID;
  int r = cap_check(q, c, rights);
  if (r)
    return r;
  *out = (SDCap){q->epoch, rights};
  return SD_OK;
}
int sd_send(SDChannel *q, SDCap c, const unsigned char *data, size_t n) {
  int r = cap_check(q, c, 1);
  if (r)
    return r;
  if ((!data && n) || n > 32)
    return SD_INVALID;
  if (q->closed)
    return SD_CLOSED;
  if (q->count == 2)
    return SD_FULL;
  SDMessage *m = &q->slots[(q->head + q->count) % 2];
  m->length = (unsigned)n;
  copy_bytes(m->bytes, data, n);
  q->count++;
  return SD_OK;
}
int sd_receive(SDChannel *q, SDCap c, void *out, size_t cap, size_t *n,
               SDValidate check, void *ctx) {
#ifdef SD_MUTATE_CHANNEL2
  int r = cap_check(q, c, 0);
#else
  int r = cap_check(q, c, 2);
#endif
  if (r)
    return r;
  if (!n || !check)
    return SD_INVALID;
  if (!q->count)
    return q->closed ? SD_CLOSED : SD_EMPTY;
  SDMessage *m = &q->slots[q->head];
  if (cap < m->length || (!out && m->length))
    return SD_FULL;
#ifdef SD_MUTATE_CHANNEL
  q->head = (q->head + 1) % 2;
  q->count--;
#endif
  if (check(ctx, out, m->length))
    return SD_COPY;
  copy_bytes(out, m->bytes, m->length);
  *n = m->length;
#ifndef SD_MUTATE_CHANNEL
  q->head = (q->head + 1) % 2;
  q->count--;
#endif
  return SD_OK;
}
int sd_channel_close(SDChannel *q, SDCap c) {
  int r = cap_check(q, c, 4);
  if (r)
    return r;
  q->closed = 1;
  return SD_OK;
}
int sd_channel_reopen(SDChannel *q) {
  if (!q || !q->closed || q->count)
    return SD_INVALID;
  if (q->retired)
    return SD_STALE;
  if (q->epoch == UINT32_MAX) {
    q->retired = 1;
    return SD_FULL;
  }
  q->epoch++;
  q->closed = 0;
  q->head = 0;
  return SD_OK;
}
static int name_valid(const char *s) {
  if (!s)
    return 0;
  unsigned n = 0;
  for (; n < 12 && s[n]; n++) {
    unsigned char c = (unsigned char)s[n];
    if (!((c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') || c == '_' ||
          c == '-' || c == '.'))
      return 0;
  }
  return n && n < 12 && !(n == 1 && s[0] == '.') &&
         !(n == 2 && s[0] == '.' && s[1] == '.');
}
static int same(const char *a, const char *b) {
  unsigned i = 0;
  while (a[i] && a[i] == b[i])
    i++;
  return a[i] == b[i];
}
int sd_files_init(SDFiles *fs, const SDEntry *entries, unsigned n) {
  if (!fs || !entries || !n || n > 4)
    return SD_INVALID;
  SDFiles next = {0};
  for (unsigned i = 0; i < n; i++) {
    if (!name_valid(entries[i].name) || entries[i].length > 256 ||
        (!entries[i].data && entries[i].length))
      return SD_INVALID;
    for (unsigned j = 0; j < i; j++)
      if (same(entries[i].name, entries[j].name))
        return SD_INVALID;
    next.entries[i] = entries[i];
  }
  next.count = n;
  *fs = next;
  return SD_OK;
}
static unsigned file_handle(const SDFiles *fs, SDHandle h) {
  return fs && h.index < 8 && fs->handles[h.index].active &&
                 fs->handles[h.index].generation == h.generation
             ? h.index
             : 8;
}
static unsigned free_handle(const SDFiles *fs) {
  for (unsigned i = 0; i < 8; i++)
    if (!fs->handles[i].active && fs->handles[i].generation < UINT32_MAX)
      return i;
  return 8;
}
int sd_open(SDFiles *fs, const char *name, SDHandle *out) {
  if (!fs || !out || !name_valid(name))
    return SD_INVALID;
  unsigned e = 0;
  while (e < fs->count && !same(name, fs->entries[e].name))
    e++;
  if (e == fs->count)
    return SD_INVALID;
  unsigned h = free_handle(fs), d = 0;
  while (d < 4 && fs->descriptions[d].active)
    d++;
  if (h == 8 || d == 4)
    return SD_FULL;
  fs->descriptions[d] = (SDOpen){1, e, 0, 1};
  SDFileHandle *v = &fs->handles[h];
  v->active = 1;
  v->open = d;
  v->generation++;
  *out = (SDHandle){h, v->generation};
  return SD_OK;
}
int sd_dup(SDFiles *fs, SDHandle h, SDHandle *out) {
  if (!fs || !out)
    return SD_INVALID;
  unsigned old = file_handle(fs, h);
  if (old == 8)
    return SD_STALE;
  unsigned n = free_handle(fs);
  if (n == 8)
    return SD_FULL;
  SDFileHandle *v = &fs->handles[n];
  v->active = 1;
  v->open = fs->handles[old].open;
  v->generation++;
#ifdef SD_MUTATE_FILES
  unsigned d = 0;
  while (d < 4 && fs->descriptions[d].active)
    d++;
  if (d < 4) {
    fs->descriptions[d] = fs->descriptions[v->open];
    fs->descriptions[d].refs = 0;
    v->open = d;
  }
#endif
  fs->descriptions[v->open].refs++;
  *out = (SDHandle){n, v->generation};
  return SD_OK;
}
int sd_seek(SDFiles *fs, SDHandle h, unsigned origin, int32_t delta) {
  if (!fs || origin > 2)
    return SD_INVALID;
  unsigned i = file_handle(fs, h);
  if (i == 8)
    return SD_STALE;
  SDOpen *d = &fs->descriptions[fs->handles[i].open];
  int64_t base = origin == 0   ? 0
                 : origin == 1 ? d->offset
                               : fs->entries[d->entry].length;
  int64_t offset = base + delta;
  if (offset < 0 || offset > fs->entries[d->entry].length)
    return SD_INVALID;
  d->offset = (unsigned)offset;
  return SD_OK;
}
int sd_read(SDFiles *fs, SDHandle h, void *out, size_t cap, size_t requested,
            size_t *n, SDValidate check, void *ctx) {
  if (!fs || !n || !check || requested > cap)
    return SD_INVALID;
  unsigned i = file_handle(fs, h);
  if (i == 8)
    return SD_STALE;
  SDOpen *d = &fs->descriptions[fs->handles[i].open];
  SDEntry *e = &fs->entries[d->entry];
  size_t amount = e->length - d->offset;
  if (amount > requested)
    amount = requested;
  if (!out && amount)
    return SD_INVALID;
#ifdef SD_MUTATE_FILES2
  d->offset += (unsigned)amount;
#endif
  if (check(ctx, out, amount))
    return SD_COPY;
  if (amount) {
#ifdef SD_MUTATE_FILES2
    copy_bytes(out, e->data + d->offset - amount, amount);
#else
    copy_bytes(out, e->data + d->offset, amount);
#endif
  }
#ifndef SD_MUTATE_FILES2
  d->offset += (unsigned)amount;
#endif
  *n = amount;
  return SD_OK;
}
int sd_close(SDFiles *fs, SDHandle h) {
  if (!fs)
    return SD_INVALID;
  unsigned i = file_handle(fs, h);
  if (i == 8)
    return SD_STALE;
  SDFileHandle *v = &fs->handles[i];
  SDOpen *d = &fs->descriptions[v->open];
  v->active = 0;
  if (!--d->refs)
    d->active = 0;
  return SD_OK;
}
static uint32_t read32(const unsigned char *p) {
  return (uint32_t)p[0] | (uint32_t)p[1] << 8 | (uint32_t)p[2] << 16 |
         (uint32_t)p[3] << 24;
}
static void put32(unsigned char *p, uint32_t v) {
  for (unsigned i = 0; i < 4; i++)
    p[i] = (unsigned char)(v >> (i * 8));
}
static int32_t signed32(uint32_t n) {
  return n <= INT32_MAX ? (int32_t)n : -(int32_t)(UINT32_MAX - n) - 1;
}
static uint32_t checksum(const unsigned char *p) {
  uint32_t n = 2166136261u;
  for (unsigned i = 0; i < 16; i++)
    n = (n ^ p[i]) * 16777619u;
  return n;
}
int sd_recover(const SDJournal *j, SDReceipt *out) {
  if (!j || !out)
    return SD_INVALID;
  SDReceipt r = {0};
  for (unsigned i = 0; i < 2; i++) {
    const unsigned char *p = j->durable[i];
    if (read32(p + 20) != 0xc017c017u)
      continue;
    if (read32(p) != 0x53445231u || !read32(p + 4) ||
        read32(p + 16) != checksum(p))
      return SD_CORRUPT;
    uint32_t id = read32(p + 4);
    if (id == r.last)
      return SD_CORRUPT;
    if (id > r.last)
      r = (SDReceipt){id, signed32(read32(p + 8)), signed32(read32(p + 12))};
  }
  *out = r;
  return SD_OK;
}
int sd_apply(SDJournal *j, uint32_t id, int32_t delta, unsigned fault,
             int32_t *out) {
  if (!j || !out || fault > 4 || !id)
    return SD_INVALID;
  SDReceipt r;
  int s = sd_recover(j, &r);
  if (s)
    return s;
  if (id == r.last) {
#ifndef SD_MUTATE_JOURNAL2
    if (delta != r.delta)
      return SD_CONFLICT;
#endif
    *out = r.value;
    return SD_OK;
  }
  if (id < r.last)
    return SD_STALE;
  if (r.last == UINT32_MAX || id != r.last + 1)
    return SD_INVALID;
  int64_t value = (int64_t)r.value + delta;
  if (value < INT32_MIN || value > INT32_MAX)
    return SD_FULL;
  unsigned slot = id & 1u;
  unsigned char *p = j->pending[slot];
  zero_bytes(p, 32);
  put32(p, 0x53445231u);
  put32(p + 4, id);
  put32(p + 8, (uint32_t)delta);
  put32(p + 12, (uint32_t)(int32_t)value);
  put32(p + 16, checksum(p));
  if (fault == 1)
    return SD_UNCERTAIN;
  copy_bytes(j->durable[slot], p, 32);
  if (fault == 2)
    return SD_UNCERTAIN;
  put32(p + 20, 0xc017c017u);
#ifdef SD_MUTATE_JOURNAL
  if (fault == 3) {
    *out = (int32_t)value;
    return SD_OK;
  }
#else
  if (fault == 3)
    return SD_UNCERTAIN;
#endif
  copy_bytes(j->durable[slot], p, 32);
  if (fault == 4)
    return SD_UNCERTAIN;
  *out = (int32_t)value;
  return SD_OK;
}
