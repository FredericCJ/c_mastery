#include "bytes.h"

enum lb_status lb_move(unsigned char *b, size_t size, size_t dst, size_t src,
                       size_t n)
{
    if (dst > size || src > size || n > size - dst || n > size - src)
    {
        return LB_RANGE;
    }
    if (!n)
    {
        return LB_OK;
    }
    if (!b)
    {
        return LB_ARGUMENT;
    }
#ifndef LB_MUTANT_DIRECTION
    if (dst > src)
    {
        for (size_t i = n; i > 0; --i)
        {
            b[dst + i - 1] = b[src + i - 1];
        }
    }
    else
#endif
    {
        for (size_t i = 0; i < n; ++i)
        {
            b[dst + i] = b[src + i];
        }
    }
    return LB_OK;
}

enum lb_status lb_copy(unsigned char *d, size_t cap, const unsigned char *s,
                       size_t length, size_t n)
{
    if (n > cap || n > length)
    {
        return LB_RANGE;
    }
    if (!n)
    {
        return LB_OK;
    }
    if (!d || !s)
    {
        return LB_ARGUMENT;
    }
    for (size_t i = 0; i < n; ++i)
    {
        d[i] = s[i];
    }
    return LB_OK;
}

enum lb_status lb_strlen(const unsigned char *s, size_t extent, size_t *out)
{
    if (!out || (!s && extent))
    {
        return LB_ARGUMENT;
    }
    for (size_t i = 0; i < extent; ++i)
    {
        if (!s[i])
        {
            *out = i;
            return LB_OK;
        }
    }
    return LB_UNTERMINATED;
}

static size_t width(unsigned char c)
{
    return c >= 0x20 && c <= 0x7e && c != '\\' ? 1 : 4;
}

enum lb_status lb_display(const unsigned char *s, size_t n, char *d, size_t cap,
                          size_t *out)
{
    if (!out || (!s && n) || (!d && cap))
    {
        return LB_ARGUMENT;
    }
    size_t required = 0;
    for (size_t i = 0; i < n; ++i)
    {
        size_t w = width(s[i]);
        if (required > SIZE_MAX - w)
        {
            return LB_RANGE;
        }
        required += w;
    }
    if (required == SIZE_MAX || cap <= required)
    {
        return LB_FULL;
    }
    size_t j = 0;
    const char hex[] = "0123456789ABCDEF";
    for (size_t i = 0; i < n; ++i)
    {
        unsigned char c = s[i];
        if (width(c) == 1)
        {
            d[j++] = (char)c;
        }
        else
        {
            d[j++] = '\\';
            d[j++] = 'x';
            d[j++] = hex[c >> 4];
            d[j++] = hex[c & 15];
        }
    }
    d[j] = 0;
    *out = j;
    return LB_OK;
}

enum lb_status lb_field_display(struct f_cursor *cursor, char *d, size_t cap,
                                size_t *out)
{
    if (!cursor)
    {
        return LB_ARGUMENT;
    }
    struct f_cursor trial = *cursor;
    const uint8_t *header, *payload;
    if (f_take(&trial, 2, &header) != F_OK)
    {
        return LB_RANGE;
    }
    size_t n = (size_t)header[0] * 256 + header[1];
    if (f_take(&trial, n, &payload) != F_OK)
    {
        return LB_RANGE;
    }
#ifdef LB_MUTANT_CURSOR
    *cursor = trial;
#endif
    enum lb_status status = lb_display(payload, n, d, cap, out);
    if (status == LB_OK)
    {
        *cursor = trial;
    }
    return status;
}

enum lb_status lb_record(const unsigned char *s, size_t n, unsigned char *d,
                         size_t width_)
{
    if (n > width_)
    {
        return LB_FULL;
    }
    if ((n && !s) || (width_ && !d))
    {
        return LB_ARGUMENT;
    }
    for (size_t i = 0; i < n; ++i)
    {
        d[i] = s[i];
    }
#ifndef LB_MUTANT_PADDING
    for (size_t i = n; i < width_; ++i)
    {
        d[i] = 0;
    }
#endif
    return LB_OK;
}
