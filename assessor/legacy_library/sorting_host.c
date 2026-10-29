#define _GNU_SOURCE
#include "sorting.h"
#include <errno.h>
#include <stdio.h>
#include <sys/random.h>

static ptrdiff_t acquire(void *ctx, unsigned char *out, size_t n)
{
    (void)ctx;
    ssize_t r = getrandom(out, n, GRND_NONBLOCK);
    if (r < 0 && errno == EINTR)
        return -2;
    return r < 0 ? -1 : (ptrdiff_t)r;
}

int main(void)
{
    unsigned char synthetic[32];
    enum lr_status r = lr_entropy(acquire, NULL, 8, synthetic);
    if (r != LR_OK)
    {
        fprintf(stderr,
                "SORTING-CHECK:getrandom unavailable status%d errno%d; no rand "
                "fallback\n",
                r, errno);
        return 1;
    }
    puts("SORTING-PASS:32 bytes obtained through Linux getrandom; no "
         "entropy-quality or unpredictability proof from bytes");
    return 0;
}
