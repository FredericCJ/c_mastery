#include "codec.h"
#ifndef CC_MUTANT
#define CC_MUTANT 0
#endif
static int spans(const uint8_t *in, size_t n, uint8_t *out, size_t cap,
                 size_t *used)
{
    return (in || n == 0) && (out || cap == 0) && used;
}

static void copy_bytes(uint8_t *out, const uint8_t *in, size_t n)
{
    for (size_t i = 0; i < n; i++)
    {
        out[i] = in[i];
    }
}

int cc_lengths(const uint16_t frequency[16], uint8_t length[16])
{
    uint32_t weight[31];
    unsigned parent[31], active[31], nodes = 16, count = 0;
    uint8_t result[16];
    if (!frequency || !length)
    {
        return CC_INVALID;
    }
    for (unsigned i = 0; i < 31; i++)
    {
        weight[i] = 0;
        parent[i] = 31;
        active[i] = 0;
    }
    for (unsigned i = 0; i < 16; i++)
    {
        weight[i] = frequency[i];
        active[i] = frequency[i] != 0;
        count += active[i];
        result[i] = 0;
    }
    while (count > 1)
    {
        unsigned a = 31, b = 31;
        for (unsigned i = 0; i < nodes; i++)
        {
            if (active[i])
            {
                if (a == 31 || weight[i] < weight[a])
                {
                    b = a;
                    a = i;
                }
                else if (b == 31 || weight[i] < weight[b])
                {
                    b = i;
                }
            }
        }
        weight[nodes] = weight[a] + weight[b];
        parent[a] = nodes;
        parent[b] = nodes;
        active[a] = active[b] = 0;
        active[nodes++] = 1;
        count--;
    }
    for (unsigned i = 0; i < 16; i++)
    {
        if (frequency[i])
        {
            unsigned at = i, depth = 0;
            while (parent[at] != 31)
            {
                depth++;
                at = parent[at];
            }
            result[i] = (uint8_t)(depth ? depth : 1);
        }
    }
    copy_bytes(length, result, 16);
    return CC_OK;
}

int cc_book(const uint8_t length[16], struct cc_book *book)
{
    unsigned count[16], next[16], code = 0;
    for (unsigned i = 0; i < 16; i++)
    {
        count[i] = 0;
        next[i] = 0;
    }
    if (!length || !book)
    {
        return CC_INVALID;
    }
    for (unsigned i = 0; i < 16; i++)
    {
        if (length[i] > 15)
        {
            return CC_INVALID;
        }
        if (length[i])
        {
            count[length[i]]++;
        }
    }
    for (unsigned bits = 1; bits <= 15; bits++)
    {
        code = (code + count[bits - 1]) * 2;
        if (CC_MUTANT != 1 && code + count[bits] > (1u << bits))
        {
            return CC_INVALID;
        }
        next[bits] = code;
    }
    for (unsigned i = 0; i < 16; i++)
    {
        book->length[i] = length[i];
        book->code[i] = (uint16_t)(length[i] ? next[length[i]]++ : 0);
    }
    return CC_OK;
}

int cc_hencode(const uint8_t *symbols, size_t n, uint8_t *out, size_t cap,
               size_t *used)
{
    uint16_t frequency[16];
    uint8_t length[16], frame[CC_FRAME_LIMIT];
    struct cc_book book;
    for (unsigned i = 0; i < 16; i++)
    {
        frequency[i] = 0;
    }
    size_t bits = 0, total, bit = 0;
    if (!spans(symbols, n, out, cap, used) || n > CC_SYMBOL_LIMIT)
    {
        return CC_INVALID;
    }
    for (size_t i = 0; i < n; i++)
    {
        if (symbols[i] >= 16)
        {
            return CC_INVALID;
        }
        frequency[symbols[i]]++;
    }
    (void)cc_lengths(frequency, length);
    (void)cc_book(length, &book);
    for (size_t i = 0; i < n; i++)
    {
        bits += length[symbols[i]];
    }
    total = 18 + (bits + 7) / 8;
    if (cap < total)
    {
        return CC_SPACE;
    }
    for (size_t i = 0; i < total; i++)
    {
        frame[i] = 0;
    }
    frame[0] = (uint8_t)n;
    frame[1] = (uint8_t)(n >> 8);
    copy_bytes(frame + 2, length, 16);
    for (size_t i = 0; i < n; i++)
    {
        for (unsigned b = length[symbols[i]]; b > 0; b--)
        {
            unsigned value = (book.code[symbols[i]] >> (b - 1)) & 1u;
            frame[18 + bit / 8] |= (uint8_t)(value << (7 - (bit % 8)));
            bit++;
        }
    }
    copy_bytes(out, frame, total);
    *used = total;
    return CC_OK;
}

