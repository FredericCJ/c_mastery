#include "transport.h"
#include <limits.h>
#ifndef RPO_MUTATION
#define RPO_MUTATION 0
#endif
int rpo_wire_read(rpo_reader_fn read, void *context, unsigned limit,
                  size_t length, unsigned char *out)
{
    if (!read || !out || !limit || (length != 24 && length != 64))
        return RPL_INVALID;
    struct rf_parser parser;
    if (rf_parser_init(&parser, 1))
        return RPL_STATE;
    unsigned char staged[64] = {0}, bytes[9];
    size_t at = 0;
    for (unsigned calls = 0; calls < limit; ++calls)
    {
        ptrdiff_t n = read(context, bytes, sizeof bytes);
        if (n == -2)
            continue;
        if (n < 0 || n > (ptrdiff_t)sizeof bytes)
            return RPL_IO;
        if (!n)
        {
            if (RPO_MUTATION != 15 && (rf_eof(&parser) || at != length))
                return RPL_CORRUPT;
            for (size_t i = 0; i < length; ++i)
                out[i] = staged[i];
            return RPL_OK;
        }
        size_t used = 0;
        while (used < (size_t)n)
        {
            size_t consumed;
            struct rf_frame frame;
            int r = rf_feed(&parser, bytes + used, (size_t)n - used, &consumed,
                            &frame);
            used += consumed;
            if (r < 0)
                return RPL_CORRUPT;
            if (r)
            {
                if ((RPO_MUTATION != 16 && frame.length != 8) ||
                    frame.length > length - at)
                    return RPL_CORRUPT;
                for (unsigned i = 0; i < frame.length; ++i)
                    staged[at++] = frame.bytes[i];
            }
        }
    }
    return RPL_IO;
}

int rpo_wire_write(rpo_writer_fn write, void *context, unsigned limit,
                   const unsigned char *bytes, size_t length)
{
    if (!write || !bytes || !limit || (length != 24 && length != 64))
        return RPL_INVALID;
    struct rf_outbox box;
    struct rf_message messages[8];
    unsigned slot;
    if (rf_outbox_init(&box, 64) || rf_open(&box, &slot) != 1)
        return RPL_STATE;
    unsigned frames = (unsigned)(length / 8);
    for (unsigned f = 0; f < frames; ++f)
    {
        struct rf_frame frame = {.length = 8};
        for (unsigned i = 0; i < 8; ++i)
            frame.bytes[i] = bytes[f * 8 + i];
        if (rf_message_init(&messages[f], f + 1, 1, &frame) ||
            rf_enqueue(&box, slot, &messages[f]) != 1)
            return RPL_STATE;
    }
    unsigned calls = 0;
    while (box.connections[slot].count)
    {
        if (calls++ == limit || rf_send_once(&box, slot, write, context) < 0)
        {
            (void)rf_disconnect(&box, slot);
            return RPL_IO;
        }
    }
    return rf_disconnect(&box, slot) == 0 ? RPL_OK : RPL_STATE;
}

static void put64(unsigned char *p, uint64_t n)
{
    for (unsigned i = 0; i < 8; ++i)
        p[i] = (unsigned char)(n >> (56 - 8 * i));
}

static uint64_t get64(const unsigned char *p)
{
    uint64_t n = 0;
    for (unsigned i = 0; i < 8; ++i)
        n = n * 256 + p[i];
    return n;
}

int rpo_reply_encode(struct rpo_reply r, unsigned char out[24])
{
    if (!out || !r.uuid || !r.position || r.position > 16)
        return RPL_INVALID;
    unsigned char p[24] = {'R', 'P', 'O', '1'};
    put64(p + 4, r.uuid);
    p[15] = (unsigned char)r.position;
    put64(p + 16, (uint64_t)r.total);
    for (unsigned i = 0; i < 24; ++i)
        out[i] = p[i];
    return RPL_OK;
}

int rpo_reply_decode(const unsigned char in[24], struct rpo_reply *out)
{
    if (!in || !out)
        return RPL_INVALID;
    if (in[0] != 'R' || in[1] != 'P' || in[2] != 'O' || in[3] != '1' ||
        in[12] || in[13] || in[14] || !in[15] || in[15] > 16)
        return RPL_CORRUPT;
    uint64_t uuid = get64(in + 4), u = get64(in + 16);
    if (!uuid)
        return RPL_IDENTITY;
    int64_t total = u <= INT64_MAX ? (int64_t)u : -(int64_t)(~u) - 1;
    *out = (struct rpo_reply){uuid, in[15], total};
    return RPL_OK;
}
