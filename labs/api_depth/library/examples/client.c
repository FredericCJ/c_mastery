#include <lc.h>
#include <stdio.h>
int main(void) {
  const unsigned char input[3] = {65, 0, 66};
  unsigned char out[3] = {0};
  size_t n = 0;
  LCBuffer *owned = lc_create(3);
  if (!owned)
    return 2;
  if (lc_append(owned, input, 3) != LC_OK ||
      lc_copy(owned, out, 3, &n) != LC_OK) {
    lc_release_buffer(owned);
    return 3;
  }
  int okay = n == 3 && out[0] == 65 && out[1] == 0 && out[2] == 66;
  lc_release_buffer(owned);
  if (!okay)
    return 4;
  puts("LC_CONSUMER embedded-zero copied and paired release");
  return 0;
}
