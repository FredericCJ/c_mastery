#include "performance.h"
#include <cpuid.h>
#include <stdio.h>
void bf_avx2(const uint32_t *, const uint32_t *, uint32_t *, size_t);

int main(void)
{
    unsigned eax, ebx, ecx, edx, lo, hi;
    if (!__get_cpuid(1, &eax, &ebx, &ecx, &edx) || (ecx & bit_OSXSAVE) == 0 ||
        (ecx & bit_AVX) == 0)
    {
        puts("AVX2_UNAVAILABLE:CPUID/OSXSAVE");
        return 77;
    }
    __asm__ volatile("xgetbv" : "=a"(lo), "=d"(hi) : "c"(0));
    if ((lo & 6) != 6 || !__get_cpuid_count(7, 0, &eax, &ebx, &ecx, &edx) ||
        (ebx & bit_AVX2) == 0)
    {
        puts("AVX2_UNAVAILABLE:XCR0/AVX2");
        return 77;
    }
    printf("AVX2_OS_STATE:xcr0=%u:%u\n", hi, lo);
    uint32_t a[25], b[25], o[25];
    for (size_t i = 0; i < 25; ++i)
    {
        a[i] = UINT32_MAX;
        b[i] = (uint32_t)i;
    }
    for (size_t n = 0; n <= 17; ++n)
        for (size_t offset = 0; offset < 8; ++offset)
        {
            for (size_t i = 0; i < 25; ++i)
                o[i] = 123;
            bf_avx2(a + offset, b + offset, o + offset, n);
            for (size_t i = 0; i < 25; ++i)
                if (o[i] !=
                    (i >= offset && i < offset + n ? (uint32_t)(i - 1) : 123))
                {
                    puts("AVX2_CHECK:extent-tail");
                    return 1;
                }
        }
    puts(
        "AVX2_PASS:lengths0..17 offsets0..28; OSXSAVE XCR0 and AVX2 checked before call");
    return 0;
}
