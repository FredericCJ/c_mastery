#include "board_input.h"
#ifndef AR_MUTANT
#define AR_MUTANT 0
#endif
struct ar_result ar_f439_read(struct fh_io bus, unsigned char *p, size_t n)
{
    if (!p || !n || n > 8)
        return (struct ar_result){0, AR_INVALID};
    size_t count = 0;
    while (count < n)
    {
        uint32_t status;
        int result = f4_uart_rx(bus, p + count, &status);
        if (result < 0)
            return (struct ar_result){(ptrdiff_t)count,
                                      AR_MUTANT == 10 ? AR_OK : AR_FAILED};
        if (!result)
            return (struct ar_result){(ptrdiff_t)count,
                                      count ? AR_OK : AR_PENDING};
        ++count;
    }
    return (struct ar_result){(ptrdiff_t)count, AR_OK};
}