int cc_hdecode(const uint8_t *frame, size_t n, uint8_t *out, size_t cap,
               size_t *used)
{
    uint8_t decoded[CC_SYMBOL_LIMIT];
    struct cc_book book;
    size_t count, bit = 0, available;
    if (!spans(frame, n, out, cap, used) || n < 18 || n > CC_FRAME_LIMIT)
    {
        return CC_INVALID;
    }
    count = (size_t)frame[0] + ((size_t)frame[1] << 8);
    if (count > CC_SYMBOL_LIMIT || cc_book(frame + 2, &book) != CC_OK)
    {
        return CC_INVALID;
    }
    if (cap < count)
    {
        return CC_SPACE;
    }
    available = (n - 18) * 8;
    for (size_t i = 0; i < count; i++)
    {
        unsigned code = 0, found = 16;
        for (unsigned depth = 1; depth <= 15; depth++)
        {
            if (bit == available)
            {
                return CC_INVALID;
            }
            code = 2 * code + ((frame[18 + bit / 8] >> (7 - bit % 8)) & 1u);
            bit++;
            for (unsigned s = 0; s < 16; s++)
            {
                if (book.length[s] == depth && book.code[s] == code)
                {
                    found = s;
                    break;
                }
            }
            if (found != 16)
            {
                break;
            }
        }
        if (found == 16)
        {
            return CC_INVALID;
        }
        decoded[i] = (uint8_t)found;
    }
    if ((bit + 7) / 8 != n - 18)
    {
        return CC_INVALID;
    }
    while (bit < available)
    {
        if (CC_MUTANT != 2 && ((frame[18 + bit / 8] >> (7 - bit % 8)) & 1u))
        {
            return CC_INVALID;
        }
        bit++;
    }
    copy_bytes(out, decoded, count);
    *used = count;
    return CC_OK;
}

int cc_lzencode(const uint8_t *input, size_t n, uint8_t *out, size_t cap,
                size_t *used)
{
    uint8_t tokens[CC_TOKEN_LIMIT];
    size_t i = 0, t = 0;
    if (!spans(input, n, out, cap, used) || n > CC_INPUT_LIMIT)
    {
        return CC_INVALID;
    }
    while (i < n)
    {
        size_t best = 0, distance = 0;
        for (size_t d = 1; d <= 32 && d <= i; d++)
        {
            size_t len = 0;
            while (len < 16 && len < n - i &&
                   input[i + len] == input[i + len - d])
            {
                len++;
            }
            if (len > best)
            {
                best = len;
                distance = d;
            }
        }
        if (best >= 3)
        {
            tokens[t++] = 1;
            tokens[t++] = (uint8_t)best;
            tokens[t++] = (uint8_t)distance;
            i += best;
        }
        else
        {
            tokens[t++] = 0;
            tokens[t++] = input[i++];
        }
    }
    if (cap < t)
    {
        return CC_SPACE;
    }
    copy_bytes(out, tokens, t);
    *used = t;
    return CC_OK;
}

int cc_lzdecode(const uint8_t *tokens, size_t n, uint8_t *out, size_t cap,
                size_t *used)
{
    uint8_t decoded[CC_INPUT_LIMIT];
    size_t i = 0, at = 0;
    if (!spans(tokens, n, out, cap, used) || n > CC_TOKEN_LIMIT)
    {
        return CC_INVALID;
    }
    while (i < n)
    {
        unsigned tag = tokens[i++];
        if (tag == 0)
        {
            if (i == n)
            {
                return CC_INVALID;
            }
            if (at == CC_INPUT_LIMIT)
            {
                return CC_INVALID;
            }
            decoded[at++] = tokens[i++];
        }
        else if (tag == 1)
        {
            unsigned length, distance;
            if (n - i < 2)
            {
                return CC_INVALID;
            }
            length = tokens[i++];
            distance = tokens[i++];
            if (length < 3 || length > 16 || distance == 0 || distance > 32 ||
                distance > at || length > CC_INPUT_LIMIT - at)
            {
                return CC_INVALID;
            }
            for (unsigned j = 0; j < length; j++)
            {
                decoded[at] = decoded[at - distance];
                if (CC_MUTANT == 3 && j >= distance)
                {
                    decoded[at] ^= 1;
                }
                at++;
            }
        }
        else
        {
            return CC_INVALID;
        }
    }
    if (cap < at)
    {
        return CC_SPACE;
    }
    copy_bytes(out, decoded, at);
    *used = at;
    return CC_OK;
}

