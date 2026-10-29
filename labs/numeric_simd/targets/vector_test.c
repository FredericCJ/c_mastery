#include "core.h"
#include <stdio.h>
int ns_vector_add(const int16_t *, const int16_t *, size_t, int16_t *);

int main(void)
{
    int16_t a[67], b[67], scalar[67], vector[67];
    size_t n, i;
    uint32_t rng = 7;
    for (n = 0; n <= 67; ++n)
    {
        for (i = 0; i < n; ++i)
        {
            rng = rng * 1664525u + 1013904223u;
            a[i] = (int16_t)((int32_t)(rng % 65536u) - 32768);
            rng = rng * 1664525u + 1013904223u;
            b[i] = (int16_t)((int32_t)(rng % 65536u) - 32768);
        }
        if (!ns_add(a, b, n, scalar) || !ns_vector_add(a, b, n, vector))
            return 1;
        for (i = 0; i < n; ++i)
            if (scalar[i] != vector[i])
                return 2;
    }
    puts("VECTOR_DIFFERENTIAL lengths=68 seed=7");
    return 0;
}
