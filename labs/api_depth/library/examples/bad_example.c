#include <lc.h>
int main(void) {
  LCBuffer *p = lc_create(1);
  if (!p)
    return 2;
  unsigned char x[2] = {1, 2};
  (void)lc_append(p, x, 2);
  size_t n = 0;
  lc_view(p, &n);
  lc_release_buffer(p);
  return n == 2 ? 0 : 17;
}
