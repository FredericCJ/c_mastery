/* Supplied ISA adapter. Learners implement the scalar semantic core and dispatch. */
#include "performance.h"
#include <emmintrin.h>

void bf_sse2(const uint32_t *a, const uint32_t *b, uint32_t *out, size_t n)
{
    size_t i = 0;
    for (; n - i >= 4; i += 4)
    {
        __m128i x = _mm_loadu_si128((const __m128i *)(const void *)(a + i));
        __m128i y = _mm_loadu_si128((const __m128i *)(const void *)(b + i));
        _mm_storeu_si128((__m128i *)(void *)(out + i), _mm_add_epi32(x, y));
    }
    for (; i < n; ++i)
    {
        out[i] = a[i] + b[i];
    }
}
