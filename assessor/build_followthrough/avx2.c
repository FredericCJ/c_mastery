#include "performance.h"
#include <immintrin.h>

void bf_avx2(const uint32_t *a, const uint32_t *b, uint32_t *out, size_t n)
{
    size_t i = 0;
    for (; n - i >= 8; i += 8)
    {
        __m256i x = _mm256_loadu_si256((const __m256i *)(const void *)(a + i));
        __m256i y = _mm256_loadu_si256((const __m256i *)(const void *)(b + i));
        _mm256_storeu_si256((__m256i *)(void *)(out + i),
                            _mm256_add_epi32(x, y));
    }
    for (; i < n; ++i)
    {
        out[i] = a[i] + b[i];
    }
}
