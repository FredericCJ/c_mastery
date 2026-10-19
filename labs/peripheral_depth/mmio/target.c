#include "mmio.h"
uint32_t pm_target_read(const PmGpio *p) {
  (void)p;
  return 0;
}
int pm_target_write(PmGpio *p, unsigned s, unsigned c) {
  (void)p;
  (void)s;
  (void)c;
  return -1;
}
