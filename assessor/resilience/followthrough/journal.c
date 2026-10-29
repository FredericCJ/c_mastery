#include "journal.h"
#include <limits.h>
_Static_assert(CHAR_BIT == 8 && INT_MAX == INT32_MAX && INT_MIN == INT32_MIN,
               "Journal wire contract requires 8-bit octets and 32-bit int");

static bool io_valid(struct rzj_io io)
{
    return io.size && io.read && io.write && io.truncate && io.flush;
}

static uint64_t get64(const unsigned char *p)
{
    uint64_t v = 0;
    for (unsigned i = 0; i < 8; ++i)
    {
        v = v * 256 + p[i];
    }
    return v;
}

static uint32_t get32(const unsigned char *p)
{
    uint32_t v = 0;
    for (unsigned i = 0; i < 4; ++i)
    {
        v = v * 256 + p[i];
    }
    return v;
}

static void put64(unsigned char *p, uint64_t v)
{
    for (unsigned i = 8; i > 0; --i)
    {
        p[i - 1] = (unsigned char)v;
        v >>= 8;
    }
}

static void put32(unsigned char *p, uint32_t v)
{
    for (unsigned i = 4; i > 0; --i)
    {
        p[i - 1] = (unsigned char)v;
        v >>= 8;
    }
}

static uint32_t checksum(const unsigned char *p)
{
    uint32_t v = 2166136261u;
    for (unsigned i = 0; i < 48; ++i)
    {
        v = (v ^ p[i]) * 16777619u;
    }
    return v;
}

static int signed32(uint32_t value)
{
    return value <= INT32_MAX ? (int)value : -1 - (int)(UINT32_MAX - value);
}

static uint32_t wire32(int value)
{
    return value >= 0 ? (uint32_t)value : UINT32_MAX - (uint32_t)(-(value + 1));
}

static int read_all(struct rzj_io io, size_t offset, unsigned char *bytes,
                    size_t size)
{
    size_t done = 0;
    while (done < size)
    {
        size_t n = 0;
        int status =
            io.read(io.context, offset + done, bytes + done, size - done, &n);
        if (status != RZ_OK || !n || n > size - done)
        {
            return RZJ_UNCERTAIN;
        }
        done += n;
    }
    return RZ_OK;
}

static int write_all(struct rzj_io io, size_t offset,
                     const unsigned char *bytes, size_t size)
{
    size_t done = 0;
    while (done < size)
    {
        size_t n = 0;
        int status =
            io.write(io.context, offset + done, bytes + done, size - done, &n);
        if (status != RZ_OK || !n || n > size - done)
        {
            return RZJ_UNCERTAIN;
        }
        done += n;
    }
    return RZ_OK;
}

static void encode(unsigned char record[RZJ_RECORD_BYTES],
                   const struct rzj_journal *j, unsigned kind, uint64_t now,
                   struct rzj_request request, int result, bool duplicate)
{
    for (unsigned i = 0; i < RZJ_RECORD_BYTES; ++i)
    {
        record[i] = 0;
    }
    record[0] = 'R';
    record[1] = 'Z';
    record[2] = 'J';
    record[3] = '1';
    record[4] = (unsigned char)kind;
    record[5] = (unsigned char)request.client;
    record[6] = (unsigned char)request.id;
    record[7] = (unsigned char)(request.delta + 8);
    put64(record + 8, (uint64_t)j->records + 1);
    put64(record + 16, now);
    put64(record + 24, request.fence);
    put32(record + 32, wire32(result));
    record[36] = (unsigned char)duplicate;
    record[37] = (unsigned char)j->receiver.window;
    uint32_t sum = checksum(record);
    put32(record + 48, sum);
    record[56] = 'C';
    record[57] = 'O';
    record[58] = 'M';
    record[59] = '1';
    put32(record + 60, ~sum);
}