int cc_lzoptimal(const uint8_t *input, size_t n, uint8_t *out, size_t cap,
                 size_t *used)
{
    unsigned cost[CC_INPUT_LIMIT + 1];
    uint8_t take[CC_INPUT_LIMIT], dist[CC_INPUT_LIMIT], tokens[CC_TOKEN_LIMIT];
    size_t at = 0, t = 0;
    if (!spans(input, n, out, cap, used) || n > CC_INPUT_LIMIT)
    {
        return CC_INVALID;
    }
    cost[n] = 0;
    for (size_t i = n; i > 0;)
    {
        i--;
        cost[i] = 2 + cost[i + 1];
        take[i] = 1;
        dist[i] = 0;
        for (size_t d = 1; d <= 32 && d <= i; d++)
        {
            size_t len = 0;
            while (len < 16 && len < n - i &&
                   input[i + len] == input[i + len - d])
            {
                len++;
                if (CC_MUTANT != 4 && len >= 3 && 3 + cost[i + len] < cost[i])
                {
                    cost[i] = 3 + cost[i + len];
                    take[i] = (uint8_t)len;
                    dist[i] = (uint8_t)d;
                }
            }
        }
    }
    if (cap < cost[0])
    {
        return CC_SPACE;
    }
    while (at < n)
    {
        if (take[at] == 1)
        {
            tokens[t++] = 0;
            tokens[t++] = input[at++];
        }
        else
        {
            tokens[t++] = 1;
            tokens[t++] = take[at];
            tokens[t++] = dist[at];
            at += take[at];
        }
    }
    copy_bytes(out, tokens, t);
    *used = t;
    return CC_OK;
}

static int compress_using(const uint8_t *input, size_t n, uint8_t *out,
                          size_t cap, size_t *used, int optimal)
{
    uint8_t tokens[CC_TOKEN_LIMIT], symbols[CC_SYMBOL_LIMIT];
    size_t t = 0;
    int status;
    if (!spans(input, n, out, cap, used))
    {
        return CC_INVALID;
    }
    status = optimal ? cc_lzoptimal(input, n, tokens, sizeof tokens, &t)
                     : cc_lzencode(input, n, tokens, sizeof tokens, &t);
    if (status != CC_OK)
    {
        return status;
    }
    for (size_t i = 0; i < t; i++)
    {
        symbols[2 * i] = (uint8_t)(tokens[i] >> 4);
        symbols[2 * i + 1] = tokens[i] & 15;
    }
    return cc_hencode(symbols, 2 * t, out, cap, used);
}

int cc_compress(const uint8_t *input, size_t n, uint8_t *out, size_t cap,
                size_t *used)
{
    return compress_using(input, n, out, cap, used, 0);
}

int cc_compress_optimal(const uint8_t *input, size_t n, uint8_t *out,
                        size_t cap, size_t *used)
{
    return compress_using(input, n, out, cap, used, 1);
}

int cc_expand(const uint8_t *frame, size_t n, uint8_t *out, size_t cap,
              size_t *used)
{
    uint8_t tokens[CC_TOKEN_LIMIT], symbols[CC_SYMBOL_LIMIT];
    size_t s = 0;
    int status;
    if (!spans(frame, n, out, cap, used))
    {
        return CC_INVALID;
    }
    status = cc_hdecode(frame, n, symbols, sizeof symbols, &s);
    if (status != CC_OK)
    {
        return status;
    }
    if (s % 2)
    {
        return CC_INVALID;
    }
    for (size_t i = 0; i < s / 2; i++)
    {
        tokens[i] = (uint8_t)((symbols[2 * i] << 4) | symbols[2 * i + 1]);
    }
    return cc_lzdecode(tokens, s / 2, out, cap, used);
}
