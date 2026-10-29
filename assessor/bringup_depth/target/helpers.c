#include <stdint.h>
struct bu_big {
  unsigned char bytes[257];
};
void wider(struct bu_big *d, const struct bu_big *s) { *d = *s; }
uint64_t divide(uint64_t x, uint64_t y) { return x / y; }
