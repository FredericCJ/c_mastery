#include "algorithms.h"
#ifndef AD_MUTANT
#define AD_MUTANT 0
#endif
static int input(const unsigned char *p, size_t n, size_t bound)
{
    return n <= bound && (!n || p);
}

enum ad_status ad_rle_encode(const unsigned char *p, size_t n,
                             unsigned char *out, size_t capacity, size_t *count)
{
    if (!input(p, n, 1024) || !count)
    {
        return AD_INVALID;
    }
    size_t need = 0;
    for (size_t i = 0; i < n;)
    {
        size_t run = 1;
        while (run < 255 && run < n - i && p[i + run] == p[i])
        {
            ++run;
        }
        need += 2;
        i += run;
    }
    if (need > capacity || (need && !out))
    {
        return AD_SPACE;
    }
    size_t at = 0;
    for (size_t i = 0; i < n;)
    {
        size_t run = 1;
        while (run < 255 && run < n - i && p[i + run] == p[i])
        {
            ++run;
        }
        out[at++] = (unsigned char)run;
        out[at++] = p[i];
        i += run;
    }
    *count = at;
    return AD_OK;
}

enum ad_status ad_rle_decode(const unsigned char *p, size_t n,
                             unsigned canonical, unsigned char *out,
                             size_t capacity, size_t *count)
{
    if (!input(p, n, 2048) || !count || canonical > 1 || n % 2)
    {
        return AD_INVALID;
    }
    size_t need = 0;
    for (size_t i = 0; i < n; i += 2)
    {
        unsigned run = p[i];
        if (!run)
        {
            return AD_INVALID;
        }
        if (canonical && AD_MUTANT != 9 && i && p[i + 1] == p[i - 1] &&
            p[i - 2] != 255)
        {
            return AD_INVALID;
        }
        if (run > 1024 - need)
        {
            return AD_OVERFLOW;
        }
        need += run;
    }
    if (need > capacity || (need && !out))
    {
        return AD_SPACE;
    }
    size_t at = 0;
    for (size_t i = 0; i < n; i += 2)
    {
        for (unsigned j = 0; j < p[i]; ++j)
        {
            out[at++] = p[i + 1];
        }
    }
    *count = at;
    return AD_OK;
}

/* Each tag has positive count1..127; high bit chooses repeated byte or literal span. */
static size_t same_run(const unsigned char *p, size_t n, size_t start)
{
    size_t run = 1;
    while (run < 127 && run < n - start && p[start + run] == p[start])
    {
        ++run;
    }
    return run;
}

enum ad_status ad_tagged_encode(const unsigned char *p, size_t n,
                                unsigned char *out, size_t capacity,
                                size_t *count)
{
    if (!input(p, n, 1024) || !count)
    {
        return AD_INVALID;
    }
    unsigned char encoded[2048];
    size_t at = 0, i = 0;
    while (i < n)
    {
        size_t run = same_run(p, n, i);
        if (run >= 3)
        {
            encoded[at++] = (unsigned char)(128u + run);
            encoded[at++] = p[i];
            i += run;
        }
        else
        {
            size_t begin = i;
            i += run;
            while (i < n && i - begin < 127)
            {
                run = same_run(p, n, i);
                if (run >= 3)
                {
                    break;
                }
                size_t room = 127 - (i - begin);
                i += run < room ? run : room;
            }
            encoded[at++] = (unsigned char)(i - begin);
            for (size_t j = begin; j < i; ++j)
            {
                encoded[at++] = p[j];
            }
        }
    }
    if (at > capacity || (at && !out))
    {
        return AD_SPACE;
    }
    for (size_t j = 0; j < at; ++j)
    {
        out[j] = encoded[j];
    }
    *count = at;
    return AD_OK;
}

enum ad_status ad_tagged_decode(const unsigned char *p, size_t n,
                                unsigned char *out, size_t capacity,
                                size_t *count)
{
    if (!input(p, n, 2048) || !count)
    {
        return AD_INVALID;
    }
    size_t i = 0, need = 0;
    while (i < n)
    {
        unsigned tag = p[i++], run = tag & 127u;
        if (!run)
        {
            return AD_INVALID;
        }
        size_t stored = (tag & 128u) ? 1 : run;
        if (stored > n - i)
        {
            return AD_INCOMPLETE;
        }
        if (run > 1024 - need)
        {
            return AD_OVERFLOW;
        }
        need += run;
        i += stored;
    }
    if (need > capacity || (need && !out))
    {
        return AD_SPACE;
    }
    i = 0;
    size_t at = 0;
    while (i < n)
    {
        unsigned tag = p[i++], run = tag & 127u;
        if (tag & 128u)
        {
            for (unsigned j = 0; j < run; ++j)
            {
                out[at++] = p[i];
            }
            ++i;
        }
        else
        {
            for (unsigned j = 0; j < run; ++j)
            {
                out[at++] = p[i++];
            }
        }
    }
    *count = at;
    return AD_OK;
}

enum ad_status ad_rle_stream_init(struct ad_rle_stream *s, unsigned canonical)
{
    if (!s || canonical > 1)
    {
        return AD_INVALID;
    }
    *s = (struct ad_rle_stream){.canonical = canonical};
    return AD_OK;
}

enum ad_status ad_rle_stream_feed(struct ad_rle_stream *s,
                                  const unsigned char *p, size_t n,
                                  size_t *consumed, unsigned char *out,
                                  size_t capacity, size_t *written)
{
    if (!s || !consumed || !written || !input(p, n, 2048) ||
        (capacity && !out) || capacity > 1024)
    {
        return AD_INVALID;
    }
    *consumed = *written = 0;
    if (s->failed)
    {
        return AD_FAILED;
    }
    for (;;)
    {
        while (s->remaining && *written < capacity)
        {
            out[(*written)++] = s->byte;
            --s->remaining;
            ++s->produced;
        }
        if (s->remaining)
        {
            return AD_SPACE;
        }
        if (*consumed == n)
        {
            return s->waiting ? AD_INCOMPLETE : AD_OK;
        }
        if (!s->waiting)
        {
            unsigned run = p[(*consumed)++];
            if (!run)
            {
                s->failed = 1;
                return AD_INVALID;
            }
            if (run > 1024 - s->produced)
            {
                s->failed = 1;
                return AD_OVERFLOW;
            }
            s->run = run;
            s->waiting = 1;
        }
        else
        {
            unsigned byte = p[(*consumed)++];
            if (s->canonical && s->have_last && byte == s->last_value &&
                s->last_count != 255)
            {
                s->failed = 1;
                return AD_INVALID;
            }
            s->byte = (unsigned char)byte;
            s->remaining = s->run;
            s->waiting = 0;
            s->last_count = s->run;
            s->last_value = byte;
            s->have_last = 1;
        }
    }
}

enum ad_status ad_rle_stream_end(struct ad_rle_stream *s)
{
    if (!s)
    {
        return AD_INVALID;
    }
    if (s->failed)
    {
        return AD_FAILED;
    }
    if (s->waiting || s->remaining)
    {
        return AD_INCOMPLETE;
    }
    return AD_OK;
}
