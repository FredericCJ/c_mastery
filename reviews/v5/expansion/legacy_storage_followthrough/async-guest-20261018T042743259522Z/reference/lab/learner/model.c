#include "cmk_contract.h"
/* Implement these contracts in PR-T48-02, 04, 07 and 08. A compiling stub
 * deliberately fails behavioral tests; no implemented status is implied. */
int cmk_boot_parse(const void *p, size_t n, struct cmk_boot *b) {
  (void)p;
  (void)n;
  (void)b;
  return -1;
}
int cmk_elf_parse(const void *p, size_t n, struct cmk_elf *e) {
  (void)p;
  (void)n;
  (void)e;
  return -1;
}
int cmk_archive_find(const void *p, size_t n, const char *s,
                     const unsigned char **r, size_t *z) {
  (void)p;
  (void)n;
  (void)s;
  (void)r;
  (void)z;
  return -1;
}
void cmk_frames_init(struct cmk_frames *f, const struct cmk_boot *b,
                     uint32_t r) {
  (void)f;
  (void)b;
  (void)r;
}
uint32_t cmk_frame_take(struct cmk_frames *f) {
  (void)f;
  return 0;
}
int cmk_frame_put(struct cmk_frames *f, uint32_t p) {
  (void)f;
  (void)p;
  return -1;
}
int cmk_user_range(uint64_t p, uint64_t n) {
  (void)p;
  (void)n;
  return 0;
}
uint32_t cmk_handle(unsigned s, unsigned g) {
  (void)s;
  (void)g;
  return 0;
}
int cmk_handle_valid(uint32_t h, unsigned s, unsigned g, int o) {
  (void)h;
  (void)s;
  (void)g;
  (void)o;
  return 0;
}
