#include "follow.h"
#if __BYTE_ORDER__ != __ORDER_BIG_ENDIAN__ || _CALL_ELF != 2
#error This observation requires big-endian PowerPC64 ELFv2.
#endif
extern long sa_raw_write(const char *, size_t);
static char message[160];
static size_t used;

static void append(const char *s)
{
    while (*s)
    {
        message[used++] = *s++;
    }
}

static void number(size_t v)
{
    char digits[24];
    size_t n = 0;
    do
    {
        digits[n++] = (char)('0' + v % 10);
        v /= 10;
    } while (v);
    while (n)
    {
        message[used++] = digits[--n];
    }
}

int main(void)
{
    unsigned char bytes[8];
    struct fd_record a = {3, 258, -2}, b = {0, 0, 0};
    const unsigned char expected[] = {1, 3, 1, 2, 255, 255, 255, 254};
    if (ff_encode(a, bytes, 8) != FD_OK)
    {
        return 1;
    }
    for (size_t i = 0; i < 8; ++i)
    {
        if (bytes[i] != expected[i])
        {
            return 1;
        }
    }
    if (ff_decode(bytes, 8, &b) != FD_OK || b.flags != 3 || b.count != 258 ||
        b.value != -2)
    {
        return 1;
    }
    struct ff_interval interval = {71, 91};
    if (ff_interval(SIZE_MAX, 0, 0, SIZE_MAX, &interval) != FD_OK ||
        interval.begin != SIZE_MAX || interval.end != SIZE_MAX)
    {
        return 1;
    }
    append("FF_BE_PASS size=");
    number(sizeof a);
    append(" align=");
    number(_Alignof(struct fd_record));
    append(" flags=");
    number(offsetof(struct fd_record, flags));
    append(" count=");
    number(offsetof(struct fd_record, count));
    append(" value=");
    number(offsetof(struct fd_record, value));
    append(" repr=");
    const unsigned char *p = (const unsigned char *)&a;
    const char hex[] = "0123456789abcdef";
    for (size_t i = 0; i < sizeof a; ++i)
    {
        message[used++] = hex[p[i] >> 4];
        message[used++] = hex[p[i] & 15];
    }
    append(" wire=01030102fffffffe\n");
    return sa_raw_write(message, used) == (long)used ? 0 : 2;
}