static int replay(struct rzj_journal *j,
                  const unsigned char record[RZJ_RECORD_BYTES])
{
    if (record[0] != 'R' || record[1] != 'Z' || record[2] != 'J' ||
        record[3] != '1' || record[56] != 'C' || record[57] != 'O' ||
        record[58] != 'M' || record[59] != '1' ||
        get64(record + 8) != (uint64_t)j->records + 1 ||
        record[37] != j->receiver.window || record[36] > 1 ||
        get32(record + 48) != checksum(record) ||
        get32(record + 60) != ~get32(record + 48))
    {
        return RZJ_CORRUPT;
    }
    for (unsigned i = 38; i < 48; ++i)
    {
        if (record[i])
        {
            return RZJ_CORRUPT;
        }
    }
    for (unsigned i = 52; i < 56; ++i)
    {
        if (record[i])
        {
            return RZJ_CORRUPT;
        }
    }
    uint64_t now = get64(record + 16), fence = get64(record + 24);
    if (now < j->receiver.last)
    {
        return RZJ_CORRUPT;
    }
    if (record[4] == 2 || record[4] == 3)
    {
        if (j->issued == UINT64_MAX || fence != j->issued + 1 || record[5] ||
            record[6] || record[7] != 8 || get32(record + 32) || record[36])
        {
            return RZJ_CORRUPT;
        }
        j->issued = fence;
        j->receiver.fence = fence;
        j->receiver.last = now;
        if (record[4] == 3)
        {
            for (unsigned i = 0; i < 4; ++i)
            {
                j->receiver.entries[i] = (struct rz_entry){0};
            }
        }
    }
    else if (record[4] == 1)
    {
        if (record[7] > 16 || !fence || fence > j->issued)
        {
            return RZJ_CORRUPT;
        }
        int result;
        bool duplicate;
        int status = rz_receive(&j->receiver, now, record[5], record[6], fence,
                                (int)record[7] - 8, &result, &duplicate);
        if (status != RZ_OK || result != signed32(get32(record + 32)) ||
            duplicate != (record[36] != 0))
        {
            return RZJ_CORRUPT;
        }
    }
    else
    {
        return RZJ_CORRUPT;
    }
    ++j->records;
    return RZ_OK;
}

int rzj_open(struct rzj_io io, unsigned window, struct rzj_journal *out)
{
    if (!io_valid(io) || !out || !window || window > 16)
    {
        return RZ_INVALID;
    }
    size_t size = 0;
    if (io.size(io.context, &size) != RZ_OK)
    {
        return RZJ_UNCERTAIN;
    }
    if (size > RZJ_RECORD_BYTES * RZJ_MAX_RECORDS)
    {
        return RZJ_CORRUPT;
    }
    struct rzj_journal staged = {.io = io};
    (void)rz_receiver_init(&staged.receiver, 0, window);
    unsigned char record[RZJ_RECORD_BYTES];
    size_t full = size / RZJ_RECORD_BYTES;
    for (size_t i = 0; i < full; ++i)
    {
        int status = read_all(io, i * RZJ_RECORD_BYTES, record, sizeof record);
        if (status != RZ_OK)
        {
            return status;
        }
        status = replay(&staged, record);
        if (status != RZ_OK)
        {
            return status;
        }
    }
    if (size % RZJ_RECORD_BYTES)
    {
        if (io.truncate(io.context, full * RZJ_RECORD_BYTES) != RZ_OK ||
            io.flush(io.context) != RZ_OK)
        {
            return RZJ_UNCERTAIN;
        }
    }
    *out = staged;
    return RZ_OK;
}

static int append(struct rzj_journal *j,
                  const unsigned char record[RZJ_RECORD_BYTES])
{
    size_t offset = (size_t)j->records * RZJ_RECORD_BYTES;
    int status = write_all(j->io, offset, record, 56);
#if RZF_MUTANT != 82
    if (status == RZ_OK)
    {
        status = j->io.flush(j->io.context);
    }
#endif
    if (status == RZ_OK)
    {
        status = write_all(j->io, offset + 56, record + 56, 8);
    }
#if RZF_MUTANT != 81
    if (status == RZ_OK)
    {
        status = j->io.flush(j->io.context);
    }
#endif
    if (status != RZ_OK)
    {
        j->poisoned = true;
        return RZJ_UNCERTAIN;
    }
    return RZ_OK;
}

static int issue(struct rzj_journal *j, uint64_t now, uint64_t *out,
                 unsigned kind)
{
    if (!j || !out)
    {
        return RZ_INVALID;
    }
    if (j->poisoned)
    {
        return RZJ_UNCERTAIN;
    }
    if (now < j->receiver.last)
    {
        return RZ_RANGE;
    }
    if (j->records == RZJ_MAX_RECORDS || j->issued == UINT64_MAX)
    {
        return RZ_FULL;
    }
    unsigned char record[RZJ_RECORD_BYTES];
    uint64_t token = j->issued + 1;
    encode(record, j, kind, now, (struct rzj_request){0, 0, token, 0}, 0,
           false);
    int status = append(j, record);
    if (status != RZ_OK)
    {
        return status;
    }
    j->issued = token;
    j->receiver.fence = token;
    j->receiver.last = now;
    if (kind == 3)
    {
        for (unsigned i = 0; i < 4; ++i)
        {
            j->receiver.entries[i] = (struct rz_entry){0};
        }
    }
    ++j->records;
    *out = token;
    return RZ_OK;
}

int rzj_issue(struct rzj_journal *j, uint64_t now, uint64_t *out)
{
    return issue(j, now, out, 2);
}

