#include "mmio.h"
uint32_t pm_target_read(const struct pm_gpio *p) { return p->idr; }
int pm_target_write(struct pm_gpio *p, unsigned set, unsigned clear) {
  if (!p || set > 65535 || clear > 65535 || (set & clear))
    return -1;
  p->bsrr = set | (clear << 16);
  return 0;
}
