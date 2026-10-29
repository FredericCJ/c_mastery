#include "ecc.h"
#include <limits.h>
_Static_assert(CHAR_BIT == 8 && UINT16_MAX == 65535,
               "Selected octet/16-bit profile");
#ifndef EC_MUTATION
#define EC_MUTATION 0
#endif
static unsigned k(unsigned n)
{
    return n == 8 ? 4u : n == 16 ? 11u : 0u;
}

static unsigned count(uint16_t x)
{
    unsigned c = 0;
    for (unsigned i = 0; i < 16; ++i)
        c += (unsigned)((x >> i) & 1u);
    return c;
}

static uint16_t mask(unsigned n)
{
    return n == 8 ? UINT16_C(255) : UINT16_MAX;
}

static uint16_t payload(unsigned n, uint16_t word)
{
    uint16_t data = 0;
    unsigned j = 0;
    for (unsigned p = 1; p < n; ++p)
        if ((p & (p - 1)) != 0)
        {
            data |= (uint16_t)(((word >> (p - 1)) & 1u) << j);
            ++j;
        }
    return data;
}

enum ec_status ec_encode(unsigned n, uint16_t data, uint16_t *out)
{
    unsigned width = k(n);
    if (!out || !width || data >= (1u << width))
        return EC_INVALID;
    uint16_t word = 0;
    unsigned j = 0, syndrome = 0;
    for (unsigned p = 1; p < n; ++p)
        if ((p & (p - 1)) != 0)
        {
            if ((data >> j) & 1u)
            {
                word |= (uint16_t)(1u << (p - 1));
                syndrome ^= p;
            }
            ++j;
        }
    for (unsigned p = 1; p < n; p *= 2)
        if (syndrome & p)
            word |= (uint16_t)(1u << (p - 1));
#if EC_MUTATION != 1
    if (count(word) & 1u)
        word |= (uint16_t)(1u << (n - 1));
#endif
    *out = word;
    return EC_OK;
}

enum ec_status ec_decode(unsigned n, uint16_t received, struct ec_result *out)
{
    if (!out || !k(n) || (received & (uint16_t)~mask(n)))
        return EC_INVALID;
    unsigned syndrome = 0, position = 0;
    uint16_t word = received;
    enum ec_status status = EC_OK;
    for (unsigned p = 1; p < n; ++p)
        if ((word >> (p - 1)) & 1u)
            syndrome ^= p;
    if (count(word) & 1u)
    {
        position = syndrome ? syndrome : n;
        word ^= (uint16_t)(1u << (position - 1));
        status = EC_CORRECTED;
    }
    else if (syndrome)
    {
#if EC_MUTATION == 2
        position = syndrome;
        word ^= (uint16_t)(1u << (position - 1));
        status = EC_CORRECTED;
#else
        return EC_DETECTED;
#endif
    }
    struct ec_result result = {payload(n, word), word, position};
    *out = result;
    return status;
}

enum ec_status ec_erasures(unsigned n, uint16_t received, uint16_t erased,
                           unsigned errors, struct ec_result *out)
{
    unsigned width = k(n), s = count(erased);
    if (!out || !width || errors > 1 || 2u * errors + s >= 4 ||
        ((received | erased) & (uint16_t)~mask(n)))
        return EC_INVALID;
    unsigned matches = 0;
    struct ec_result result = {0, 0, 0};
    uint16_t known = (uint16_t)(mask(n) & (uint16_t)~erased);
#if EC_MUTATION == 4
    known = mask(n);
#endif
    for (unsigned data = 0; data < (1u << width); ++data)
    {
        uint16_t word = 0;
        (void)ec_encode(n, (uint16_t)data, &word);
#if EC_MUTATION == 3
        if (count((uint16_t)((word ^ received) & known)) <= errors + 1u)
        {
#else
        if (count((uint16_t)((word ^ received) & known)) <= errors)
        {
#endif
            ++matches;
            result.data = (uint16_t)data;
            result.word = word;
        }
    }
    if (matches == 0)
        return EC_DETECTED;
    if (matches != 1)
        return EC_AMBIGUOUS;
    *out = result;
    return (s || result.word != received) ? EC_CORRECTED : EC_OK;
}

enum ec_status ec_interleave(unsigned n, const uint16_t words[4],
                             uint8_t bits[64])
{
    return ec_interleave_phase(n, words, 0, bits);
}

enum ec_status ec_interleave_phase(unsigned n, const uint16_t words[4],
                                   unsigned phase, uint8_t bits[64])
{
    if (!k(n) || !words || !bits || phase >= 4)
        return EC_INVALID;
    for (unsigned lane = 0; lane < 4; ++lane)
        if (words[lane] & (uint16_t)~mask(n))
            return EC_INVALID;
#if EC_MUTATION == 6
    phase = 0;
#endif
    for (unsigned bit = 0; bit < n; ++bit)
        for (unsigned lane = 0; lane < 4; ++lane)
#if EC_MUTATION == 5
            bits[lane * n + bit] = (uint8_t)((words[lane] >> bit) & 1u);
#else
            bits[4u * bit + (lane + phase) % 4u] =
                (uint8_t)((words[lane] >> bit) & 1u);
#endif
    return EC_OK;
}

enum ec_status ec_deinterleave(unsigned n, const uint8_t bits[64],
                               uint16_t words[4])
{
    return ec_deinterleave_phase(n, bits, 0, words);
}

enum ec_status ec_deinterleave_phase(unsigned n, const uint8_t bits[64],
                                     unsigned phase, uint16_t words[4])
{
    if (!k(n) || !words || !bits || phase >= 4)
        return EC_INVALID;
    for (unsigned i = 0; i < 4u * n; ++i)
        if (bits[i] > 1)
            return EC_INVALID;
    uint16_t result[4];
    for (unsigned lane = 0; lane < 4; ++lane)
        result[lane] = 0;
    for (unsigned bit = 0; bit < n; ++bit)
        for (unsigned lane = 0; lane < 4; ++lane)
            result[lane] |=
                (uint16_t)((unsigned)bits[4u * bit + (lane + phase) % 4u]
                           << bit);
    for (unsigned lane = 0; lane < 4; ++lane)
        words[lane] = result[lane];
    return EC_OK;
}
