#include "cache.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define CHECK(x)                                                    \
    do                                                              \
    {                                                               \
        if (!(x))                                                   \
        {                                                           \
            fprintf(stderr, "SC_CHECK line%d: %s\n", __LINE__, #x); \
            return 1;                                               \
        }                                                           \
    } while (0)

struct store_disk
{
    uint8_t bytes[67][4096];
    unsigned reads, writes, flushes;
    int fail;
    uint64_t identity, generation;
};
static struct store_disk disk;

static int rd(void *p, uint32_t page, void *out)
{
    struct store_disk *d = p;
    if (page >= 67)
    {
        return 1;
    }
    d->reads++;
    if (d->fail)
    {
        memcpy(out, d->bytes[page], 137);
        return 1;
    }
    memcpy(out, d->bytes[page], 4096);
    return 0;
}

static int wr(void *p, uint32_t page, const void *in)
{
    struct store_disk *d = p;
    if (page >= 67)
    {
        return 1;
    }
    d->writes++;
    memcpy(d->bytes[page], in, 4096);
    return 0;
}

static int fl(void *p)
{
    ((struct store_disk *)p)->flushes++;
    return 0;
}

static store_io io(void)
{
    store_io x = {&disk, rd, wr, fl, 67};
    return x;
}

static struct sc_view view(void)
{
    struct sc_view v = {io(), disk.identity, disk.generation, 1, 0};
    return v;
}

static void fill(uint64_t identity, uint64_t generation)
{
    disk.identity = identity;
    disk.generation = generation;
    for (unsigned i = 0; i < 67; i++)
    {
        memset(disk.bytes[i], (int)((identity * 17 + generation + 3 * i) % 251),
               4096);
    }
    disk.reads = disk.writes = disk.flushes = 0;
    disk.fail = 0;
}

static int direct(void)
{
    fill(1, 0);
    struct sc_cache c;
    uint8_t out[4096];
    CHECK(sc_init(&c, SC_DIRECT) == STORE_OK);
    struct sc_view v = view();
    unsigned trace[] = {1, 1, 5, 1};
    for (unsigned i = 0; i < 4; i++)
    {
        CHECK(sc_read(&c, v, trace[i], out) == STORE_OK);
        CHECK(out[0] == disk.bytes[2 + trace[i]][0] && out[4095] == out[0]);
    }
    CHECK(c.reads == 3 && c.hits == 1 && disk.reads == 3 && disk.writes == 0 &&
          disk.flushes == 0);
    store s = {0};
    s.io = io();
    for (unsigned i = 0; i < 4; i++)
    {
        CHECK(store_page_read(&s, trace[i], out) == STORE_OK &&
              out[0] == disk.bytes[2 + trace[i]][0]);
    }
    CHECK(s.reads == 3 && s.cache_hits == 1);
    printf("SC direct validated cache=%zu entry=%zu scratch=%zu\n", sizeof c,
           sizeof c.entries[0], sizeof s.scratch);
    return 0;
}

static int two_way(void)
{
    fill(1, 0);
    struct sc_cache c;
    uint8_t out[4096];
    CHECK(sc_init(&c, SC_TWO_WAY) == STORE_OK);
    unsigned ids[] = {1, 3, 1, 5, 1};
    for (unsigned i = 0; i < 5; i++)
    {
        CHECK(sc_read(&c, view(), ids[i], out) == STORE_OK &&
              out[0] == disk.bytes[2 + ids[i]][0]);
    }
    CHECK(c.reads == 3 && c.hits == 2);
    CHECK(sc_init(&c, SC_PIN_ROOT) == STORE_OK);
    unsigned roots[] = {1, 4, 7, 10, 1};
    for (unsigned i = 0; i < 5; i++)
    {
        CHECK(sc_read(&c, view(), roots[i], out) == STORE_OK &&
              out[0] == disk.bytes[2 + roots[i]][0]);
    }
    CHECK(c.reads == 4 && c.hits == 1);
    puts("SC two_way validated");
    return 0;
}

static int identities(void)
{
    fill(1, 0);
    struct sc_cache c;
    uint8_t out[4096];
    CHECK(sc_init(&c, SC_TWO_WAY) == STORE_OK);
    CHECK(sc_read(&c, view(), 1, out) == STORE_OK);
    fill(1, 1);
    CHECK(sc_read(&c, view(), 1, out) == STORE_OK &&
          out[0] == disk.bytes[3][0]);
    fill(2, 1);
    CHECK(sc_read(&c, view(), 1, out) == STORE_OK &&
          out[0] == disk.bytes[3][0]);
    struct sc_view v = view();
    v.wal = 1;
    CHECK(sc_read(&c, v, 1, out) == STORE_OK && out[0] == disk.bytes[35][0]);
    CHECK(c.reads == 4 && c.hits == 0);
    puts("SC identities validated");
    return 0;
}

static int failure(void)
{
    fill(1, 0);
    struct sc_cache c, before;
    uint8_t out[4096], prior[4096];
    CHECK(sc_init(&c, SC_DIRECT) == STORE_OK);
    CHECK(sc_read(&c, view(), 1, out) == STORE_OK);
    memcpy(&before, &c, sizeof c);
    memset(out, 0xa5, 4096);
    memcpy(prior, out, 4096);
    disk.fail = 1;
    CHECK(sc_read(&c, view(), 5, out) == STORE_IO);
    CHECK(memcmp(out, prior, 4096) == 0 && c.reads == before.reads + 1 &&
          c.hits == before.hits &&
          memcmp(c.entries, before.entries, sizeof c.entries) == 0);
    disk.fail = 0;
    CHECK(sc_read(&c, view(), 1, out) == STORE_OK && c.hits == 1);
    c.hits = UINT64_MAX;
    memcpy(prior, out, 4096);
    CHECK(sc_read(&c, view(), 1, out) == STORE_FULL &&
          memcmp(prior, out, 4096) == 0);
    c.reads = UINT64_MAX;
    CHECK(sc_read(&c, view(), 2, out) == STORE_FULL &&
          memcmp(prior, out, 4096) == 0);
    puts("SC failure validated");
    return 0;
}

static unsigned valid_entries(const store *s)
{
    unsigned n = 0;
    for (unsigned i = 0; i < 4; i++)
    {
        n += s->cache[i].valid != 0;
    }
    return n;
}

static int setup(store *s)
{
    memset(&disk, 0, sizeof disk);
    disk.identity = 1;
    return store_format(io()) || store_open(s, io());
}

static int transition(const char *which)
{
    store s;
    CHECK(setup(&s) == 0);
    uint8_t bytes[4096];
    CHECK(store_page_read(&s, 1, bytes) == STORE_OK);
    CHECK(store_begin(&s) == STORE_OK);
    CHECK(store_put(&s, "k", 1, "v", 1) == STORE_OK);
    unsigned writes = disk.writes;
    CHECK(store_get(&s, "k", 1, bytes, 4096, (size_t[]){0}) == STORE_OK);
    CHECK(disk.writes == writes);
    CHECK(store_commit(&s) == STORE_OK);
    if (!strcmp(which, "commit"))
    {
        CHECK(valid_entries(&s) == 0);
    }
    if (!strcmp(which, "checkpoint"))
    {
        CHECK(store_page_read(&s, 1, bytes) == STORE_OK);
        CHECK(store_checkpoint(&s) == STORE_OK);
        CHECK(valid_entries(&s) == 0);
    }
    if (!strcmp(which, "stale"))
    {
        CHECK(store_checkpoint(&s) == STORE_OK);
        CHECK(store_page_read(&s, 1, bytes) == STORE_OK);
        CHECK(bytes[8] == 1);
    }
    if (!strcmp(which, "contained"))
    {
        CHECK(store_checkpoint(&s) == STORE_OK);
        CHECK(store_page_read(&s, 1, bytes) == STORE_OK && bytes[8] == 0);
        size_t length = 777;
        memset(bytes, 0xa5, sizeof bytes);
        CHECK(store_get(&s, "k", 1, bytes, sizeof bytes, &length) ==
                  STORE_CORRUPT &&
              length == 777 && bytes[0] == 0xa5);
        puts("SC combined stale page contained as STORE_CORRUPT");
    }
    struct sc_cache c;
    CHECK(sc_init(&c, SC_DIRECT) == STORE_OK);
    printf("SC %s validated\n", which);
    return 0;
}

static int reused(void)
{
    store s;
    CHECK(setup(&s) == 0);
    CHECK(store_begin(&s) == STORE_OK);
    for (unsigned i = 0; i < 20; i++)
    {
        uint8_t key = (uint8_t)i;
        CHECK(store_put(&s, &key, 1, &key, 1) == STORE_OK);
    }
    CHECK(store_commit(&s) == STORE_OK);
    CHECK(s.used & 4u);
    struct sc_cache c;
    uint8_t old[4096], out[4096];
    CHECK(sc_init(&c, SC_TWO_WAY) == STORE_OK);
    struct sc_view v = {io(), 1, s.generation, s.root, s.wal_active};
    CHECK(sc_read(&c, v, 2, old) == STORE_OK);
    CHECK(old[8] == 1);
    CHECK(store_begin(&s) == STORE_OK);
    for (unsigned i = 0; i < 20; i++)
    {
        uint8_t key = (uint8_t)i;
        CHECK(store_delete(&s, &key, 1) == STORE_OK);
    }
    CHECK(store_commit(&s) == STORE_OK);
    CHECK(s.used == 3);
    CHECK(store_begin(&s) == STORE_OK);
    for (unsigned i = 100; i < 120; i++)
    {
        uint8_t key = (uint8_t)i;
        CHECK(store_put(&s, &key, 1, &key, 1) == STORE_OK);
    }
    CHECK(store_commit(&s) == STORE_OK);
    CHECK(s.used & 4u);
    v.generation = s.generation;
    v.root = s.root;
    v.wal = s.wal_active;
    CHECK(sc_read(&c, v, 2, out) == STORE_OK && out[8] == 3 &&
          memcmp(old, out, 4096) != 0 && c.reads == 2);
    puts("SC reused validated");
    return 0;
}

static int workload(void)
{
    fill(1, 0);
    struct sc_cache c;
    uint8_t out[4096];
    for (unsigned policy = 0; policy < 3; policy++)
    {
        CHECK(sc_init(&c, policy) == STORE_OK);
        for (unsigned cycle = 0; cycle < 2; cycle++)
        {
            for (unsigned id = 0; id < 8; id++)
            {
                CHECK(sc_read(&c, view(), id, out) == STORE_OK);
            }
        }
        printf("SC scan policy%u reads%llu hits%llu\n", policy,
               (unsigned long long)c.reads, (unsigned long long)c.hits);
        CHECK(c.reads == (policy == SC_PIN_ROOT ? 15u : 16u) &&
              c.hits == (policy == SC_PIN_ROOT ? 1u : 0u));
        CHECK(sc_init(&c, policy) == STORE_OK);
        for (unsigned i = 0; i < 16; i++)
        {
            CHECK(sc_read(&c, view(), 1, out) == STORE_OK);
        }
        CHECK(c.reads == 1 && c.hits == 15);
    }
    puts("SC workload validated");
    return 0;
}

static int oracle(void)
{
    unsigned policy, id, gen, identity, wal, fail, reset;
    struct sc_cache c;
    CHECK(sc_init(&c, 0) == STORE_OK);
    while (scanf("%u %u %u %u %u %u %u", &reset, &policy, &id, &gen, &identity,
                 &wal, &fail) == 7)
    {
        if (reset)
        {
            CHECK(sc_init(&c, policy) == STORE_OK);
        }
        fill(identity, gen);
        disk.fail = (int)fail;
        struct sc_view v = view();
        v.wal = (int)wal;
        uint8_t out[4096];
        memset(out, 0xa5, 4096);
        int r = sc_read(&c, v, id, out);
        printf("%d %llu %llu %u %u\n", r, (unsigned long long)c.reads,
               (unsigned long long)c.hits, out[0], out[4095]);
    }
    return 0;
}

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        return 2;
    }
    if (!strcmp(argv[1], "direct"))
    {
        return direct();
    }
    if (!strcmp(argv[1], "two_way"))
    {
        return two_way();
    }
    if (!strcmp(argv[1], "identities"))
    {
        return identities();
    }
    if (!strcmp(argv[1], "failure"))
    {
        return failure();
    }
    if (!strcmp(argv[1], "commit") || !strcmp(argv[1], "checkpoint") ||
        !strcmp(argv[1], "stale") || !strcmp(argv[1], "contained"))
    {
        return transition(argv[1]);
    }
    if (!strcmp(argv[1], "reused"))
    {
        return reused();
    }
    if (!strcmp(argv[1], "workload"))
    {
        return workload();
    }
    if (!strcmp(argv[1], "oracle"))
    {
        return oracle();
    }
    return 2;
}
