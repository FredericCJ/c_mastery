#include "api.h"

uint64_t ad_sum64(const uint32_t *p, size_t n)
{
    uint64_t result = 0;
    for (size_t i = 0; i < n; i++)
        result += p[i];
    return result;
}

uint32_t ad_sum32(const uint32_t *p, size_t n)
{
    uint32_t result = 0;
    for (size_t i = 0; i < n; i++)
        result += p[i];
    return result;
}

struct ad_pair ad_pair(uint64_t a, uint64_t b)
{
    return (struct ad_pair){a, b};
}

struct ad_triple ad_triple(uint64_t a, uint64_t b, uint64_t c)
{
    return (struct ad_triple){a, b, c};
}

struct ad_mixed ad_mixed(double r, uint64_t i)
{
    return (struct ad_mixed){r + 0.5, i + 1};
}

uint64_t ad_nine(uint64_t a, uint64_t b, uint64_t c, uint64_t d, uint64_t e,
                 uint64_t f, uint64_t g, uint64_t h, uint64_t i, double r)
{
    return a + 2 * b + 3 * c + 4 * d + 5 * e + 6 * f + 7 * g + 8 * h + 9 * i +
           (uint64_t)r;
}

uint32_t ad_inline_add(uint32_t a, uint32_t b)
{
    uint32_t scratch;
#ifdef AD_MUTANT_EARLY
    /* A concrete allocation admitted by the missing early-clobber contract: the
     * scratch register aliases the still-unconsumed b input. Not a claim that
     * every compiler/version selects this allocation for the original C block. */
    __asm__ volatile("mov %[a],%[b]\n\tadd %[b],%[a]"
                     : [a] "+r"(a), [b] "+r"(b)::"cc");
    scratch = b;
#else
    __asm__ volatile("mov %[a],%[t]\n\tadd %[b],%[a]"
                     : [a] "+r"(a), [t] "=&r"(scratch)
                     : [b] "r"(b)
                     : "cc");
#endif
    (void)scratch;
    return a;
}

uint32_t ad_named_memory(uint32_t *p)
{
    __asm__ volatile("addl $1,%0" : "+m"(*p)::"cc");
    return *p;
}

uint32_t ad_opaque_memory(uint32_t *p)
{
    __asm__ volatile("addl $1,(%0)" ::"r"(p) : "cc", "memory");
    return *p;
}
