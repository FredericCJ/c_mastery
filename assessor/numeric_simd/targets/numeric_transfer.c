#include "numeric_transfer.h"
#include "numeric_extension.h"
#include <pthread.h>
#include <sched.h>
#include <math.h>
#include <string.h>
#if defined(__x86_64__)
#include <emmintrin.h>
#endif
#ifndef NT_MUTANT
#define NT_MUTANT 0
#endif
int nt_dot(const int16_t *a, const int16_t *b, size_t n, int64_t *out,
           unsigned mode)
{
    if (!out || n > 4096 || mode > 1 || (n && (!a || !b)))
        return 0;
    size_t i = 0;
    int64_t sum = 0;
    if (mode)
    {
#if defined(__x86_64__)
        __m128i accum = _mm_setzero_si128();
        for (; i + 8 <= n; i += 8)
        {
            __m128i x = _mm_loadu_si128((const __m128i *)(const void *)(a + i)),
                    y = _mm_loadu_si128((const __m128i *)(const void *)(b + i));
            __m128i lo = _mm_mullo_epi16(x, y), hi = _mm_mulhi_epi16(x, y);
            __m128i p[2] = {_mm_unpacklo_epi16(lo, hi),
                            _mm_unpackhi_epi16(lo, hi)};
            for (unsigned j = 0; j < 2; j++)
            {
                __m128i sign = _mm_srai_epi32(p[j], 31);
                accum = _mm_add_epi64(accum, _mm_unpacklo_epi32(p[j], sign));
                accum = _mm_add_epi64(accum, _mm_unpackhi_epi32(p[j], sign));
            }
        }
        int64_t lanes[2];
        _mm_storeu_si128((__m128i *)(void *)lanes, accum);
        sum = lanes[0] + lanes[1];
#else
        return 0;
#endif
    }
    if (NT_MUTANT != 1)
        for (; i < n; i++)
            sum += (int32_t)a[i] * b[i];
    *out = sum;
    return 1;
}

struct ns_job
{
    const double *p;
    size_t n;
    double out;
    unsigned yield;
    int ok;
};

static void *worker(void *v)
{
    struct ns_job *j = v;
    if (j->yield)
        for (unsigned i = 0; i < 17; i++)
            sched_yield();
    j->ok = nd_pairwise(j->p, j->n, &j->out);
    return NULL;
}

int nt_parallel(const double *a, size_t n, double *out, unsigned schedule)
{
    if (!out || n > 257 || schedule > 1 || (n && !a))
        return 0;
    for (size_t i = 0; i < n; i++)
        if (!isfinite(a[i]) || fabs(a[i]) > 1e100)
            return 0;
    if (n < 2)
        return nd_pairwise(a, n, out);
    size_t m = n / 2;
    struct ns_job x = {a, m, 0, schedule, 0},
                  y = {a + m, n - m, 0, !schedule, 0};
    pthread_t t, u;
    if (pthread_create(&t, NULL, worker, &x))
        return 0;
    if (pthread_create(&u, NULL, worker, &y))
    {
        pthread_join(t, NULL);
        return 0;
    }
    pthread_join(t, NULL);
    pthread_join(u, NULL);
    if (!x.ok || !y.ok)
        return 0;
    *out = NT_MUTANT == 2 ? y.out : x.out + y.out;
    return 1;
}

int nt_ulp(double a, double b, uint64_t *out)
{
    if (!out || !isfinite(a) || !isfinite(b))
        return 0;
    uint64_t x, y;
    memcpy(&x, &a, 8);
    memcpy(&y, &b, 8);
    uint64_t sign = UINT64_C(1) << 63;
    x = x & sign ? sign - (x & ~sign) : sign + x;
    y = y & sign ? sign - (y & ~sign) : sign + y;
    *out = x > y ? x - y : y - x;
    return 1;
}

int nt_close(double a, double b, double abs, double rel)
{
    if (!isfinite(a) || !isfinite(b) || fabs(a) > 1e100 || fabs(b) > 1e100 ||
        !(abs >= 0 && abs <= 1 && rel >= 0 && rel <= 1))
        return 0;
    double diff = fabs(a - b), scale = fmax(fabs(a), fabs(b));
    return diff <= abs || diff <= rel * scale;
}
