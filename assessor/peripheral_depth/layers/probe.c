#include "service.h"
#include <stdio.h>
int main(void) {
  for (unsigned a = 0; a < 256; a++)
    for (unsigned b = 0; b < 256; b++) {
      uint8_t out[8];
      if (pl_encode((uint8_t)a, (uint8_t)b, out))
        return 1;
      printf("F %u %u", a, b);
      for (unsigned k = 0; k < 8; k++)
        printf(" %u", (unsigned)out[k]);
      puts("");
    }
  return 0;
}
