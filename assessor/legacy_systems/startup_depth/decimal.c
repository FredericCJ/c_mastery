#include "startup.h"

enum sd_status sd_parse_u32(const unsigned char *p, size_t n, uint32_t *out)
{
    if (!out || (!p && n))
        return SD_ARGUMENT;
    if (!n || n > 10)
        return SD_FORMAT;
    uint32_t v = 0;
    for (size_t i = 0; i < n; i++)
    {
        if (p[i] < '0' || p[i] > '9')
            return SD_FORMAT;
        uint32_t digit = p[i] - '0';
        if (v > (UINT32_MAX - digit) / 10)
            return SD_RANGE;
        v = v * 10 + digit;
    }
    *out = v;
    return SD_OK;
}

enum sd_status sd_parse_i32(const unsigned char *p, size_t n, int32_t *out)
{
    if (!out || (!p && n))
        return SD_ARGUMENT;
    int negative = n && p[0] == '-';
    uint32_t magnitude;
    enum sd_status status =
        sd_parse_u32(p ? p + negative : p, n - (size_t)negative, &magnitude);
    if (status != SD_OK)
        return status;
    if (magnitude > (negative ? UINT32_C(2147483648) : INT32_MAX))
        return SD_RANGE;
    *out = negative ? (magnitude == UINT32_C(2147483648) ? INT32_MIN
                                                         : -(int32_t)magnitude)
                    : (int32_t)magnitude;
    return SD_OK;
}

static enum sd_status publish(const unsigned char *temporary, size_t n,
                              unsigned char *out, size_t capacity,
                              int terminated, size_t *length)
{
    if (!out || !length || (terminated != 0 && terminated != 1))
        return SD_ARGUMENT;
    if (n > capacity || (terminated && n == capacity))
        return SD_RANGE;
    for (size_t i = 0; i < n; i++)
        out[i] = temporary[i];
    if (terminated)
        out[n] = 0;
    *length = n;
    return SD_OK;
}

enum sd_status sd_format_u32(uint32_t value, unsigned char *out,
                             size_t capacity, int terminated, size_t *length)
{
    unsigned char reversed[10], forward[10];
    size_t n = 0;
    do
    {
        reversed[n++] = (unsigned char)('0' + value % 10);
        value /= 10;
    } while (value);
    for (size_t i = 0; i < n; i++)
        forward[i] = reversed[n - 1 - i];
    return publish(forward, n, out, capacity, terminated, length);
}

enum sd_status sd_format_u32_nodiv(uint32_t value, unsigned char *out,
                                   size_t capacity, int terminated,
                                   size_t *length)
{
    static const uint32_t powers[] = {1000000000, 100000000, 10000000, 1000000,
                                      100000,     10000,     1000,     100,
                                      10,         1};
    unsigned char bytes[10];
    size_t n = 0;
    int started = 0;
    for (size_t i = 0; i < 10; i++)
    {
        unsigned digit = 0;
        while (value >= powers[i])
        {
            value -= powers[i];
            digit++;
        }
        if (digit || started || i == 9)
        {
            bytes[n++] = (unsigned char)('0' + digit);
            started = 1;
        }
    }
    return publish(bytes, n, out, capacity, terminated, length);
}

enum sd_status sd_format_i32(int32_t value, unsigned char *out, size_t capacity,
                             int terminated, size_t *length)
{
    unsigned char bytes[11];
    size_t n;
    int negative = value < 0;
    uint32_t magnitude = negative ? 0u - (uint32_t)value : (uint32_t)value;
    enum sd_status status =
        sd_format_u32(magnitude, bytes + negative, 10, 0, &n);
    if (status != SD_OK)
        return status;
    if (negative)
        bytes[0] = '-';
    return publish(bytes, n + (size_t)negative, out, capacity, terminated,
                   length);
}
