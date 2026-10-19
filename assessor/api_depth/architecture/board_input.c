#include "board_input.h"
#ifndef AR_MUTANT
#define AR_MUTANT 0
#endif
ArResult ar_f439_read(FHIO bus, unsigned char *p, size_t n) {
  if (!p || !n || n > 8)
    return (ArResult){0, AR_INVALID};
  size_t count = 0;
  while (count < n) {
    uint32_t status;
    int result = f4_uart_rx(bus, p + count, &status);
    if (result < 0)
      return (ArResult){(ptrdiff_t)count, AR_MUTANT == 10 ? AR_OK : AR_FAILED};
    if (!result)
      return (ArResult){(ptrdiff_t)count, count ? AR_OK : AR_PENDING};
    ++count;
  }
  return (ArResult){(ptrdiff_t)count, AR_OK};
}
