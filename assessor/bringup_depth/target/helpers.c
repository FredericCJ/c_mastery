#include <stdint.h>
typedef struct {
  unsigned char bytes[257];
} Big;
void wider(Big *d, const Big *s) { *d = *s; }
uint64_t divide(uint64_t x, uint64_t y) { return x / y; }
