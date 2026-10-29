#include "durable.h"
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define CHECK(x)                                                            \
    do                                                                      \
    {                                                                       \
        if (!(x))                                                           \
        {                                                                   \
            fprintf(stderr, "RP_CHECK %s:%d %s\n", __FILE__, __LINE__, #x); \
            return 1;                                                       \
        }                                                                   \
    } while (0)

struct rp_medium
{
    unsigned char stable[STORE_IMAGE_PAGES][STORE_PAGE_SIZE];
    unsigned char pending[STORE_IMAGE_PAGES][STORE_PAGE_SIZE];
    unsigned char dirty[STORE_IMAGE_PAGES];
    unsigned event, cut, mode, dead, reads, read_fail, retries, zero;
    unsigned tear_n;
    size_t chunk;
};
static struct rp_medium disk, base;
static struct rpl_pages adapter;
static store pages;
static struct rpl_log logbook;

static void persist(struct rp_medium *m)
{
    for (unsigned i = 0; i < STORE_IMAGE_PAGES; ++i)
        if (m->dirty[i])
        {
            memcpy(m->stable[i], m->pending[i], STORE_PAGE_SIZE);
            m->dirty[i] = 0;
        }
}

static int cut(struct rp_medium *m, size_t offset, size_t n)
{
    ++m->event;
    if (!m->cut || m->cut != m->event)
        return 0;
    if (m->mode == 1)
        persist(m);
    if (m->mode == 2 && n)
    {
        unsigned page = (unsigned)(offset / STORE_PAGE_SIZE);
        size_t in = offset % STORE_PAGE_SIZE;
        memcpy(m->stable[page] + in, m->pending[page] + in, n / 2);
    }
    if (m->mode == 3 && n)
    {
        unsigned page = (unsigned)(offset / STORE_PAGE_SIZE);
        memcpy(m->stable[page], m->pending[page], m->tear_n);
    }
    if (m->mode == 4 && !n)
    {
        unsigned left = m->tear_n;
        for (unsigned page = 0; page < STORE_IMAGE_PAGES && left; ++page)
            if (m->dirty[page])
            {
                memcpy(m->stable[page], m->pending[page], STORE_PAGE_SIZE);
                --left;
            }
    }
    m->dead = 1;
    return 1;
}

static ptrdiff_t mread(void *ctx, size_t off, unsigned char *p, size_t n)
{
    struct rp_medium *m = ctx;
    unsigned page = (unsigned)(off / STORE_PAGE_SIZE);
    size_t in = off % STORE_PAGE_SIZE;
    if (m->dead || ++m->reads == m->read_fail || page >= STORE_IMAGE_PAGES)
        return -1;
    if (m->retries)
    {
        --m->retries;
        return -2;
    }
    if (m->zero)
        return 0;
    if (n > m->chunk)
        n = m->chunk;
    if (n > STORE_PAGE_SIZE - in)
        n = STORE_PAGE_SIZE - in;
    memcpy(p, (m->dirty[page] ? m->pending[page] : m->stable[page]) + in, n);
    return (ptrdiff_t)n;
}

static ptrdiff_t mwrite(void *ctx, size_t off, const unsigned char *p, size_t n)
{
    struct rp_medium *m = ctx;
    unsigned page = (unsigned)(off / STORE_PAGE_SIZE);
    size_t in = off % STORE_PAGE_SIZE;
    if (m->dead || page >= STORE_IMAGE_PAGES)
        return -1;
    if (m->retries)
    {
        --m->retries;
        return -2;
    }
    if (m->zero)
        return 0;
    if (n > m->chunk)
        n = m->chunk;
    if (n > STORE_PAGE_SIZE - in)
        n = STORE_PAGE_SIZE - in;
    if (!m->dirty[page])
        memcpy(m->pending[page], m->stable[page], STORE_PAGE_SIZE);
    memcpy(m->pending[page] + in, p, n);
    m->dirty[page] = 1;
    if (cut(m, off, n))
        return -1;
    return (ptrdiff_t)n;
}

static int mflush(void *ctx)
{
    struct rp_medium *m = ctx;
    if (m->dead || cut(m, 0, 0))
        return -1;
    persist(m);
    return 0;
}

static int attach(struct rp_medium *m, store_io *io)
{
    struct rpl_bytes b = {m, mread, mwrite, mflush, sizeof m->stable, 4098};
    return rpl_pages(&adapter, b, io);
}

static void restart(struct rp_medium *m)
{
    memset(m->dirty, 0, sizeof m->dirty);
    m->dead = 0;
    m->cut = 0;
    m->event = 0;
    m->reads = 0;
    m->read_fail = 0;
    m->retries = 0;
    m->zero = 0;
}

static int setup(void)
{
    memset(&disk, 0, sizeof disk);
    disk.chunk = 1024;
    store_io io;
    CHECK(attach(&disk, &io) == RPL_OK);
    CHECK(store_format(io) == STORE_OK);
    CHECK(store_open(&pages, io) == STORE_OK);
    CHECK(rpl_create(&pages, UINT64_C(0x1020304050607080), 17, &logbook) ==
          RPL_OK);
    CHECK(store_checkpoint(&pages) == STORE_OK);
    disk.event = 0;
    return 0;
}

static int reopen(void)
{
    store_io io;
    CHECK(attach(&disk, &io) == RPL_OK);
    CHECK(store_open(&pages, io) == STORE_OK);
    CHECK(rpl_open(&pages, UINT64_C(0x1020304050607080), 17, &logbook) ==
          RPL_OK);
    return 0;
}

static int bytes_case(void)
{
    unsigned char wire[64];
    struct rpl_event e = {3, 31, RPL_QUERY_RESULT, INT32_MIN},
                     out = {9, 9, 9, 9};
    CHECK(rpl_event_encode(9, 17, e, wire) == RPL_OK);
    CHECK(rpl_event_decode(9, 17, wire, &out) == RPL_OK);
    CHECK(out.position == 3 && out.key == 31 && out.value == INT32_MIN &&
          out.kind == RPL_QUERY_RESULT);
    wire[63] ^= 1;
    out = (struct rpl_event){9, 9, 9, 9};
    CHECK(rpl_event_decode(9, 17, wire, &out) == RPL_CORRUPT);
    CHECK(out.value == 9);
    wire[63] ^= 1;
    CHECK(rpl_event_decode(8, 17, wire, &out) == RPL_IDENTITY);
    memset(&disk, 0, sizeof disk);
    disk.chunk = 127;
    store_io io;
    CHECK(attach(&disk, &io) == RPL_OK);
    unsigned char page[STORE_PAGE_SIZE], copy[STORE_PAGE_SIZE];
    for (unsigned i = 0; i < STORE_PAGE_SIZE; ++i)
        page[i] = (unsigned char)(i / 127 + i % 127);
    disk.retries = 2;
    CHECK(io.write(io.ctx, 3, page) == 0);
    CHECK(io.flush(io.ctx) == 0);
    CHECK(!memcmp(disk.stable[3], page, sizeof page));
    CHECK(io.read(io.ctx, 3, copy) == 0);
    CHECK(!memcmp(page, copy, sizeof page));
    memset(copy, 0xa5, sizeof copy);
    disk.read_fail = disk.reads + 2;
    CHECK(io.read(io.ctx, 3, copy) != 0);
    for (unsigned i = 0; i < sizeof copy; ++i)
        CHECK(copy[i] == 0xa5);
    disk.read_fail = 0;
    disk.zero = 1;
    CHECK(io.write(io.ctx, 3, page) != 0);
    disk.zero = 0;
    adapter.bytes.call_limit = 1;
    CHECK(io.read(io.ctx, 3, copy) != 0);
    return 0;
}

static int log_case(void)
{
    CHECK(setup() == 0);
    CHECK(rpl_append(&logbook, RPL_SET, 3, 7) == RPL_OK);
    CHECK(rpl_append(&logbook, RPL_QUERY_RESULT, 3, 9) == RPL_OK);
    int32_t value = 0;
    CHECK(rpl_get(&logbook, 1, 3, &value) == RPL_OK && value == 7);
    CHECK(rpl_get(&logbook, 2, 3, &value) == RPL_OK && value == 9);
    CHECK(store_checkpoint(&pages) == STORE_OK);
    disk.event = 0;
    disk.cut = 1;
    CHECK(rpl_append(&logbook, RPL_SET, 4, 11) == RPL_IO);
    CHECK(logbook.memory.count == 2 && logbook.memory.arena.used == 6);
    restart(&disk);
    CHECK(reopen() == 0);
    CHECK(logbook.memory.count == 2);
    /* A valid but unpublished extra event is corruption, not a new prefix. */
    unsigned char record[64], key[2] = {'E', 3};
    CHECK(rpl_event_encode(logbook.uuid, 17,
                           (struct rpl_event){3, 4, RPL_SET, 11},
                           record) == RPL_OK);
    CHECK(store_begin(&pages) == STORE_OK);
    CHECK(store_put(&pages, key, 2, record, 64) == STORE_OK);
    CHECK(store_commit(&pages) == STORE_OK);
    struct rpl_log unchanged = logbook;
    CHECK(rpl_open(&pages, logbook.uuid, 17, &logbook) == RPL_CORRUPT);
    CHECK(!memcmp(&unchanged, &logbook, sizeof logbook));
    CHECK(setup() == 0);
    for (unsigned i = 0; i < 16; ++i)
        CHECK(rpl_append(&logbook, RPL_SET, i, (int32_t)i - 8) == RPL_OK);
    CHECK(rpl_append(&logbook, RPL_SET, 0, 0) == RPL_FULL);
    CHECK(logbook.memory.arena.used == 48);
    restart(&disk);
    CHECK(reopen() == 0);
    CHECK(logbook.memory.count == 16 && logbook.memory.arena.used == 48);
    return 0;
}

static int snapshot_case(void)
{
    CHECK(setup() == 0);
    CHECK(rpl_append(&logbook, RPL_SET, 3, 7) == RPL_OK);
    CHECK(rpl_append(&logbook, RPL_QUERY_RESULT, 3, 9) == RPL_OK);
    CHECK(rpl_snapshot_save(&logbook, 1) == RPL_OK);
    struct rpl_snapshot snap;
    CHECK(rpl_snapshot_load(&logbook, &snap) == RPL_OK);
    CHECK(snap.position == 1 && snap.present == 8 && snap.values[3] == 7);
    struct rpl_snapshot full, fast;
    CHECK(rpl_state(&logbook, 0, &full) == RPL_OK);
    CHECK(rpl_state(&logbook, 1, &fast) == RPL_OK);
    CHECK(full.position == fast.position && full.present == fast.present &&
          !memcmp(full.values, fast.values, sizeof full.values));
    restart(&disk);
    CHECK(reopen() == 0);
    CHECK(rpl_snapshot_load(&logbook, &snap) == RPL_OK && snap.values[3] == 7);
    struct rpl_log unchanged = logbook;
    CHECK(rpl_open(&pages, logbook.uuid, 18, &logbook) == RPL_IDENTITY);
    CHECK(!memcmp(&unchanged, &logbook, sizeof logbook));
    CHECK(rpl_snapshot_save(&logbook, 0) == RPL_OK);
    CHECK(rpl_snapshot_load(&logbook, &snap) == RPL_OK && snap.position == 0 &&
          snap.present == 0);
    /* The recorded query answer9 remains9 even if a fresh external query would
     * answer99. The log never invokes that query or repeats an effect. */
    CHECK(rpl_state(&logbook, 1, &fast) == RPL_OK && fast.values[3] == 9);
    CHECK(fast.values[3] != 99);
    unsigned naive_effect = 0;
    for (unsigned replay = 0; replay < 2; ++replay)
        ++naive_effect;
    CHECK(naive_effect == 2);
    return 0;
}

static int faults_case(void)
{
    CHECK(setup() == 0);
    CHECK(rpl_append(&logbook, RPL_SET, 0, 7) == RPL_OK);
    CHECK(store_checkpoint(&pages) == STORE_OK);
    disk.event = 0;
    base = disk;
    CHECK(rpl_append(&logbook, RPL_QUERY_RESULT, 0, 9) == RPL_OK);
    unsigned steps = disk.event;
    CHECK(steps == 139);
    unsigned append_cases = 0, recovery_cases = 0;
    for (unsigned mode = 0; mode < 3; ++mode)
        for (unsigned at = 1; at <= steps; ++at)
        {
            disk = base;
            CHECK(reopen() == 0);
            disk.event = 0;
            disk.mode = mode;
            disk.cut = at;
            int r = rpl_append(&logbook, RPL_QUERY_RESULT, 0, 9);
            CHECK(r == RPL_IO || r == RPL_UNCERTAIN);
            CHECK(logbook.memory.count == 1);
            restart(&disk);
            CHECK(reopen() == 0);
            unsigned expected = mode == 1 && at >= 138 ? 2 : 1;
            CHECK(logbook.memory.count == expected);
            int32_t v;
            CHECK(rpl_get(&logbook, expected, 0, &v) == RPL_OK &&
                  v == (expected == 2 ? 9 : 7));
            ++append_cases;
        }
    disk = base;
    CHECK(reopen() == 0);
    CHECK(rpl_append(&logbook, RPL_QUERY_RESULT, 0, 9) == RPL_OK);
    base = disk;
    disk.event = 0;
    CHECK(store_checkpoint(&pages) == STORE_OK);
    steps = disk.event;
    CHECK(steps == 139);
    for (unsigned mode = 0; mode < 3; ++mode)
        for (unsigned at = 1; at <= steps; ++at)
        {
            disk = base;
            restart(&disk);
            disk.cut = at;
            disk.mode = mode;
            store_io io;
            CHECK(attach(&disk, &io) == RPL_OK);
            CHECK(store_open(&pages, io) == STORE_IO);
            restart(&disk);
            CHECK(reopen() == 0);
            CHECK(logbook.memory.count == 2);
            ++recovery_cases;
        }
    printf("FAULTS %u %u\n", append_cases, recovery_cases);
    return 0;
}

static int oracle(void)
{
    char op;
    unsigned kind, key, pos;
    int32_t value;
    uint64_t uuid;
    uint32_t schema;
    while (scanf(" %c", &op) == 1)
    {
        if (op == 'N')
        {
            CHECK(setup() == 0);
            puts("0");
        }
        else if (op == 'A')
        {
            CHECK(scanf("%u %u %" SCNd32, &kind, &key, &value) == 3);
            printf("%d\n", rpl_append(&logbook, kind, key, value));
        }
        else if (op == 'G')
        {
            CHECK(scanf("%u %u", &pos, &key) == 2);
            value = 1234567;
            int r = rpl_get(&logbook, pos, key, &value);
            printf("%d %" PRId32 "\n", r, value);
        }
        else if (op == 'S')
        {
            CHECK(scanf("%u", &pos) == 1);
            printf("%d\n", rpl_snapshot_save(&logbook, pos));
        }
        else if (op == 'R')
        {
            restart(&disk);
            CHECK(reopen() == 0);
            printf("%u\n", logbook.memory.count);
        }
        else if (op == 'Q')
        {
            struct rpl_snapshot s;
            CHECK(scanf("%u", &pos) == 1);
            int r = rpl_state(&logbook, pos, &s);
            printf("%d", r);
            if (!r)
            {
                printf(" %u %" PRIu32, s.position, s.present);
                for (unsigned i = 0; i < 32; ++i)
                    printf(" %" PRId32, s.values[i]);
            }
            puts("");
        }
        else if (op == 'E')
        {
            unsigned char b[64];
            CHECK(scanf("%" SCNu64 " %" SCNu32 " %u %u %u %" SCNd32, &uuid,
                        &schema, &pos, &kind, &key, &value) == 6);
            int r = rpl_event_encode(
                uuid, schema, (struct rpl_event){pos, key, kind, value}, b);
            printf("%d", r);
            if (!r)
                for (unsigned i = 0; i < 64; ++i)
                    printf(" %u", b[i]);
            puts("");
        }
        else
            return 1;
    }
    return 0;
}

static int tears_case(void)
{
    CHECK(setup() == 0);
    disk.chunk = STORE_PAGE_SIZE;
    CHECK(rpl_append(&logbook, RPL_SET, 0, 7) == RPL_OK);
    CHECK(store_checkpoint(&pages) == STORE_OK);
    disk.event = 0;
    base = disk;
    unsigned append_cases = 0, recovery_cases = 0;
    for (unsigned phase = 0; phase < 2; ++phase)
    {
        if (phase)
        {
            disk = base;
            CHECK(reopen() == 0);
            CHECK(rpl_append(&logbook, RPL_QUERY_RESULT, 0, 9) == RPL_OK);
            base = disk;
        }
        for (unsigned at = 1; at <= 37; ++at)
        {
            unsigned flushing = phase ? (at == 33 || at == 35 || at == 37)
                                      : (at == 2 || at == 35 || at == 37);
            unsigned bound = flushing ? STORE_IMAGE_PAGES : STORE_PAGE_SIZE;
            for (unsigned prefix = 0; prefix <= bound; ++prefix)
            {
                disk = base;
                restart(&disk);
                if (!phase)
                    CHECK(reopen() == 0);
                disk.cut = at;
                disk.mode = flushing ? 4 : 3;
                disk.tear_n = prefix;
                disk.event = 0;
                if (phase)
                {
                    store_io io;
                    CHECK(attach(&disk, &io) == RPL_OK);
                    CHECK(store_open(&pages, io) == STORE_IO);
                }
                else
                {
                    int r = rpl_append(&logbook, RPL_QUERY_RESULT, 0, 9);
                    CHECK(r == RPL_IO || r == RPL_UNCERTAIN);
                    CHECK(logbook.memory.count == 1);
                }
                restart(&disk);
                CHECK(reopen() == 0);
                unsigned expected =
                    phase ? 2
                          : ((at == 36 && prefix == STORE_PAGE_SIZE) ||
                                     (at == 37 && prefix >= 1)
                                 ? 2
                                 : 1);
                CHECK(logbook.memory.count == expected);
                int32_t value;
                CHECK(rpl_get(&logbook, expected, 0, &value) == RPL_OK &&
                      value == (expected == 2 ? 9 : 7));
                if (phase)
                    ++recovery_cases;
                else
                    ++append_cases;
            }
        }
    }
    printf("TEARS %u %u\n", append_cases, recovery_cases);
    return 0;
}

int main(int argc, char **argv)
{
    if (argc != 2)
        return 2;
    if (!strcmp(argv[1], "durable_log"))
        return log_case();
    if (!strcmp(argv[1], "durable_snapshot"))
        return snapshot_case();
    if (!strcmp(argv[1], "durable_bytes"))
        return bytes_case();
    if (!strcmp(argv[1], "durable_faults"))
        return faults_case();
    if (!strcmp(argv[1], "durable_tears"))
        return tears_case();
    if (!strcmp(argv[1], "oracle"))
        return oracle();
    return 2;
}
