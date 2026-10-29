/* GCC/Clang vector extension, explicit and separate from ISO C core. */
#include "core.h"
#include <string.h>
typedef int32_t lanes __attribute__((vector_size(16)));

int ns_vector_add(const int16_t *a, const int16_t *b, size_t n, int16_t *o)
{
    size_t i = 0, j;
    if (n > 65535)
        return 0;
    for (; n - i >= 4; i += 4)
    {
        lanes x = {a[i], a[i + 1], a[i + 2], a[i + 3]};
        lanes y = {b[i], b[i + 1], b[i + 2], b[i + 3]}, s = x + y;
        lanes hi = {32767, 32767, 32767, 32767},
              lo = {-32768, -32768, -32768, -32768};
        lanes gt = s > hi, lt = s < lo;
        s = (gt & hi) | (~gt & s);
        s = (lt & lo) | (~lt & s);
        for (j = 0; j < 4; ++j)
            o[i + j] = (int16_t)s[j];
    }
    for (; i < n; ++i)
    {
        int32_t s = (int32_t)a[i] + b[i];
        if (s > 32767)
            s = 32767;
        if (s < -32768)
            s = -32768;
        o[i] = (int16_t)s;
    }
    return 1;
}
