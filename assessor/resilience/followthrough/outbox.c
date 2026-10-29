#include "outbox.h"

static uint32_t get32(const unsigned char *p)
{
    uint32_t v = 0;
    for (unsigned i = 0; i < 4; ++i)
        v = v * 256 + p[i];
    return v;
}

static uint64_t get64(const unsigned char *p)
{
    uint64_t v = 0;
    for (unsigned i = 0; i < 8; ++i)
        v = v * 256 + p[i];
    return v;
}

static void put32(unsigned char *p, uint32_t v)
{
    for (unsigned i = 4; i; i--)
    {
        p[i - 1] = (unsigned char)v;
        v >>= 8;
    }
}

static void put64(unsigned char *p, uint64_t v)
{
    for (unsigned i = 8; i; i--)
    {
        p[i - 1] = (unsigned char)v;
        v >>= 8;
    }
}

static uint32_t checksum(const unsigned char *p)
{
    uint32_t v = 2166136261u;
    for (unsigned i = 0; i < 20; ++i)
        v = (v ^ p[i]) * 16777619u;
    return v;
}

static int read_all(struct rzj_io io, size_t at, unsigned char *p, size_t n)
{
    size_t done = 0;
    while (done < n)
    {
        size_t got = 0;
        if (io.read(io.context, at + done, p + done, n - done, &got) != RZ_OK ||
            !got || got > n - done)
            return RZJ_UNCERTAIN;
        done += got;
    }
    return RZ_OK;
}

static int write_all(struct rzj_io io, size_t at, const unsigned char *p,
                     size_t n)
{
    size_t done = 0;
    while (done < n)
    {
        size_t sent = 0;
        if (io.write(io.context, at + done, p + done, n - done, &sent) !=
                RZ_OK ||
            !sent || sent > n - done)
            return RZJ_UNCERTAIN;
        done += sent;
    }
    return RZ_OK;
}

static bool valid_io(struct rzj_io io)
{
    return io.size && io.read && io.write && io.flush && io.truncate;
}

static int usable(const struct rzs_outbox *s)
{
    return s->poisoned || s->journal->poisoned ? RZJ_UNCERTAIN : RZ_OK;
}

static void enqueue(struct rzs_outbox *s, struct rzj_record record)
{
    struct rzs_event event = {record.sequence,      record.request.client,
                              record.request.id,    1,
                              record.request.fence, record.result};
    s->events[s->count++] = event;
    s->pending.values[(s->pending.head + s->pending.count) % 4] =
        record.sequence;
    ++s->pending.count;
}

int rzs_open(struct rzj_journal *j, struct rzj_io ack, struct msg_hub *hub,
             struct rzs_outbox *out)
{
    if (!j || !hub || !out || !valid_io(ack) || ack.context == j->io.context)
        return RZ_INVALID;
    if (j->poisoned)
        return RZJ_UNCERTAIN;
    (void)pthread_mutex_lock(&hub->mutex);
    bool empty = !hub->stop && !hub->channel[0].closed &&
                 hub->channel[0].capacity && !hub->channel[0].count &&
                 !hub->channel[0].offered;
    (void)pthread_mutex_unlock(&hub->mutex);
    if (!empty)
        return RZ_BUSY;
    size_t size = 0;
    if (ack.size(ack.context, &size) != RZ_OK)
        return RZJ_UNCERTAIN;
    if (size > RZS_ACK_BYTES * RZJ_MAX_RECORDS)
        return RZJ_CORRUPT;
    unsigned all[RZJ_MAX_RECORDS], count = 0;
    struct rzs_outbox staged = {
        .journal = j, .hub = hub, .acknowledgments = ack, .epoch = j->issued};
    for (unsigned sequence = 1; sequence <= j->records; ++sequence)
    {
        struct rzj_record record;
        int status = rzj_record(j, sequence, &record);
        if (status != RZ_OK)
            return status;
        if (record.kind == 3)
        {
            staged.seen[0] = staged.seen[1] = 0;
        }
        if (record.kind == 1 && !record.duplicate)
        {
            if (record.request.client > 1 || record.request.id < 1 ||
                record.request.id > 15)
                return RZJ_CORRUPT;
            all[count++] = sequence;
            staged.seen[record.request.client] |= 1u << record.request.id;
        }
    }
    size_t full = size / RZS_ACK_BYTES;
    if (full > count)
        return RZJ_CORRUPT;
    for (size_t i = 0; i < full; ++i)
    {
        unsigned char b[RZS_ACK_BYTES];
        int status = read_all(ack, i * RZS_ACK_BYTES, b, sizeof b);
        if (status != RZ_OK)
            return status;
        if (b[0] != 'R' || b[1] != 'Z' || b[2] != 'A' || b[3] != '1' || b[4] ||
            b[5] || b[6] || b[7] || get64(b + 8) != all[i] ||
            get32(b + 16) != i + 1 || get32(b + 20) != checksum(b) ||
            b[24] != 'A' || b[25] != 'C' || b[26] != 'K' || b[27] != '1' ||
            get32(b + 28) != ~get32(b + 20))
            return RZJ_CORRUPT;
    }
    if (count - full > 4)
        return RZJ_CORRUPT;
    staged.ack_records = (unsigned)full;
    staged.acknowledged = full ? all[full - 1] : 0;
    for (unsigned i = (unsigned)full; i < count; ++i)
    {
        struct rzj_record record;
        int status = rzj_record(j, all[i], &record);
        if (status != RZ_OK)
            return status;
        enqueue(&staged, record);
    }
    if (size % RZS_ACK_BYTES &&
        (ack.truncate(ack.context, full * RZS_ACK_BYTES) != RZ_OK ||
         ack.flush(ack.context) != RZ_OK))
        return RZJ_UNCERTAIN;
    *out = staged;
    return RZ_OK;
}

