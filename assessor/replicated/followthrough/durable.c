#include "durable.h"
#include "arch.h"
#include <limits.h>
#include <string.h>
#ifndef RPL_MUTATION
#define RPL_MUTATION 0
#endif
_Static_assert(CHAR_BIT == 8 && INT_MAX == INT32_MAX,
               "8-bit octets, int32 int");

static void put32(unsigned char *p, uint32_t v)
{
    for (unsigned i = 0; i < 4; ++i)
        p[i] = (unsigned char)(v >> (24 - 8 * i));
}

static uint32_t get32(const unsigned char *p)
{
    return (uint32_t)p[0] << 24 | (uint32_t)p[1] << 16 | (uint32_t)p[2] << 8 |
           p[3];
}

static void put64(unsigned char *p, uint64_t v)
{
    put32(p, (uint32_t)(v >> 32));
    put32(p + 4, (uint32_t)v);
}

static uint64_t get64(const unsigned char *p)
{
    return (uint64_t)get32(p) << 32 | get32(p + 4);
}

static uint32_t crc(const unsigned char *p, size_t n)
{
    uint32_t c = UINT32_MAX;
    for (size_t i = 0; i < n; ++i)
    {
        c ^= p[i];
        for (unsigned j = 0; j < 8; ++j)
            c = (c >> 1) ^ (UINT32_C(0xedb88320) & (0u - (c & 1u)));
    }
    return ~c;
}

static void record(unsigned char p[64], unsigned type, uint64_t uuid,
                   uint32_t schema, unsigned position)
{
    memset(p, 0, 64);
    memcpy(p, "RPL1", 4);
    p[4] = 1;
    p[5] = (unsigned char)type;
    put64(p + 8, uuid);
    put32(p + 16, schema);
    put32(p + 20, position);
}

static void seal(unsigned char p[64])
{
    put32(p + 60, crc(p, 60));
}

static int header(const unsigned char p[64], unsigned type, uint64_t uuid,
                  uint32_t schema)
{
    if (memcmp(p, "RPL1", 4) || p[4] != 1 || p[5] != type || p[6] || p[7] ||
        (RPL_MUTATION != 3 && get32(p + 60) != crc(p, 60)))
        return RPL_CORRUPT;
    if (get64(p + 8) != uuid || (RPL_MUTATION != 4 && get32(p + 16) != schema))
        return RPL_IDENTITY;
    return RPL_OK;
}

static int zero(const unsigned char *p, size_t n)
{
    for (size_t i = 0; i < n; ++i)
        if (p[i])
            return 0;
    return 1;
}

int rpl_event_encode(uint64_t uuid, uint32_t schema, struct rpl_event e,
                     unsigned char out[64])
{
    if (!out || !uuid || !schema || !e.position || e.position > 16 ||
        e.key > 31 || (e.kind != RPL_SET && e.kind != RPL_QUERY_RESULT))
        return RPL_INVALID;
    unsigned char p[64];
    record(p, 1, uuid, schema, e.position);
    p[24] = (unsigned char)e.key;
    p[25] = (unsigned char)e.kind;
    p[26] = 2;
    if (ar_encode(2, (struct ar_record){e.value}, p + 27, 5) != AR_OK)
        return RPL_INVALID;
    seal(p);
    memcpy(out, p, 64);
    return RPL_OK;
}

int rpl_event_decode(uint64_t uuid, uint32_t schema, const unsigned char in[64],
                     struct rpl_event *out)
{
    if (!in || !out || !uuid || !schema)
        return RPL_INVALID;
    int r = header(in, 1, uuid, schema);
    if (r)
        return r;
    struct rpl_event e = {get32(in + 20), in[24], in[25], 0};
    struct ar_record value;
    if (!e.position || e.position > 16 || e.key > 31 ||
        (e.kind != 1 && e.kind != 2) || in[26] != 2 || !zero(in + 32, 28) ||
        ar_decode(2, in + 27, 5, &value) != AR_OK)
        return RPL_CORRUPT;
    e.value = value.value;
    *out = e;
    return RPL_OK;
}

static int translate(int r)
{
    return r == STORE_OK              ? RPL_OK
           : r == STORE_FULL          ? RPL_FULL
           : r == STORE_INDETERMINATE ? RPL_UNCERTAIN
           : r == STORE_IO            ? RPL_IO
           : r == STORE_CORRUPT       ? RPL_CORRUPT
                                      : RPL_STATE;
}