int rzj_rotate(struct rzj_journal *j, uint64_t now, uint64_t *out)
{
    return issue(j, now, out, 3);
}

int rzj_record(const struct rzj_journal *j, unsigned sequence,
               struct rzj_record *out)
{
    if (!j || !out || !sequence || sequence > j->records)
    {
        return RZ_INVALID;
    }
    if (j->poisoned)
    {
        return RZJ_UNCERTAIN;
    }
    unsigned char bytes[RZJ_RECORD_BYTES];
    int status = read_all(j->io, (size_t)(sequence - 1) * RZJ_RECORD_BYTES,
                          bytes, sizeof bytes);
    if (status != RZ_OK)
    {
        return status;
    }
    if (get64(bytes + 8) != sequence || get32(bytes + 48) != checksum(bytes) ||
        get32(bytes + 60) != ~get32(bytes + 48))
    {
        return RZJ_CORRUPT;
    }
    *out = (struct rzj_record){
        sequence,
        bytes[4],
        get64(bytes + 16),
        {bytes[5], bytes[6], get64(bytes + 24), (int)bytes[7] - 8},
        signed32(get32(bytes + 32)),
        bytes[36] != 0};
    return RZ_OK;
}

int rzj_apply(struct rzj_journal *j, uint64_t now, struct rzj_request request,
              int *out, bool *duplicate)
{
    if (!j || !out || !duplicate)
    {
        return RZ_INVALID;
    }
    if (j->poisoned)
    {
        return RZJ_UNCERTAIN;
    }
    if (!request.fence || request.fence > j->issued)
    {
        return RZ_DENIED;
    }
    struct rz_receiver staged = j->receiver;
    int result;
    bool replayed;
#if RZF_MUTANT == 83
    staged.fence = request.fence;
#endif
    int status = rz_receive(&staged, now, request.client, request.id,
                            request.fence, request.delta, &result, &replayed);
    if (status != RZ_OK)
    {
        return status;
    }
    if (j->records == RZJ_MAX_RECORDS)
    {
        return RZ_FULL;
    }
    unsigned char record[RZJ_RECORD_BYTES];
    encode(record, j, 1, now, request, result, replayed);
    status = append(j, record);
    if (status != RZ_OK)
    {
        return status;
    }
    j->receiver = staged;
    ++j->records;
    *out = result;
    *duplicate = replayed;
    return RZ_OK;
}

int rzj_mirror(const struct rzj_journal *source, struct rzj_io target)
{
    if (!source || !io_valid(target) || target.context == source->io.context)
    {
        return RZ_INVALID;
    }
    if (source->poisoned)
    {
        return RZJ_UNCERTAIN;
    }
    size_t size = 0, limit = (size_t)source->records * RZJ_RECORD_BYTES;
    if (target.size(target.context, &size) != RZ_OK)
    {
        return RZJ_UNCERTAIN;
    }
    if (size > limit)
    {
        return RZJ_CORRUPT;
    }
    unsigned char record[RZJ_RECORD_BYTES], other[RZJ_RECORD_BYTES];
    for (size_t i = 0; i < limit; i += RZJ_RECORD_BYTES)
    {
        if (read_all(source->io, i, record, sizeof record) != RZ_OK)
        {
            return RZJ_UNCERTAIN;
        }
        if (i < size)
        {
            size_t n = size - i;
            if (n > RZJ_RECORD_BYTES)
            {
                n = RZJ_RECORD_BYTES;
            }
            if (read_all(target, i, other, n) != RZ_OK)
            {
                return RZJ_UNCERTAIN;
            }
            for (size_t k = 0; k < n; ++k)
            {
                if (record[k] != other[k])
                {
                    return RZJ_CORRUPT;
                }
            }
        }
    }
    for (size_t i = (size / RZJ_RECORD_BYTES) * RZJ_RECORD_BYTES; i < limit;
         i += RZJ_RECORD_BYTES)
    {
        if (read_all(source->io, i, record, sizeof record) != RZ_OK ||
            write_all(target, i, record, 56) != RZ_OK ||
            target.flush(target.context) != RZ_OK ||
            write_all(target, i + 56, record + 56, 8) != RZ_OK ||
            target.flush(target.context) != RZ_OK)
        {
            return RZJ_UNCERTAIN;
        }
    }
    return RZ_OK;
}

int rzj_replica_read(struct rzj_io io, unsigned window, struct rzj_replica *out)
{
    if (!out)
    {
        return RZ_INVALID;
    }
    struct rzj_journal recovered;
    int status = rzj_open(io, window, &recovered);
    if (status == RZ_OK)
    {
        *out = (struct rzj_replica){recovered.receiver.total,
                                    recovered.receiver.fence, recovered.issued,
                                    recovered.records};
    }
    return status;
}