int rzs_apply(struct rzs_outbox *s, uint64_t now, struct rzj_request request,
              int *out, bool *duplicate)
{
    if (!s || !out || !duplicate || request.client > 1 || request.id < 1 ||
        request.id > 15)
        return RZ_INVALID;
    int status = usable(s);
    if (status != RZ_OK)
        return status;
    if (!request.fence || request.fence != s->epoch)
        return RZ_STALE;
    struct rz_receiver receiver = s->journal->receiver;
    int result;
    bool replayed;
    status = rz_receive(&receiver, now, request.client, request.id,
                        request.fence, request.delta, &result, &replayed);
    if (status != RZ_OK)
        return status;
#if RZF_MUTANT != 93
    if (!replayed && (s->seen[request.client] & (1u << request.id)))
        return RZ_STALE;
#endif
#if RZF_MUTANT != 91
    if (!replayed && s->count == 4)
        return RZ_FULL;
#endif
    status = rzj_apply(s->journal, now, request, &result, &replayed);
    if (status != RZ_OK)
        return status;
    if (!replayed)
    {
        /* External serialization makes the preflight reservation stable. */
        if (s->count == 4)
            return RZ_FULL;
        enqueue(s, (struct rzj_record){s->journal->records, 1, now, request,
                                       result, false});
        s->seen[request.client] |= 1u << request.id;
    }
    *out = result;
    *duplicate = replayed;
    return RZ_OK;
}

int rzs_rotate(struct rzs_outbox *s, uint64_t now, uint64_t *out)
{
    if (!s || !out)
        return RZ_INVALID;
    int status = usable(s);
    if (status != RZ_OK)
        return status;
    if (s->count)
        return RZ_BUSY;
    uint64_t epoch;
    status = rzj_rotate(s->journal, now, &epoch);
    if (status != RZ_OK)
        return status;
    s->epoch = epoch;
    s->seen[0] = s->seen[1] = 0;
    *out = epoch;
    return RZ_OK;
}

int rzs_forward(struct rzs_outbox *s)
{
    if (!s)
        return RZ_INVALID;
    int status = usable(s);
    if (status != RZ_OK)
        return status;
    if (!s->pending.count)
        return RZ_EMPTY;
    unsigned sequence = s->pending.values[s->pending.head];
    status = rz_forward(&s->pending, s->hub, 0);
    if (status == RZ_OK)
        for (unsigned i = 0; i < s->count; ++i)
            if (s->events[i].sequence == sequence)
            {
                s->events[i].state = 2;
                return RZ_OK;
            }
    return status;
}

int rzs_deliver(struct rzs_outbox *s, struct rzs_event *out)
{
    if (!s || !out)
        return RZ_INVALID;
    int status = usable(s);
    if (status != RZ_OK)
        return status;
    unsigned channel = 99, value = 0;
    enum msg_status m = msg_select(s->hub, 1, false, &channel, &value);
    if (m != MSG_OK)
        return m == MSG_EMPTY       ? RZ_EMPTY
               : m == MSG_CLOSED    ? RZ_CLOSED
               : m == MSG_CANCELLED ? RZ_CANCELLED
                                    : RZ_INVALID;
    for (unsigned i = 0; i < s->count; ++i)
        if (s->events[i].sequence == value && s->events[i].state == 2)
        {
            s->events[i].state = 3;
            *out = s->events[i];
            return RZ_OK;
        }
    return RZJ_CORRUPT;
}

int rzs_ack(struct rzs_outbox *s, unsigned sequence)
{
    if (!s)
        return RZ_INVALID;
    int status = usable(s);
    if (status != RZ_OK)
        return status;
    if (!s->count || s->events[0].sequence != sequence ||
        s->events[0].state != 3)
        return RZ_STALE;
    if (s->ack_records == RZJ_MAX_RECORDS)
        return RZ_FULL;
    unsigned char b[RZS_ACK_BYTES] = {0};
    b[0] = 'R';
    b[1] = 'Z';
    b[2] = 'A';
    b[3] = '1';
    put64(b + 8, sequence);
    put32(b + 16, s->ack_records + 1);
    uint32_t sum = checksum(b);
    put32(b + 20, sum);
    b[24] = 'A';
    b[25] = 'C';
    b[26] = 'K';
    b[27] = '1';
    put32(b + 28, ~sum);
    size_t at = (size_t)s->ack_records * RZS_ACK_BYTES;
    status = write_all(s->acknowledgments, at, b, 24);
    if (status == RZ_OK)
        status = s->acknowledgments.flush(s->acknowledgments.context);
    if (status == RZ_OK)
        status = write_all(s->acknowledgments, at + 24, b + 24, 8);
#if RZF_MUTANT != 92
    if (status == RZ_OK)
        status = s->acknowledgments.flush(s->acknowledgments.context);
#endif
    if (status != RZ_OK)
    {
        s->poisoned = true;
        return RZJ_UNCERTAIN;
    }
    s->acknowledged = sequence;
    ++s->ack_records;
    --s->count;
    for (unsigned i = 0; i < s->count; ++i)
        s->events[i] = s->events[i + 1];
    s->events[s->count] = (struct rzs_event){0};
    return RZ_OK;
}
