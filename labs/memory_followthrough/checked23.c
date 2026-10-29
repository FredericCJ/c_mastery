/* Optional installed C23-header probe. Authority is explicitly N3096 7.20,
   April2023 draft; this is not a published-edition conformance certificate. */
#include "memory.h"
#include <stdckdint.h>
#include <stdio.h>

static int checked(size_t n, size_t s, size_t h, size_t t, size_t *out)
{
    size_t v;
    if (ckd_mul(&v, n, s) || ckd_add(&v, v, h) || ckd_add(&v, v, t) || !v)
        return 0;
    *out = v;
    return 1;
}

int main(void)
{
    size_t values[] = {
        0, 1, 7, SIZE_MAX / 7, SIZE_MAX / 7 + 1, SIZE_MAX - 1, SIZE_MAX};
    unsigned trials = 0;
    for (unsigned i = 0; i < 7; i++)
        for (unsigned j = 0; j < 7; j++)
            for (unsigned h = 0; h < 2; h++)
            {
                size_t a = 77, b = 77;
                size_t stride = values[j] ? values[j] : 1;
                int x = checked(values[i], stride, h, 1, &a);
                enum mw_status y =
                    mw_size(values[i], stride, h, 1, 1, SIZE_MAX, &b);
                if (x != (y == MW_OK) || a != b)
                    return 1;
                trials++;
            }
    unsigned char narrow = 77;
    int wrapped = ckd_add(&narrow, 255, 1);
    if (!wrapped || narrow != 0)
        return 1;
    printf(
        "MW:C23-header:draft-scoped:cases%u:temporary-preserves:wrapped-result0-on-error:not-C17-baseline\n",
        trials);
    return 0;
}
