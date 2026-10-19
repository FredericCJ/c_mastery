#include "cmk_contract.h"
static uint16_t u16(const unsigned char *p) {
  return (uint16_t)((unsigned)p[0] | (unsigned)p[1] << 8);
}
static uint32_t u32(const unsigned char *p) {
  return (uint32_t)u16(p) | (uint32_t)u16(p + 2) << 16;
}
static uint64_t u64(const unsigned char *p) {
  return (uint64_t)u32(p) | (uint64_t)u32(p + 4) << 32;
}
static int extent(uint64_t p, uint64_t n, uint64_t end) {
  return p <= end && n <= end - p;
}
int cmk_user_range(uint64_t p, uint64_t n) {
  return p >= CMK_USER_BASE && p < CMK_USER_LIMIT &&
         extent(p, n, CMK_USER_LIMIT);
}
uint32_t cmk_handle(unsigned s, unsigned g) {
  return s < 16 && g > 0 && g < 65536 ? ((uint32_t)g << 16) | (s + 1u) : 0;
}
int cmk_handle_valid(uint32_t h, unsigned s, unsigned g, int o) {
  return o && h != 0 && h == cmk_handle(s, g);
}
int cmk_boot_parse(const void *v, size_t n, struct cmk_boot *out) {
  const unsigned char *p = v;
  struct cmk_boot b = {0};
  size_t off = 8;
  int ended = 0;
  if (!p || !out || n < 16 || u32(p) > n || u32(p) < 16 || (u32(p) & 7u))
    return -1;
  n = u32(p);
  while (off <= n - 8) {
    uint32_t type = u32(p + off), size = u32(p + off + 4);
    if (size < 8 || size > n - off)
      return -1;
    if (type == 0) {
      if (size != 8 || off + 8 != n)
        return -1;
      ended = 1;
      break;
    }
    if (type == 3) {
      if (size < 17 || b.module_end || u32(p + off + 8) >= u32(p + off + 12))
        return -1;
      b.module_start = u32(p + off + 8);
      b.module_end = u32(p + off + 12);
    }
    if (type == 6) {
      if (size < 16 || b.map || u32(p + off + 8) < 24 || u32(p + off + 8) % 8 ||
          u32(p + off + 12) != 0)
        return -1;
      b.entry_size = u32(p + off + 8);
      b.map_bytes = size - 16;
      if (b.map_bytes % b.entry_size)
        return -1;
      b.map = p + off + 16;
      for (unsigned j = 0; j < b.map_bytes; j += b.entry_size)
        if (u64(b.map + j + 8) > UINT64_MAX - u64(b.map + j))
          return -1;
    }
    /* Add alignment padding only after proving the remaining extent. This
       also works when size is near UINT32_MAX on a 32-bit size_t host. */
    size_t advance = size, padding = (8u - (size & 7u)) & 7u;
    if (padding > n - off - advance)
      return -1;
    off += advance + padding;
  }
  if (!ended || !b.map || !b.module_end)
    return -1;
  *out = b;
  return 0;
}
int cmk_elf_parse(const void *v, size_t n, struct cmk_elf *out) {
  const unsigned char *p = v;
  struct cmk_elf e = {0};
  int executable = 0;
  if (!p || !out || n < 64 || p[0] != 127 || p[1] != 'E' || p[2] != 'L' ||
      p[3] != 'F' || p[4] != 2 || p[5] != 1 || p[6] != 1 || u16(p + 16) != 2 ||
      u16(p + 18) != 62 || u32(p + 20) != 1 || u16(p + 52) != 64 ||
      u16(p + 54) != 56)
    return -1;
  uint64_t ph = u64(p + 32);
  unsigned count = u16(p + 56);
  e.entry = u64(p + 24);
  if (count == 0 || count > 16 || !extent(ph, (uint64_t)count * 56, n))
    return -1;
  for (unsigned i = 0; i < count; i++) {
    const unsigned char *h = p + ph + i * 56;
    uint32_t type = u32(h);
    if (type == 2 || type == 3 || type == 7)
      return -1;
    if (type != 1)
      continue;
    if (e.count == CMK_MAX_SEGMENTS)
      return -1;
    struct cmk_segment s = {u64(h + 8), u64(h + 16), u64(h + 32), u64(h + 40),
                            u32(h + 4)};
    uint64_t a = u64(h + 48);
    if (!s.memsz || s.filesz > s.memsz || !extent(s.offset, s.filesz, n) ||
        !cmk_user_range(s.address, s.memsz) ||
        s.address + s.memsz > CMK_USER_BASE + 0x100000 || !(s.flags & 4) ||
        s.flags & ~7u || (s.flags & 3) == 3 || a != 4096 ||
        (s.offset & 4095) != (s.address & 4095))
      return -1;
    uint64_t lo = s.address & ~UINT64_C(4095),
             hi = (s.address + s.memsz + 4095) & ~UINT64_C(4095);
    for (unsigned j = 0; j < e.count; j++) {
      struct cmk_segment *t = &e.segments[j];
      uint64_t tl = t->address & ~UINT64_C(4095),
               th = (t->address + t->memsz + 4095) & ~UINT64_C(4095);
      if (lo < th && tl < hi)
        return -1;
    }
    if ((s.flags & 1) && e.entry >= s.address && e.entry < s.address + s.memsz)
      executable = 1;
    e.segments[e.count++] = s;
  }
  if (!e.count || !executable)
    return -1;
  *out = e;
  return 0;
}
int cmk_archive_find(const void *v, size_t n, const char *name,
                     const unsigned char **data, size_t *len) {
  const unsigned char *p = v;
  size_t wanted = 0;
  int found = 0;
  const unsigned char *result = 0;
  size_t bytes = 0;
  if (!p || !name || !data || !len || n < 16 || p[0] != 'C' || p[1] != 'M' ||
      p[2] != 'K' || p[3] != 'A' || u32(p + 4) != 1 || u32(p + 12) != n)
    return -1;
  unsigned count = u32(p + 8);
  if (count == 0 || count > 16 || 16u + 40u * count > n)
    return -1;
  while (wanted < 24 && name[wanted])
    wanted++;
  if (!wanted || wanted >= 24)
    return -1;
  for (unsigned i = 0; i < count; i++) {
    const unsigned char *e = p + 16 + 40 * i;
    size_t k = 0;
    while (k < 24 && e[k])
      k++;
    if (!k || k == 24)
      return -1;
    for (unsigned z = (unsigned)k + 1; z < 24; z++)
      if (e[z])
        return -1;
    uint32_t off = u32(e + 24), size = u32(e + 28);
    if (!size || off < 16 + 40 * count || !extent(off, size, n) ||
        u32(e + 32) || u32(e + 36))
      return -1;
    for (unsigned j = 0; j < i; j++) {
      const unsigned char *a = p + 16 + 40 * j;
      uint32_t ao = u32(a + 24), az = u32(a + 28);
      if (off < ao + az && ao < off + size)
        return -1;
      unsigned z = 0;
      while (z < 24 && e[z] == a[z] && e[z])
        z++;
      if (z < 24 && e[z] == a[z])
        return -1;
    }
    if (k == wanted) {
      size_t z = 0;
      while (z < k && e[z] == (unsigned char)name[z])
        z++;
      if (z == k) {
        result = p + off;
        bytes = size;
        found = 1;
      }
    }
  }
  if (!found)
    return -1;
  *data = result;
  *len = bytes;
  return 0;
}
void cmk_frames_init(struct cmk_frames *f, const struct cmk_boot *b,
                     uint32_t reserved) {
  for (unsigned i = 0; i < CMK_MAX_FRAMES; i++)
    f->state[i] = 0;
  for (unsigned j = 0; j < b->map_bytes; j += b->entry_size) {
    const unsigned char *p = b->map + j;
    uint64_t start = u64(p), end = start + u64(p + 8);
    if (u32(p + 16) != 1)
      continue;
    for (unsigned i = 0; i < CMK_MAX_FRAMES; i++) {
      uint64_t a = (uint64_t)i * 4096;
      if (a >= reserved && a >= start && a + 4096 <= end)
        f->state[i] = 1;
    }
  }
  /* Reserved overlapping entries take precedence over usable entries. */
  for (unsigned j = 0; j < b->map_bytes; j += b->entry_size) {
    const unsigned char *p = b->map + j;
    uint64_t start = u64(p), end = start + u64(p + 8);
    if (u32(p + 16) == 1)
      continue;
    for (unsigned i = 0; i < CMK_MAX_FRAMES; i++) {
      uint64_t a = (uint64_t)i * 4096;
      if (a < end && start < a + 4096)
        f->state[i] = 0;
    }
  }
}
uint32_t cmk_frame_take(struct cmk_frames *f) {
  for (unsigned i = 1; i < CMK_MAX_FRAMES; i++)
    if (f->state[i] == 1) {
      f->state[i] = 2;
      return i * 4096;
    }
  return 0;
}
int cmk_frame_put(struct cmk_frames *f, uint32_t p) {
  if ((p & 4095) || p / 4096 >= CMK_MAX_FRAMES || f->state[p / 4096] != 2)
    return -1;
  f->state[p / 4096] = 1;
  return 0;
}