static int valid(const struct rpl_log *l)
{
    return l && l->pages && !l->poisoned && !l->pages->poisoned &&
           !l->pages->active;
}

static int write_record(store *s, const unsigned char *key, size_t n,
                        const unsigned char b[64])
{
    return store_put(s, key, n, b, 64);
}

static int empty(void *ctx, const unsigned char *k, size_t kn,
                 const unsigned char *v, size_t vn)
{
    (void)k;
    (void)kn;
    (void)v;
    (void)vn;
    *(int *)ctx = 0;
    return 1;
}

static int finish(struct rpl_log *l, int result)
{
    if (result != STORE_OK)
    {
        store_abort(l->pages);
        if (l->pages->poisoned)
            l->poisoned = 1;
    }
    return translate(result);
}

int rpl_create(store *s, uint64_t uuid, uint32_t schema, struct rpl_log *out)
{
    if (!s || !out || !uuid || !schema)
        return RPL_INVALID;
    if (s->poisoned || s->active)
        return RPL_STATE;
    int clear = 1, r = store_scan(s, NULL, 0, empty, &clear);
    if (r)
        return translate(r);
    if (!clear)
        return RPL_STATE;
    unsigned char b[64];
    record(b, 2, uuid, schema, 0);
    seal(b);
    r = store_begin(s);
    if (!r)
        r = write_record(s, (const unsigned char *)"M", 1, b);
    if (!r)
        r = store_commit(s);
    else
        store_abort(s);
    if (r)
        return translate(r);
    struct rpl_log built = {0};
    built.pages = s;
    built.uuid = uuid;
    built.schema = schema;
    rp_log_init(&built.memory);
    *out = built;
    return RPL_OK;
}

struct rp_scan
{
    uint64_t uuid;
    uint32_t schema;
    unsigned seen, count, mask, meta, snapshot, chunks, snapshot_position;
    struct rpl_event events[16];
    struct rpl_snapshot image;
    int status;
};

static int visit(void *ctx, const unsigned char *key, size_t kn,
                 const unsigned char *p, size_t n)
{
    struct rp_scan *s = ctx;
    int r = RPL_CORRUPT;
    if (n != 64)
        goto fail;
    if (kn == 1 && key[0] == 'M')
    {
        r = header(p, 2, s->uuid, s->schema);
        if (r)
            goto fail;
        if (s->meta || get32(p + 20) > 16 || !zero(p + 24, 36))
            goto corrupt;
        s->meta = 1;
        s->count = get32(p + 20);
        return 0;
    }
    if (kn == 2 && key[0] == 'E' && key[1] >= 1 && key[1] <= 16)
    {
        struct rpl_event e;
        r = rpl_event_decode(s->uuid, s->schema, p, &e);
        if (r)
            goto fail;
        if (e.position != key[1] || (s->seen & (1u << (e.position - 1))))
            goto corrupt;
        s->events[e.position - 1] = e;
        s->seen |= 1u << (e.position - 1);
        return 0;
    }
    if (kn == 1 && key[0] == 'S')
    {
        r = header(p, 3, s->uuid, s->schema);
        if (r)
            goto fail;
        if (s->snapshot || get32(p + 20) > 16 || !zero(p + 24, 36))
            goto corrupt;
        s->snapshot = 1;
        s->snapshot_position = get32(p + 20);
        return 0;
    }
    if (kn == 2 && key[0] == 'C' && key[1] < 8)
    {
        unsigned chunk = key[1];
        r = header(p, 4, s->uuid, s->schema);
        if (r)
            goto fail;
        if ((s->chunks & (1u << chunk)) || p[24] != chunk || p[25] > 15 ||
            p[26] || p[27] || !zero(p + 48, 12))
            goto corrupt;
        if (s->chunks && s->image.position != get32(p + 20))
            goto corrupt;
        s->image.position = get32(p + 20);
        for (unsigned i = 0; i < 4; ++i)
        {
            struct ar_record value;
            if (ar_decode(2, p + 28 + 5 * i, 5, &value) != AR_OK)
                goto corrupt;
            if (!(p[25] & (1u << i)) && value.value != 0)
                goto corrupt;
            s->image.values[4 * chunk + i] = value.value;
        }
        s->image.present |= (uint32_t)p[25] << (chunk * 4);
        s->chunks |= 1u << chunk;
        return 0;
    }
corrupt:
    r = RPL_CORRUPT;
fail:
    s->status = r;
    return 1;
}

