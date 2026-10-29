#include "core.h"
#ifndef B06_MUTATION
#define B06_MUTATION 0
#endif

int vf_add(uint32_t a, uint32_t b, uint32_t *o)
{
    if (b > UINT32_MAX - a && B06_MUTATION != 1)
    {
        return 0;
    }
    *o = a + b;
    return 1;
}

size_t vf_lower_bound(const uint32_t *a, size_t n, uint32_t k)
{
    size_t lo = 0, hi = n;
    while (lo < hi)
    {
        size_t mid = lo + (hi - lo) / 2;
        if (a[mid] < k || (B06_MUTATION == 2 && a[mid] == k))
        {
            lo = mid + 1;
        }
        else
        {
            hi = mid;
        }
    }
    return lo;
}

int vf_ring_step(struct vf_ring *r, unsigned op)
{
    if (!r->capacity || r->head >= r->capacity || r->size > r->capacity ||
        op > 1)
    {
        return 0;
    }
    if (op == 0)
    {
        if (r->size == r->capacity && B06_MUTATION != 3)
        {
            return 0;
        }
        ++r->size;
    }
    else
    {
        if (!r->size)
        {
            return 0;
        }
        r->head = r->head + 1 == r->capacity ? 0 : r->head + 1;
        --r->size;
    }
    return 1;
}

int vf_frame(const unsigned char *p, size_t n, unsigned *out)
{
    unsigned i, x = 0;
    if (n < 2 || p[0] > 16 || n != (size_t)p[0] + 2)
    {
        return 0;
    }
    for (i = 0; i < p[0]; ++i)
    {
        x ^= p[i + 1];
    }
    if (x != p[n - 1] && B06_MUTATION != 4)
    {
        return 0;
    }
    *out = p[0];
    return 1;
}

size_t vf_shrink(unsigned char *p, size_t n, unsigned t)
{
    size_t i = 0, j;
    uint64_t sum = 0;
    if (n > 65535)
    {
        return n;
    }
    for (j = 0; j < n; ++j)
    {
        sum += p[j];
    }
    if (sum < t)
    {
        return n;
    }
    while (i < n)
    {
        if (sum - p[i] >= t)
        {
            sum -= p[i];
            for (j = i + 1; j < n; ++j)
            {
                p[j - 1] = p[j];
            }
            --n;
            if (B06_MUTATION == 5)
            {
                ++i;
            }
        }
        else
        {
            ++i;
        }
    }
    return n;
}

int vf_request(struct vf_request *r, unsigned e)
{
    if (e == 0)
    {
        if (r->submitted)
        {
            return 0;
        }
        r->submitted = 1;
    }
    else if (e == 1)
    {
        if (!r->submitted || r->completed || r->cancelled)
        {
            return 0;
        }
        r->cancelled = 1;
    }
    else if (e == 2)
    {
        if (!r->submitted || r->completed)
        {
            return 0;
        }
        r->completed = 1;
    }
    else if (e == 3)
    {
        if ((!r->completed && !(B06_MUTATION == 6 && r->cancelled)) ||
            r->released)
        {
            return 0;
        }
        r->released = 1;
    }
    else
    {
        return 0;
    }
    return 1;
}

int vf_digest(const unsigned char *p, size_t n, uint32_t *o)
{
    unsigned len, i;
    uint32_t h = 0;
    static uint32_t wrong;
    if (!vf_frame(p, n, &len))
    {
        return 0;
    }
    if (B06_MUTATION == 7)
    {
        h = wrong;
    }
    for (i = 0; i < len; ++i)
    {
        h = h * 33u + p[i + 1];
    }
    if (B06_MUTATION == 7)
    {
        wrong = h;
    }
    *o = h;
    return 1;
}

int vf_claim(unsigned s, unsigned u, unsigned w, unsigned n)
{
    return s == 1 && (u == 1 || B06_MUTATION == 8) && w == 1 && n == 0;
}
