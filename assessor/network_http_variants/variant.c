#include "variant.h"
#include "systems.h"
#ifndef NV_MUTATION
#define NV_MUTATION 0
#endif
int nv_sum(const unsigned char *p, size_t n, struct nv_sum *out)
{
    if (!out || (!p && n))
        return NV_INVALID;
    struct s_checksum s = {0, 0, 0};
    uint16_t value;
    if (s_checksum_add(&s, p, n) != S_OK ||
        s_checksum_finish(&s, &value) != S_OK)
        return NV_INVALID;
    *out = (struct nv_sum){(uint16_t)~value, (unsigned)(n % 2)};
    return NV_OK;
}

int nv_combine(struct nv_sum a, struct nv_sum b, struct nv_sum *out)
{
    if (!out || a.odd > 1 || b.odd > 1)
        return NV_INVALID;
    uint32_t right = b.sum;
    if (a.odd && NV_MUTATION != 1)
        right = ((right & 255u) << 8) | (right >> 8);
    uint32_t sum = (uint32_t)a.sum + right;
    sum = (sum & 65535u) + (sum >> 16);
    if (NV_MUTATION == 2)
        sum = (uint32_t)a.sum + right;
    *out = (struct nv_sum){(uint16_t)sum, a.odd ^ b.odd};
    return NV_OK;
}

int nv_length(const unsigned char *s, size_t n, uint32_t maximum, uint32_t cap,
              uint32_t *out)
{
    if (!s || !n || !out || cap > 1024)
        return 400;
    uint32_t value = 0;
    for (size_t i = 0; i < n; i++)
    {
        if (s[i] < '0' || s[i] > '9')
            return 400;
        unsigned digit = (unsigned)(s[i] - '0');
        if (NV_MUTATION == 3)
        {
            if (value > (maximum - digit) / 10u)
                return 400;
        }
        else if (value > maximum / 10u ||
                 (value == maximum / 10u && digit > maximum % 10u))
            return 400;
        value = value * 10u + digit;
        if (NV_MUTATION == 4 && value > cap)
            return 413;
    }
    if (value > cap)
        return 413;
    *out = value;
    return NV_OK;
}

static void two(char *p, unsigned n)
{
    p[0] = (char)('0' + n / 10);
    p[1] = (char)('0' + n % 10);
}

int nv_date(unsigned available, uint32_t days, uint32_t seconds, char *out,
            size_t cap, size_t *length)
{
    if (!length || available > 1)
        return NV_INVALID;
    if (!available)
    {
        *length = 0;
        return NV_OK;
    }
    if (!out || cap < 30 || days > 2932896u || seconds > 86399u)
        return NV_INVALID;
    /* Days before 1970 are 1969*365+1969/4-1969/100+1969/400=719162.
       Each era starts on Jan 1 of year 1 mod400, so its last year is leap. */
    uint32_t d = days + 719162u, era = d / 146097u;
    d %= 146097u;
    unsigned century = d / 36524u;
    if (century == 4 && NV_MUTATION != 5)
        century = 3;
    d -= century * 36524u;
    unsigned quad = d / 1461u;
    d %= 1461u;
    unsigned year = d / 365u;
    if (year == 4)
        year = 3;
    d -= year * 365u;
    unsigned y =
        (unsigned)(era * 400u) + century * 100u + quad * 4u + year + 1u;
    unsigned leap = y % 4 == 0 && (y % 100 != 0 || y % 400 == 0), m = 0;
    static const unsigned lengths[12] = {31, 28, 31, 30, 31, 30,
                                         31, 31, 30, 31, 30, 31};
    while (d >= lengths[m] + (m == 1 ? leap : 0))
    {
        d -= lengths[m] + (m == 1 ? leap : 0);
        m++;
    }
    static const char weekdays[] = "SunMonTueWedThuFriSat",
                      months[] = "JanFebMarAprMayJunJulAugSepOctNovDec";
    char value[30] = "Thu, 01 Jan 1970 00:00:00 GMT";
    unsigned weekday = (days + 4u) % 7u;
    for (unsigned i = 0; i < 3; i++)
    {
        value[i] = weekdays[weekday * 3 + i];
        value[8 + i] = months[m * 3 + i];
    }
    two(value + 5, (unsigned)d + 1);
    two(value + 12, y / 100);
    two(value + 14, y % 100);
    two(value + 17, seconds / 3600);
    two(value + 20, (seconds / 60) % 60);
    two(value + 23, seconds % 60);
    for (unsigned i = 0; i < 30; i++)
        out[i] = value[i];
    *length = 29;
    return NV_OK;
}

int nv_stamp(struct nv_stamp s, uint32_t leap_day, char *out, size_t cap,
             size_t *length)
{
    if (!length || s.available > 1 || s.trusted > 1)
        return NV_INVALID;
    if (!s.available || !s.trusted)
    {
        *length = 0;
        return NV_OK;
    }
    if (s.scale == NV_ORDINARY_UTC)
        return nv_date(1, s.days, s.seconds, out, cap, length);
    if (s.scale != NV_POSITIVE_LEAP || s.seconds != 86400u ||
        (s.days != leap_day && NV_MUTATION != 6))
        return NV_INVALID;
    char value[30];
    size_t n;
    if (nv_date(1, s.days, 86399, value, sizeof value, &n) != NV_OK || !out ||
        cap < 30)
        return NV_INVALID;
    value[23] = '6';
    value[24] = '0';
    for (unsigned i = 0; i < 30; i++)
        out[i] = value[i];
    *length = n;
    return NV_OK;
}
