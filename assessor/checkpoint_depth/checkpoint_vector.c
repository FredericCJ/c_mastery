#include "checkpoint_vector.h"
#ifndef CP_MUTANT
#define CP_MUTANT 0
#endif
#if defined(__x86_64__) && (defined(__GNUC__) || defined(__clang__))
#include <cpuid.h>
#include <immintrin.h>
#define CPV_X86 1

static int leaf1(void *context, uint32_t *ecx)
{
    unsigned a, b, c, d;
    (void)context;
    if (!__get_cpuid(1, &a, &b, &c, &d))
        return 0;
    *ecx = c;
    return 1;
}

static uint64_t xgetbv0(void *context)
{
    uint32_t low, high;
    (void)context;
    __asm__ volatile("xgetbv" : "=a"(low), "=d"(high) : "c"(0));
    return ((uint64_t)high << 32) | low;
}

__attribute__((target("avx"), noinline)) static void
vector_kernel(const double *restrict history, size_t n, double *restrict out)
{
    const __m256d quarter = _mm256_set1_pd(0.25), half = _mm256_set1_pd(0.5);
    size_t i = 0;
    for (; n - i >= 4; i += 4)
    {
        __m256d x = _mm256_loadu_pd(history + i + 2),
                prev = _mm256_loadu_pd(history + i + 1),
                older = _mm256_loadu_pd(history + i);
        __m256d first =
            _mm256_add_pd(_mm256_mul_pd(x, quarter), _mm256_mul_pd(prev, half));
        _mm256_storeu_pd(out + i,
                         _mm256_add_pd(first, _mm256_mul_pd(older, quarter)));
    }
    if (CP_MUTANT != 15)
        for (; i < n; ++i)
            out[i] = (0.25 * history[i + 2] + 0.5 * history[i + 1]) +
                     0.25 * history[i];
}
#else
#define CPV_X86 0
#endif
int cpv_detect(struct cpv_probe p)
{
    uint32_t flags = 0;
    const uint32_t avx = UINT32_C(1) << 28, os = UINT32_C(1) << 27;
    if (!p.leaf1 || !p.xgetbv0 || !p.leaf1(p.context, &flags))
        return 0;
    if (CP_MUTANT == 13)
        return (flags & avx) != 0;
    if ((flags & (avx | os)) != (avx | os))
        return 0;
    uint64_t state = p.xgetbv0(p.context);
    return CP_MUTANT == 14 ? (state & 2) == 2 : (state & 6) == 6;
}

int cpv_available(void)
{
#if CPV_X86
    struct cpv_probe p = {0, leaf1, xgetbv0};
    return cpv_detect(p);
#else
    return 0;
#endif
}

enum cpn_status cpv_plan(struct cpv_plan *p, enum cpv_backend requested)
{
    if (!p || requested < CPV_SCALAR || requested > CPV_COMPILER)
        return CPN_INVALID;
    if (requested == CPV_AUTO || requested == CPV_AVX)
    {
        int supported = cpv_available();
        if (requested == CPV_AVX && !supported)
            return CPN_UNSUPPORTED;
        requested = supported ? CPV_AVX : CPV_SCALAR;
    }
    p->backend = requested;
    return CPN_OK;
}

/* Staging supplies a truthful disjoint restrict contract, even for overlapping public extents. */
#if defined(__GNUC__) || defined(__clang__)
__attribute__((noinline))
#endif
static void compiler_kernel(const double *restrict history, size_t n,
                            double *restrict out)
{
    for (size_t i = 0; i < n; ++i)
        out[i] =
            (0.25 * history[i + 2] + 0.5 * history[i + 1]) + 0.25 * history[i];
}
#if defined(__GNUC__) && !defined(__clang__)
__attribute__((noinline, optimize("no-tree-vectorize")))
#elif defined(__clang__)
__attribute__((noinline))
#endif
static void scalar_kernel(const double *history, size_t n, double *out)
{
#if defined(__clang__)
#pragma clang loop vectorize(disable) interleave(disable)
#endif
    for (size_t i = 0; i < n; ++i)
        out[i] =
            (0.25 * history[i + 2] + 0.5 * history[i + 1]) + 0.25 * history[i];
}

enum cpn_status cpv_run(const struct cpv_plan *plan, struct cpn_double *s,
                        const double *in, size_t n, double *out,
                        size_t capacity)
{
    if (!plan || (plan->backend != CPV_SCALAR && plan->backend != CPV_AVX &&
                  plan->backend != CPV_COMPILER))
        return CPN_INVALID;
    if (!s || n > CPN_BLOCK || (n && (!in || !out)))
        return CPN_INVALID;
    if (capacity < n)
        return CPN_CAPACITY;
    if (cpn_double(s, 0, 0, 0, 0) != CPN_OK)
        return CPN_INVALID;
    double history[CPN_BLOCK + 2];
    history[0] = s->older;
    history[1] = s->previous;
    for (size_t i = 0; i < n; ++i)
    {
        double x = in[i];
        if (!(x == x && x >= -1e100 && x <= 1e100))
            return CPN_INVALID;
        history[i + 2] = x;
    }
    if (!n)
        return CPN_OK;
    if (plan->backend == CPV_AVX)
    {
#if CPV_X86
        vector_kernel(history, n, out);
#else
        return CPN_UNSUPPORTED;
#endif
    }
    else if (plan->backend == CPV_COMPILER)
        compiler_kernel(history, n, out);
    else
        scalar_kernel(history, n, out);
    s->older = history[n];
    s->previous = CP_MUTANT == 16 ? out[n - 1] : history[n + 1];
    return CPN_OK;
}
