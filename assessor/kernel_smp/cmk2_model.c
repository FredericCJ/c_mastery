#include "cmk2.h"

static uint32_t u32(const unsigned char *p)
{
    return (uint32_t)p[0] | (uint32_t)p[1] << 8 | (uint32_t)p[2] << 16 |
           (uint32_t)p[3] << 24;
}

int cmk2_madt_parse(const void *data, size_t available,
                    struct cmk2_topology *out)
{
    const unsigned char *p = data;
    if (!p || !out || available < 44 || p[0] != 'A' || p[1] != 'P' ||
        p[2] != 'I' || p[3] != 'C')
        return -1;
    uint32_t n = u32(p + 4);
    unsigned sum = 0;
    if (n < 44 || n > available || n > 65536)
        return -1;
    for (uint32_t i = 0; i < n; i++)
        sum += p[i];
    if ((sum & 255) != 0)
        return -1;
    struct cmk2_topology t = {0};
    t.lapic = u32(p + 36);
    if (!t.lapic || (t.lapic & 4095))
        return -1;
    for (uint32_t at = 44; at < n;)
    {
        if (n - at < 2 || p[at + 1] < 2 || p[at + 1] > n - at)
            return -1;
        unsigned len = p[at + 1];
        if (p[at] == 0)
        {
            if (len != 8)
                return -1;
            if (u32(p + at + 4) & 1)
            {
                if (t.count == CMK2_MAX_CPUS)
                    return -1;
                for (unsigned j = 0; j < t.count; j++)
                    if (t.ids[j] == p[at + 3])
                        return -1;
                t.ids[t.count++] = p[at + 3];
            }
        }
        else if (p[at] == 5 || p[at] == 9)
            return -1; /* explicit fixed xAPIC profile */
        at += len;
    }
    if (!t.count)
        return -1;
    *out = t;
    return 0;
}

int cmk2_channel_send(struct cmk2_channel *q, struct cmk2_message m)
{
    if (!q || q->count > CMK2_QUEUE_CAPACITY || q->head >= CMK2_QUEUE_CAPACITY)
        return CMK2_INVALID;
    if (q->closed)
        return CMK2_CLOSED;
    if (q->count == CMK2_QUEUE_CAPACITY)
        return CMK2_FULL;
    q->ring[(q->head + q->count) % CMK2_QUEUE_CAPACITY] = m;
    q->count++;
    return CMK2_OK;
}

int cmk2_channel_receive(struct cmk2_channel *q, struct cmk2_message *m)
{
    if (!q || !m || q->count > CMK2_QUEUE_CAPACITY ||
        q->head >= CMK2_QUEUE_CAPACITY)
        return CMK2_INVALID;
    if (!q->count)
        return q->closed ? CMK2_CLOSED : CMK2_EMPTY;
    *m = q->ring[q->head];
    q->head = (q->head + 1) % CMK2_QUEUE_CAPACITY;
    q->count--;
    return CMK2_OK;
}

void cmk2_channel_close(struct cmk2_channel *q)
{
    if (q)
        q->closed = 1;
}