static int scan(store *pages, uint64_t uuid, uint32_t schema, struct rp_scan *s)
{
    memset(s, 0, sizeof *s);
    s->uuid = uuid;
    s->schema = schema;
    int r = store_scan(pages, NULL, 0, visit, s);
    if (r)
        return translate(r);
    if (s->status)
        return s->status;
    if (!s->meta || (RPL_MUTATION != 6 && s->seen != ((1u << s->count) - 1u)))
        return RPL_CORRUPT;
    if (s->snapshot)
    {
        if (s->chunks != 255 || s->image.position != s->snapshot_position ||
            s->image.position > s->count)
            return RPL_CORRUPT;
        struct rpl_snapshot expected = {s->image.position, 0, {0}};
        for (unsigned i = 0; i < expected.position; ++i)
        {
            struct rpl_event e = s->events[i];
            expected.present |= UINT32_C(1) << e.key;
            expected.values[e.key] = e.value;
        }
        if (expected.present != s->image.present ||
            memcmp(expected.values, s->image.values, sizeof expected.values))
            return RPL_CORRUPT;
    }
    else if (s->chunks)
        return RPL_CORRUPT;
    return RPL_OK;
}

int rpl_open(store *pages, uint64_t uuid, uint32_t schema, struct rpl_log *out)
{
    if (!pages || !out || !uuid || !schema)
        return RPL_INVALID;
    if (pages->poisoned || pages->active)
        return RPL_STATE;
    struct rp_scan s;
    int r = scan(pages, uuid, schema, &s);
    if (r)
        return r;
    struct rpl_log built = {0};
    built.pages = pages;
    built.uuid = uuid;
    built.schema = schema;
    rp_log_init(&built.memory);
    for (unsigned i = 0; i < s.count; ++i)
    {
        r = rp_log_append(&built.memory, s.events[i].key, s.events[i].value);
        if (r != RP_OK)
            return RPL_CORRUPT;
        built.kinds[i] = s.events[i].kind;
    }
    *out = built;
    return RPL_OK;
}

int rpl_append(struct rpl_log *l, unsigned kind, unsigned key, int32_t value)
{
    if (!l || key > 31 || (kind != 1 && kind != 2))
        return RPL_INVALID;
    if (!valid(l))
        return RPL_STATE;
    if (l->memory.count == 16)
        return RPL_FULL;
    unsigned count = l->memory.count, used = l->memory.arena.used;
    if (rp_log_append(&l->memory, key, value) != RP_OK)
        return RPL_FULL;
    unsigned char b[64], k[2] = {'E', (unsigned char)(count + 1)};
    int r = store_begin(l->pages);
    if (!r)
    {
        (void)rpl_event_encode(l->uuid, l->schema,
                               (struct rpl_event){count + 1, key, kind, value},
                               b);
        r = write_record(l->pages, k, 2, b);
    }
    if (!r)
    {
        record(b, 2, l->uuid, l->schema, count + 1);
        seal(b);
        r = write_record(l->pages, (const unsigned char *)"M", 1, b);
    }
    if (!r)
        r = store_commit(l->pages);
    if (r)
    {
        if (RPL_MUTATION != 1)
            l->memory.count = count;
        l->memory.arena.used = used;
        return finish(l, r);
    }
    l->kinds[count] = kind;
    return RPL_OK;
}

int rpl_get(const struct rpl_log *l, unsigned position, unsigned key,
            int32_t *out)
{
    if (!out || !l || key > 31 || position > l->memory.count)
        return RPL_INVALID;
    if (!valid(l))
        return RPL_STATE;
    int value;
    int r = rp_log_get(&l->memory, position, key, &value);
    if (r != RP_OK)
        return RPL_STATE;
    *out = (int32_t)value;
    return RPL_OK;
}

