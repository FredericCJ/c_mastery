#include "contracts.h"
#include <stdio.h>
#include <string.h>
#define CHECK(x)                                                               \
  do {                                                                         \
    if (!(x)) {                                                                \
      fprintf(stderr, "KF_CHECK:%u:%s\n", __LINE__, #x);                       \
      return 1;                                                                \
    }                                                                          \
  } while (0)
static void w16(unsigned char *p, unsigned x) {
  p[0] = (unsigned char)x;
  p[1] = (unsigned char)(x >> 8);
}
static void w32(unsigned char *p, uint32_t x) {
  for (unsigned i = 0; i < 4; ++i)
    p[i] = (unsigned char)(x >> (8 * i));
}
static void w64(unsigned char *p, uint64_t x) {
  for (unsigned i = 0; i < 8; ++i)
    p[i] = (unsigned char)(x >> (8 * i));
}
static int permissions(void) {
  unsigned cases = 0;
  for (unsigned bits = 0; bits < 65536; ++bits) {
    uint64_t entries[4];
    unsigned present = 1, user = 1, write = 1, nx = 0;
    for (unsigned i = 0; i < 4; ++i) {
      unsigned x = (bits >> (4 * i)) & 15;
      entries[i] = (x & 7) | ((uint64_t)(x >> 3) << 63);
      present &= !!(x & 1);
      write &= !!(x & 2);
      user &= !!(x & 4);
      nx |= !!(x & 8);
    }
    for (unsigned access = 0; access < 3; ++access) {
      unsigned fault = 99;
      int allowed = kf_permission(entries, access == 1, access == 2, 1, &fault);
      unsigned expected =
          present && user && (access != 1 || write) && (access != 2 || !nx);
      CHECK(allowed == (int)expected);
      CHECK(fault ==
            (expected ? 0
                      : present + 2 * (access == 1) + 4 + 16 * (access == 2)));
      ++cases;
    }
  }
  CHECK(cases == 196608);
  uint64_t e[] = {7, 7, 7, 3};
  unsigned fault;
  CHECK(!kf_permission(e, 1, 0, 1, &fault) && fault == 7);
  e[3] = 7 | UINT64_C(1) << 63;
  CHECK(!kf_permission(e, 0, 1, 1, &fault) && fault == 21);
  e[3] = 0;
  CHECK(!kf_permission(e, 1, 0, 1, &fault) && fault == 6);
  puts("KF_PASS:permissions:196608-effective-level-access-cases:write7-NX21-"
       "guard6");
  return 0;
}
static int scheduling(void) {
  unsigned a[] = {1, 0, 1}, current = 0, expect[] = {2, 0, 2, 0};
  for (unsigned i = 0; i < 4; ++i) {
    int n = kf_next(a, 3, current);
    CHECK(n == (int)expect[i]);
    current = (unsigned)n;
  }
  for (unsigned bits = 0; bits < 8; ++bits)
    for (unsigned c = 0; c < 3; ++c) {
      unsigned alive[3];
      for (unsigned i = 0; i < 3; ++i)
        alive[i] = (bits >> i) & 1;
      int wanted = -1;
      for (unsigned d = 1; d <= 3; ++d)
        if (alive[(c + d) % 3]) {
          wanted = (int)((c + d) % 3);
          break;
        }
      CHECK(kf_next(alive, 3, c) == wanted);
    }
  unsigned weights[] = {1, 0, 2}, count[3] = {0};
  int64_t credits[3] = {0};
  for (unsigned i = 0; i < 300; ++i) {
    int pick = kf_weighted(a, weights, 3, credits);
    CHECK(pick == 0 || pick == 2);
    ++count[pick];
  }
  CHECK(count[0] == 100 && count[2] == 200);
  int64_t old[3];
  memcpy(old, credits, sizeof old);
  weights[0] = 65536;
  CHECK(kf_weighted(a, weights, 3, credits) == -1 &&
        !memcmp(old, credits, sizeof old));
  puts("KF_PASS:scheduling:2-0-2-0:zero-one-all-alive-sets:weighted100-200:"
       "transactional");
  return 0;
}
static KfPool pool;
static KfSpace space;
static void init(void) {
  memset(&pool, 0, sizeof pool);
  memset(pool.bytes, 0xa5, sizeof pool.bytes);
  for (unsigned i = 768; i < 1024; ++i)
    pool.frames.state[i] = 1;
  memset(&space, 0, sizeof space);
  space.live = 1;
  space.generation = 1;
}
static unsigned char elf[65536];
static size_t image(unsigned count) {
  memset(elf, 0, sizeof elf);
  memcpy(elf, "\177ELF", 4);
  elf[4] = 2;
  elf[5] = 1;
  elf[6] = 1;
  w16(elf + 16, 2);
  w16(elf + 18, 62);
  w32(elf + 20, 1);
  w64(elf + 24, CMK_USER_BASE);
  w64(elf + 32, 64);
  w16(elf + 52, 64);
  w16(elf + 54, 56);
  w16(elf + 56, count);
  for (unsigned i = 0; i < count; ++i) {
    unsigned char *h = elf + 64 + 56 * i;
    w32(h, 1);
    w32(h + 4, i ? 6 : 5);
    w64(h + 8, 4096 * (i + 1));
    w64(h + 16, CMK_USER_BASE + 4096 * i);
    w64(h + 32, 1);
    w64(h + 40, 4096);
    w64(h + 48, 4096);
    elf[4096 * (i + 1)] = (unsigned char)(i + 1);
  }
  return 4096 * (count + 1);
}
static int refusal(size_t n) {
  KfSpace before = space;
  cmk_frames state = pool.frames;
  return !kf_load_image(&pool, &space, elf, n, 256) &&
         !memcmp(&space, &before, sizeof space) &&
         !memcmp(&pool.frames, &state, sizeof state);
}
static int images(void) {
  init();
  size_t n = image(1);
  CHECK(kf_load_image(&pool, &space, elf, n, 1));
  CHECK(pool.bytes[0][0] == 1);
  for (unsigned i = 1; i < 4096; ++i)
    CHECK(pool.bytes[0][i] == 0);
  CHECK(space.pages[0].permissions == 13);
  init();
  n = image(1);
  w64(elf + 64 + 40, 8192);
  CHECK(kf_load_image(&pool, &space, elf, n, 2));
  CHECK(pool.bytes[0][0] == 1);
  for (unsigned i = 1; i < 8192; ++i)
    CHECK(pool.bytes[i / 4096][i % 4096] == 0);
  CHECK(kf_free(&pool) == 254);
  init();
  n = image(9);
  CHECK(refusal(n));
  init();
  n = image(1);
  w64(elf + 64 + 32, 4097);
  CHECK(refusal(n));
  n = image(1);
  w64(elf + 64 + 8, UINT64_MAX - 4095);
  CHECK(refusal(n));
  n = image(1);
  w64(elf + 24, CMK_USER_BASE + 4096);
  CHECK(refusal(n));
  n = image(1);
  w32(elf + 68, 7);
  CHECK(refusal(n));
  n = image(2);
  w64(elf + 120 + 16, CMK_USER_BASE);
  CHECK(refusal(n));
  n = image(2);
  w64(elf + 64 + 40, 1);
  w64(elf + 120 + 8, 4097);
  w64(elf + 120 + 16, CMK_USER_BASE + 1);
  w64(elf + 120 + 40, 1);
  CHECK(refusal(n));
  for (unsigned type = 2; type <= 7; ++type)
    if (type == 2 || type == 3 || type == 7) {
      n = image(2);
      w32(elf + 120, type);
      CHECK(refusal(n));
    }
  n = image(1);
  w64(elf + 64 + 48, 1);
  CHECK(refusal(n));
  n = image(1);
  w16(elf + 16, 3);
  CHECK(refusal(n));
  /* No section headers: source image already has e_shoff/e_shnum/e_shstrndx0.
   */
  init();
  n = image(2);
  CHECK(kf_load_image(&pool, &space, elf, n, 2));
  CHECK(pool.bytes[0][0] == 1 && pool.bytes[1][0] == 2);
  for (unsigned fail = 0; fail < 2; ++fail) {
    init();
    n = image(2);
    KfSpace before = space;
    cmk_frames state = pool.frames;
    CHECK(!kf_load_image(&pool, &space, elf, n, fail) &&
          !memcmp(&space, &before, sizeof space) &&
          !memcmp(&pool.frames, &state, sizeof state));
  }
  puts("KF_PASS:images:late9th-filesz-wrap-entry-overlap-WX-profile:4095-8191-"
       "zero-tail:no-sections:rollback");
  return 0;
}
static int archives(void) {
  unsigned char a[128] = {0};
  memcpy(a, "CMKA", 4);
  w32(a + 4, 1);
  w32(a + 8, 1);
  w32(a + 12, 64);
  memcpy(a + 16, "x", 2);
  w32(a + 40, 56);
  w32(a + 44, 8);
  const unsigned char *p = 0;
  size_t n = 0;
  CHECK(cmk_archive_find(a, 64, "x", &p, &n) == 0 && p == a + 56 && n == 8);
  /* Successor version2 uses the old reserved word as a bounded entry type. */
  w32(a + 4, 2);
  w32(a + 48, 2);
  CHECK(kf_archive_typed(a, 64, "x", 2, &p, &n) && p == a + 56 && n == 8);
  const unsigned char *old = p;
  size_t len = n;
  CHECK(!kf_archive_typed(a, 64, "x", 1, &p, &n) && p == old && n == len);
  w32(a + 8, 2);
  w32(a + 12, 112);
  memcpy(a + 56, a + 16, 40);
  w32(a + 40, 96);
  w32(a + 44, 8);
  w32(a + 80, 104);
  w32(a + 84, 8);
  CHECK(!kf_archive_typed(a, 112, "x", 2, &p, &n));
  a[56] = 'y';
  CHECK(kf_archive_typed(a, 112, "x", 2, &p, &n) && p == a + 96);
  w32(a + 80, 100);
  CHECK(!kf_archive_typed(a, 112, "x", 2, &p, &n));
  w32(a + 80, 104);
  w32(a + 40, 95);
  CHECK(!kf_archive_typed(a, 112, "x", 2, &p, &n));
  w32(a + 40, 96);
  memset(a + 16, 'z', 24);
  CHECK(!kf_archive_typed(a, 112, "x", 2, &p, &n));
  puts("KF_PASS:archives:payload56-total64:typed-readonly-data:no-dynamic-"
       "loading:duplicates-overlap-names");
  return 0;
}
static int handles(void) {
  KfHandles owner = {0}, foreign = {0};
  unsigned resource = 999;
  uint32_t token = kf_handle_open(&owner, 0, 42, 3);
  CHECK(token == 65537 && kf_handle_get(&owner, token, 3, &resource) &&
        resource == 42);
  CHECK(!kf_handle_get(&foreign, token, 1, &resource));
  KfHandles before = owner;
  uint64_t invalid[] = {0, 65536, 65553, UINT64_C(0x100010001), token + 1};
  for (unsigned i = 0; i < 5; ++i) {
    resource = 999;
    CHECK(!kf_handle_get(&owner, invalid[i], 1, &resource) && resource == 999 &&
          !kf_handle_close(&owner, invalid[i]) &&
          !memcmp(&owner, &before, sizeof owner));
  }
  uint32_t read = kf_delegate(&owner, token, &foreign, 4, 1);
  CHECK(read == 65541 && kf_handle_get(&foreign, read, 1, &resource) &&
        resource == 42 && !kf_handle_get(&foreign, read, 2, &resource));
  CHECK(!kf_delegate(&foreign, read, &owner, 3, 2));
  CHECK(kf_handle_close(&owner, token));
  uint32_t next = kf_handle_open(&owner, 0, 99, 3);
  CHECK(next == 131073 && !kf_handle_get(&owner, token, 1, &resource));
  CHECK(kf_handle_get(&foreign, read, 1, &resource) &&
        resource == 42); /* immutable resource lifetime is separate from closing
                            another handle */
  CHECK(kf_handle_close(&owner, next));
  owner.slot[0].generation = UINT16_MAX;
  next = kf_handle_open(&owner, 0, 99, 3);
  CHECK(next == 0xffff0001u && kf_handle_close(&owner, next) &&
        owner.slot[0].retired && !kf_handle_open(&owner, 0, 99, 3));
  puts("KF_PASS:handles:65537:high-bits-foreign-stale-generation-retired:"
       "delegated-readonly-no-amplification");
  return 0;
}
static int lines(void) {
  KfLine s = {0};
  char out[64];
  unsigned n = 999;
  memset(out, 0xa5, sizeof out);
  CHECK(kf_line(&s, '\n', out, &n) == 1 && n == 0 && out[0] == 0);
  for (unsigned i = 0; i < 63; ++i)
    CHECK(!kf_line(&s, 'x', out, &n));
  CHECK(kf_line(&s, '\n', out, &n) == 1 && n == 63 && out[63] == 0);
  for (unsigned i = 0; i < 64; ++i)
    CHECK(!kf_line(&s, 'x', out, &n));
  for (unsigned i = 0; i < 4; ++i)
    CHECK(!kf_line(&s, (unsigned char)"exit"[i], out, &n));
  CHECK(kf_line(&s, '\n', out, &n) == 2);
  for (unsigned i = 0; i < 4; ++i)
    CHECK(!kf_line(&s, (unsigned char)"help"[i], out, &n));
  CHECK(kf_line(&s, '\r', out, &n) == 1 && n == 4 && !strcmp(out, "help"));
  CHECK(!kf_line(&s, '\n', out, &n));
  CHECK(!kf_line(&s, 'e', out, &n));
  CHECK(!kf_line(&s, 'c', out, &n));
  CHECK(!kf_line(&s, 'h', out, &n));
  CHECK(!kf_line(&s, 'o', out, &n));
  CHECK(kf_line(&s, '\n', out, &n) == 1 && n == 4);
  CHECK(!kf_line(&s, 'p', out, &n));
  memset(&s, 0, sizeof s);
  CHECK(kf_line(&s, '\n', out, &n) == 1 && n == 0);
  puts("KF_PASS:lines:empty63-overlong-tail-discard-following-help:split-CRLF:"
       "partial-termination");
  return 0;
}
int main(int argc, char **argv) {
  if (argc != 2)
    return 2;
  if (!strcmp(argv[1], "permissions"))
    return permissions();
  if (!strcmp(argv[1], "scheduling"))
    return scheduling();
  if (!strcmp(argv[1], "images"))
    return images();
  if (!strcmp(argv[1], "archives"))
    return archives();
  if (!strcmp(argv[1], "handles"))
    return handles();
  if (!strcmp(argv[1], "lines"))
    return lines();
  return 2;
}