int rpl_snapshot_save(struct rpl_log *l, unsigned position)
{
    if (!l || position > l->memory.count)
        return RPL_INVALID;
    if (!valid(l))
        return RPL_STATE;
    unsigned actual = RPL_MUTATION == 2 ? l->memory.count : position;
    unsigned char b[64], key[2] = {'C', 0};
    int r = store_begin(l->pages);
    for (unsigned chunk = 0; !r && chunk < 8; ++chunk)
    {
        record(b, 4, l->uuid, l->schema, position);
        b[24] = (unsigned char)chunk;
        for (unsigned i = 0; i < 4; ++i)
        {
            int v = 0;
            unsigned k = chunk * 4 + i;
            if (rp_log_get(&l->memory, actual, k, &v) == RP_OK)
                b[25] |= (unsigned char)(1u << i);
            (void)ar_encode(2, (struct ar_record){v}, b + 28 + 5 * i, 5);
        }
        seal(b);
        key[1] = (unsigned char)chunk;
        r = write_record(l->pages, key, 2, b);
    }
    if (!r)
    {
        record(b, 3, l->uuid, l->schema, position);
        seal(b);
        r = write_record(l->pages, (const unsigned char *)"S", 1, b);
    }
    if (!r)
        r = store_commit(l->pages);
    return finish(l, r);
}

int rpl_snapshot_load(struct rpl_log *l, struct rpl_snapshot *out)
{
    if (!l || !out)
        return RPL_INVALID;
    if (!valid(l))
        return RPL_STATE;
    struct rp_scan s;
    int r = scan(l->pages, l->uuid, l->schema, &s);
    if (r)
        return r;
    if (!s.snapshot)
        return RPL_STATE;
    *out = s.image;
    return RPL_OK;
}

int rpl_state(struct rpl_log *l, unsigned use_snapshot,
              struct rpl_snapshot *out)
{
    if (!l || !out || use_snapshot > 1)
        return RPL_INVALID;
    if (!valid(l))
        return RPL_STATE;
    struct rpl_snapshot s = {0, 0, {0}};
    if (use_snapshot)
    {
        int r = rpl_snapshot_load(l, &s);
        if (r)
            return r;
    }
    for (unsigned i = s.position; i < l->memory.count; ++i)
    {
        struct rp_event e = l->memory.events[i];
        s.present |= UINT32_C(1) << e.key;
        s.values[e.key] = e.value;
    }
    s.position = l->memory.count;
    *out = s;
    return RPL_OK;
}

static int page_read(void *ctx, uint32_t page, void *out)
{
    struct rpl_pages *a = ctx;
    if (page >= STORE_IMAGE_PAGES)
        return -1;
    unsigned char bytes[STORE_PAGE_SIZE];
    size_t done = 0, offset = (size_t)page * STORE_PAGE_SIZE;
    for (unsigned calls = 0; done < sizeof bytes && calls < a->bytes.call_limit;
         ++calls)
    {
        ptrdiff_t n = a->bytes.read(a->bytes.context, offset + done,
                                    bytes + done, sizeof bytes - done);
        if (n == -2)
            continue;
        if (n <= 0 || (size_t)n > sizeof bytes - done)
            return -1;
        done += (size_t)n;
    }
    if (done != sizeof bytes)
        return -1;
    memcpy(out, bytes, sizeof bytes);
    return 0;
}

static int page_write(void *ctx, uint32_t page, const void *in)
{
    struct rpl_pages *a = ctx;
    if (page >= STORE_IMAGE_PAGES)
        return -1;
    const unsigned char *bytes = in;
    size_t done = 0, offset = (size_t)page * STORE_PAGE_SIZE;
    for (unsigned calls = 0;
         done < STORE_PAGE_SIZE && calls < a->bytes.call_limit; ++calls)
    {
        size_t progress = RPL_MUTATION == 5 ? 0 : done;
        ptrdiff_t n = a->bytes.write(a->bytes.context, offset + progress,
                                     bytes + progress, STORE_PAGE_SIZE - done);
        if (n == -2)
            continue;
        if (n <= 0 || (size_t)n > STORE_PAGE_SIZE - done)
            return -1;
        done += (size_t)n;
    }
    return done == STORE_PAGE_SIZE ? 0 : -1;
}

static int page_flush(void *ctx)
{
    struct rpl_pages *a = ctx;
    return a->bytes.flush(a->bytes.context);
}

int rpl_pages(struct rpl_pages *a, struct rpl_bytes bytes, store_io *out)
{
    if (!a || !out || !bytes.read || !bytes.write || !bytes.flush ||
        !bytes.call_limit ||
        bytes.extent < (size_t)STORE_IMAGE_PAGES * STORE_PAGE_SIZE)
        return RPL_INVALID;
    a->bytes = bytes;
    *out = (store_io){a, page_read, page_write, page_flush, STORE_IMAGE_PAGES};
    return RPL_OK;
}
