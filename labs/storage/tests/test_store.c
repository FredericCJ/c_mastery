#define _POSIX_C_SOURCE 200809L
#include "store.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#if defined(STORAGE_WITH_RUNTIME)
#include "cvm.h"
#include "store_host.h"
#endif
#if defined(__unix__)
#include "store_file.h"
#include <unistd.h>
#endif
#define CHECK(x)                                                         \
    do                                                                   \
    {                                                                    \
        if (!(x))                                                        \
        {                                                                \
            fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, #x); \
            return 1;                                                    \
        }                                                                \
    } while (0)

typedef struct store_medium
{
    uint8_t stable[STORE_IMAGE_PAGES][STORE_PAGE_SIZE];
    uint8_t pending[STORE_IMAGE_PAGES][STORE_PAGE_SIZE];
    uint8_t dirty[STORE_IMAGE_PAGES];
    unsigned event, cut, mode;
    int dead, read_fail;
} medium;

static medium disk, base, snap;

static void persist(medium *m)
{
    unsigned p;
    for (p = STORE_IMAGE_PAGES; p > 0; p--)
        if (m->dirty[p - 1])
        {
            memcpy(m->stable[p - 1], m->pending[p - 1], STORE_PAGE_SIZE);
            m->dirty[p - 1] = 0;
        }
}

static int boundary(medium *m, uint32_t page, int writing)
{
    unsigned p;
    m->event++;
    if (!m->cut || m->cut != m->event)
        return 0;
    /* Admitted crash states: discard, reverse-order partial set, prefix/suffix
     * addressed-page tear, or all pending bytes persisted despite failed call. */
    if (m->mode == 1)
    {
        for (p = STORE_IMAGE_PAGES; p > 0; p--)
            if (m->dirty[p - 1] && ((p - 1) % 2 == 0))
                memcpy(m->stable[p - 1], m->pending[p - 1], STORE_PAGE_SIZE);
    }
    if ((m->mode == 2 || m->mode == 3) && writing)
    {
        size_t begin = m->mode == 2 ? 0 : 2048;
        memcpy(m->stable[page] + begin, m->pending[page] + begin, 2048);
    }
    if (m->mode == 4)
        persist(m);
    m->dead = 1;
    return -1;
}

static int mread(void *ctx, uint32_t p, void *b)
{
    medium *m = ctx;
    if (m->dead || m->read_fail || p >= STORE_IMAGE_PAGES)
        return -1;
    memcpy(b, m->dirty[p] ? m->pending[p] : m->stable[p], STORE_PAGE_SIZE);
    return 0;
}

static int mwrite(void *ctx, uint32_t p, const void *b)
{
    medium *m = ctx;
    if (m->dead || p >= STORE_IMAGE_PAGES)
        return -1;
    memcpy(m->pending[p], b, STORE_PAGE_SIZE);
    m->dirty[p] = 1;
    return boundary(m, p, 1);
}

static int mflush(void *ctx)
{
    medium *m = ctx;
    if (m->dead)
        return -1;
    if (boundary(m, 0, 0))
        return -1;
    persist(m);
    return 0;
}

static store_io mio(medium *m)
{
    store_io io = {m, mread, mwrite, mflush, STORE_IMAGE_PAGES};
    return io;
}

static void crash(medium *m)
{
    memset(m->dirty, 0, sizeof(m->dirty));
    m->dead = 0;
    m->cut = 0;
    m->event = 0;
    m->read_fail = 0;
}

static void key(unsigned x, uint8_t b[4])
{
    b[0] = (uint8_t)(x >> 24);
    b[1] = (uint8_t)(x >> 16);
    b[2] = (uint8_t)(x >> 8);
    b[3] = (uint8_t)x;
}

static int put(store *s, unsigned k, unsigned v)
{
    uint8_t a[4], b[4];
    key(k, a);
    key(v, b);
    return store_put(s, a, 4, b, 4);
}

static int get(store *s, unsigned k, unsigned *v)
{
    uint8_t a[4], b[4];
    size_t n = 999;
    int r;
    key(k, a);
    r = store_get(s, a, 4, b, 4, &n);
    if (!r)
    {
        if (n != 4)
            return STORE_CORRUPT;
        *v = ((unsigned)b[0] << 24) | ((unsigned)b[1] << 16) |
             ((unsigned)b[2] << 8) | b[3];
    }
    return r;
}

static int del(store *s, unsigned k)
{
    uint8_t a[4];
    key(k, a);
    return store_delete(s, a, 4);
}

static int seed(store *s)
{
    memset(&disk, 0, sizeof(disk));
    CHECK(store_format(mio(&disk)) == STORE_OK);
    CHECK(store_open(s, mio(&disk)) == STORE_OK);
    return 0;
}

typedef struct scan_state
{
    unsigned count, last;
    int bad;
} scan_state;

static int collect(void *ctx, const uint8_t *k, size_t kl, const uint8_t *v,
                   size_t vl)
{
    scan_state *c = ctx;
    unsigned x;
    (void)v;
    if (kl != 4 || vl != 4)
    {
        c->bad = 1;
        return 1;
    }
    x = ((unsigned)k[0] << 24) | ((unsigned)k[1] << 16) |
        ((unsigned)k[2] << 8) | k[3];
    if (c->count && x <= c->last)
        c->bad = 1;
    c->last = x;
    c->count++;
    return 0;
}

static int format_test(void)
{
    store s;
    size_t n = 123;
    uint8_t v[64] = {7};
    CHECK(seed(&s) == 0);
    CHECK(store_check(&s) == STORE_OK);
    CHECK(store_get(&s, "x", 1, v, sizeof(v), &n) == STORE_NOT_FOUND);
    CHECK(n == 123 && v[0] == 7);
    CHECK(store_begin(&s) == STORE_OK);
    CHECK(store_put(&s, "", 0, v, 1) == STORE_INVALID);
    CHECK(store_put(&s, "a", 1, v, 65) == STORE_INVALID);
    CHECK(store_put(&s, "a", 1, NULL, 0) == STORE_OK);
    CHECK(store_get(&s, "a", 1, v, 0, &n) == STORE_OK && n == 0);
    store_abort(&s);
    CHECK(store_get(&s, "a", 1, v, 1, &n) == STORE_NOT_FOUND);
    disk.read_fail = 1;
    CHECK(store_open(&s, mio(&disk)) == STORE_IO);
    return 0;
}

/* Early gates operate on explicitly injected component states. They
 * deliberately avoid opening, committing or recovering an image before those
 * mechanisms are implemented. The later integration cases still exercise the
 * complete API. */
static void component_state(store *s, int active)
{
    memset(&disk, 0, sizeof disk);
    memset(s, 0, sizeof *s);
    s->io = mio(&disk);
    s->root = s->saved_root = 1;
    s->used = s->saved_used = 3;
    s->scratch[1].leaf = 1;
    s->active = active;
}

static int codec_test(void)
{
    uint8_t bytes[STORE_PAGE_SIZE], before[STORE_PAGE_SIZE];
    store_node node = {0}, out, old;
    node.leaf = 1;
    node.n = 1;
    node.klen[0] = 3;
    node.vlen[0] = 2;
    node.key[0][0] = 0;
    node.key[0][1] = 0x80;
    node.key[0][2] = 0xff;
    node.value[0][0] = 0x34;
    node.value[0][1] = 0x12;
    CHECK(store_page_encode(7, UINT64_C(0x0102030405060708), &node, bytes) ==
          STORE_OK);
    CHECK(bytes[0] == 0x32 && bytes[1] == 0x54 && bytes[2] == 0x53 &&
          bytes[3] == 0x43);
    CHECK(bytes[4] == 11 && bytes[8] == 8 && bytes[15] == 1 && bytes[16] == 7);
    CHECK(bytes[20] == 1 && bytes[21] == 1 && bytes[32] == 3 && bytes[40] == 2);
    CHECK(bytes[64] == 0 && bytes[65] == 0x80 && bytes[66] == 0xff &&
          bytes[192] == 0x34);
    CHECK(store_page_decode(7, UINT64_C(0x0102030405060708), bytes, &out) ==
          STORE_OK);
    CHECK(!memcmp(&node, &out, sizeof node));
    memset(node.key[7], 0xaa, sizeof node.key[7]);
    memset(node.value[7], 0xbb, sizeof node.value[7]);
    node.klen[7] = 16;
    node.vlen[7] = 64;
    node.child[8] = 31;
    memset(node.key[0] + 3, 0xcc, STORE_KEY_MAX - 3);
    memset(node.value[0] + 2, 0xdd, STORE_VALUE_MAX - 2);
    CHECK(store_page_encode(7, UINT64_C(0x0102030405060708), &node, before) ==
          STORE_OK);
    CHECK(!memcmp(bytes, before, sizeof bytes));
    old = out;
    bytes[64] ^= 1;
    CHECK(store_page_decode(7, UINT64_C(0x0102030405060708), bytes, &out) ==
          STORE_CORRUPT);
    CHECK(!memcmp(&old, &out, sizeof old));
    bytes[64] ^= 1;
    CHECK(store_page_decode(8, UINT64_C(0x0102030405060708), bytes, &out) ==
          STORE_CORRUPT);
    CHECK(store_page_decode(7, 0, bytes, &out) == STORE_CORRUPT);
    memcpy(before, bytes, sizeof bytes);
    node.n = 8;
    CHECK(store_page_encode(7, 0, &node, bytes) == STORE_INVALID &&
          !memcmp(before, bytes, sizeof bytes));
    node.n = 1;
    node.klen[0] = 17;
    CHECK(store_page_encode(7, 0, &node, bytes) == STORE_INVALID);
    node.klen[0] = 3;
    node.vlen[0] = 65;
    CHECK(store_page_encode(7, 0, &node, bytes) == STORE_INVALID);
    CHECK(store_page_decode(32, 0, bytes, &out) == STORE_INVALID);
    return 0;
}

static int initial_image_test(void)
{
    memset(&disk, 0, sizeof disk);
    CHECK(store_format(mio(&disk)) == STORE_OK);
    CHECK(disk.stable[0][4] == 20 && disk.stable[1][4] == 20);
    CHECK(disk.stable[2][4] == 10 && disk.stable[2][20] == 1 &&
          disk.stable[2][24] == 3);
    store_node root;
    CHECK(store_page_decode(1, 0, disk.stable[3], &root) == STORE_OK &&
          root.leaf && root.n == 0);
    for (unsigned p = 0; p < STORE_IMAGE_PAGES; ++p)
        CHECK(!disk.dirty[p]);
    store_io io = mio(&disk);
    io.page_count = STORE_IMAGE_PAGES - 1;
    CHECK(store_format(io) == STORE_INVALID);
    return 0;
}

static int cache_test(void)
{
    store s;
    uint8_t bytes[STORE_PAGE_SIZE];
    component_state(&s, 0);
    memset(disk.stable[3], 0x11, STORE_PAGE_SIZE);
    memset(disk.stable[7], 0x55, STORE_PAGE_SIZE);
    memset(disk.stable[35], 0x77, STORE_PAGE_SIZE);
    CHECK(store_page_read(&s, 1, bytes) == STORE_OK && bytes[0] == 0x11 &&
          s.reads == 1);
    CHECK(store_page_read(&s, 1, bytes) == STORE_OK && s.cache_hits == 1 &&
          s.reads == 1);
    CHECK(store_page_read(&s, 5, bytes) == STORE_OK && bytes[0] == 0x55 &&
          s.reads == 2);
    CHECK(store_page_read(&s, 1, bytes) == STORE_OK && bytes[0] == 0x11 &&
          s.reads == 3);
    s.wal_active = 1;
    CHECK(store_page_read(&s, 1, bytes) == STORE_OK && bytes[0] == 0x77 &&
          s.reads == 4);
    disk.read_fail = 1;
    memset(bytes, 0xa5, sizeof bytes);
    CHECK(store_page_read(&s, 2, bytes) == STORE_IO && bytes[0] == 0xa5 &&
          bytes[4095] == 0xa5);
    CHECK(store_page_read(&s, 32, bytes) == STORE_INVALID);
    CHECK(disk.event == 0);
    return 0;
}

static int lookup_test(void)
{
    store s;
    unsigned value;
    uint8_t lower[4];
    scan_state scan = {0};
    component_state(&s, 0);
    s.root = 3;
    s.used = 15;
    for (unsigned leaf = 1; leaf <= 2; ++leaf)
    {
        store_node n = {0};
        n.leaf = 1;
        n.n = 4;
        n.next = leaf == 1 ? 2 : 0;
        for (unsigned i = 0; i < 4; ++i)
        {
            unsigned k = (leaf - 1) * 4 + i;
            n.klen[i] = n.vlen[i] = 4;
            key(k, n.key[i]);
            key(k + 100, n.value[i]);
        }
        CHECK(store_page_encode(leaf, 0, &n, disk.stable[2 + leaf]) ==
              STORE_OK);
    }
    store_node root = {0};
    root.n = 1;
    root.klen[0] = 4;
    key(4, root.key[0]);
    root.child[0] = 1;
    root.child[1] = 2;
    CHECK(store_page_encode(3, 0, &root, disk.stable[5]) == STORE_OK);
    for (unsigned k = 0; k < 8; ++k)
        CHECK(get(&s, k, &value) == STORE_OK && value == k + 100);
    value = 999;
    CHECK(get(&s, 9, &value) == STORE_NOT_FOUND && value == 999);
    key(3, lower);
    CHECK(store_scan(&s, lower, 4, collect, &scan) == STORE_OK &&
          scan.count == 5 && !scan.bad);
    CHECK(disk.event == 0);
    return 0;
}

static int insertion_test(void)
{
    store s;
    unsigned value;
    scan_state scan = {0};
    component_state(&s, 1);
    for (unsigned i = 0; i < 92; ++i)
    {
        unsigned k = (i * 37) % 92;
        CHECK(put(&s, k, k + 100) == STORE_OK);
        CHECK(store_check(&s) == STORE_OK);
    }
    for (unsigned i = 0; i < 92; ++i)
        CHECK(get(&s, i, &value) == STORE_OK && value == i + 100);
    CHECK(put(&s, 46, 999) == STORE_OK && get(&s, 46, &value) == STORE_OK &&
          value == 999);
    CHECK(store_scan(&s, 0, 0, collect, &scan) == STORE_OK &&
          scan.count == 92 && !scan.bad);
    CHECK(disk.event == 0);
    return 0;
}

static int deletion_test(void)
{
    store s;
    component_state(&s, 1);
    for (unsigned i = 0; i < 92; ++i)
        CHECK(put(&s, i, i) == STORE_OK);
    uint32_t peak = s.used;
    for (unsigned i = 0; i < 92; ++i)
    {
        CHECK(del(&s, (i * 37) % 92) == STORE_OK);
        CHECK(store_check(&s) == STORE_OK);
    }
    CHECK(s.used != peak && del(&s, 0) == STORE_NOT_FOUND);
    for (unsigned i = 0; i < 92; ++i)
        CHECK(put(&s, i, i) == STORE_OK);
    CHECK(store_check(&s) == STORE_OK && disk.event == 0);
    return 0;
}

static int transaction_test(void)
{
    store s;
    unsigned value;
    component_state(&s, 0);
    CHECK(store_page_encode(1, 0, &s.scratch[1], disk.stable[3]) == STORE_OK);
    CHECK(store_begin(&s) == STORE_OK);
    CHECK(put(&s, 3, 33) == STORE_OK && get(&s, 3, &value) == STORE_OK &&
          value == 33);
    store_abort(&s);
    CHECK(get(&s, 3, &value) == STORE_NOT_FOUND);
    CHECK(store_begin(&s) == STORE_OK);
    int rc = STORE_OK;
    unsigned k = 0;
    for (; k < 1000 && rc == STORE_OK; ++k)
        rc = put(&s, k, k);
    CHECK(rc == STORE_FULL && !s.active &&
          get(&s, 0, &value) == STORE_NOT_FOUND);
    CHECK(disk.event == 0);
    return 0;
}

static int published_component(store *s)
{
    component_state(s, 0);
    CHECK(store_page_encode(1, 0, &s->scratch[1], disk.stable[3]) == STORE_OK);
    CHECK(store_begin(s) == STORE_OK && put(s, 3, 33) == STORE_OK);
    return 0;
}

static int publish_test(void)
{
    store s;
    unsigned value;
    CHECK(published_component(&s) == 0);
    CHECK(store_commit(&s) == STORE_OK && s.generation == 1 && s.wal_active &&
          !s.active);
    CHECK(disk.stable[66][4] == 30 && disk.stable[66][8] == 1);
    for (unsigned p = 0; p < STORE_IMAGE_PAGES; ++p)
        CHECK(!disk.dirty[p]);
    CHECK(get(&s, 3, &value) == STORE_OK && value == 33);
    unsigned last = disk.event;
    CHECK(published_component(&s) == 0);
    disk.cut = last;
    disk.mode = 4;
    CHECK(store_commit(&s) == STORE_INDETERMINATE && s.poisoned);
    CHECK(disk.stable[66][4] ==
          30); /* Error does not imply absence of durable commit. */
    return 0;
}

static int checkpoint_order_test(void)
{
    store s;
    CHECK(published_component(&s) == 0);
    CHECK(store_commit(&s) == STORE_OK);
    uint8_t images[STORE_DATA_PAGES][STORE_PAGE_SIZE];
    memcpy(images, disk.stable[34], sizeof images);
    CHECK(store_checkpoint(&s) == STORE_OK && !s.wal_active &&
          s.checkpoint_generation == 1);
    CHECK(!memcmp(images, disk.stable[2], sizeof images));
    CHECK(disk.stable[1][4] == 20 && disk.stable[1][8] == 1);
    CHECK(disk.stable[66][0] == 0 && disk.stable[66][4] == 0);
    for (unsigned p = 0; p < STORE_IMAGE_PAGES; ++p)
        CHECK(!disk.dirty[p]);
    return 0;
}

static int tree_test(void)
{
    store s;
    unsigned i, v;
    uint32_t peak;
    scan_state c = {0};
    CHECK(seed(&s) == 0);
    CHECK(store_begin(&s) == STORE_OK);
    for (i = 0; i < 92; i++)
    {
        CHECK(put(&s, i, i + 100) == STORE_OK);
        CHECK(store_check(&s) == STORE_OK);
    }
    peak = s.used;
    for (i = 0; i < 92; i++)
        CHECK(get(&s, i, &v) == STORE_OK && v == i + 100);
    CHECK(store_scan(&s, NULL, 0, collect, &c) == STORE_OK && c.count == 92 &&
          !c.bad);
    CHECK(store_commit(&s) == STORE_OK);
    CHECK(store_checkpoint(&s) == STORE_OK);
    crash(&disk);
    CHECK(store_open(&s, mio(&disk)) == STORE_OK);
    CHECK(store_begin(&s) == STORE_OK);
    for (i = 0; i < 92; i++)
    {
        unsigned k = (i * 37) % 92;
        CHECK(del(&s, k) == STORE_OK);
        CHECK(store_check(&s) == STORE_OK);
    }
    CHECK(s.used != peak);
    CHECK(store_commit(&s) == STORE_OK);
    CHECK(store_begin(&s) == STORE_OK);
    for (i = 0; i < 92; i++)
        CHECK(put(&s, i, i) == STORE_OK);
    CHECK(store_check(&s) == STORE_OK);
    store_abort(&s);
    CHECK(get(&s, 0, &v) == STORE_NOT_FOUND);
    return 0;
}

static int differential_test(void)
{
    store s;
    unsigned state = 12345, i, j, v, k, op, values[80] = {0};
    uint8_t present[80] = {0};
    CHECK(seed(&s) == 0);
    CHECK(store_begin(&s) == STORE_OK);
    for (i = 0; i < 800; i++)
    {
        state = state * 1664525u + 1013904223u;
        k = (state >> 16) % 80;
        op = state & 3u;
        if (op == 0)
        {
            int r = del(&s, k);
            CHECK(r == (present[k] ? STORE_OK : STORE_NOT_FOUND));
            present[k] = 0;
        }
        else
        {
            values[k] = state;
            CHECK(put(&s, k, state) == STORE_OK);
            present[k] = 1;
        }
        CHECK(store_check(&s) == STORE_OK);
        if (i % 13 == 0)
            for (j = 0; j < 80; j++)
            {
                int r = get(&s, j, &v);
                CHECK(r == (present[j] ? STORE_OK : STORE_NOT_FOUND));
                if (present[j])
                    CHECK(v == values[j]);
            }
    }
    CHECK(store_commit(&s) == STORE_OK);
    crash(&disk);
    CHECK(store_open(&s, mio(&disk)) == STORE_OK);
    for (j = 0; j < 80; j++)
    {
        int r = get(&s, j, &v);
        CHECK(r == (present[j] ? STORE_OK : STORE_NOT_FOUND));
        if (present[j])
            CHECK(v == values[j]);
    }
    return 0;
}

static int old_state(store *s)
{
    unsigned v;
    return get(s, 1, &v) == STORE_OK && v == 10 && get(s, 2, &v) == STORE_OK &&
           v == 20 && get(s, 3, &v) == STORE_NOT_FOUND;
}

static int new_state(store *s)
{
    unsigned v;
    return get(s, 1, &v) == STORE_OK && v == 11 &&
           get(s, 2, &v) == STORE_NOT_FOUND && get(s, 3, &v) == STORE_OK &&
           v == 30;
}

static int prepare_base(void)
{
    store s;
    CHECK(seed(&s) == 0);
    CHECK(store_begin(&s) == STORE_OK);
    CHECK(put(&s, 1, 10) == STORE_OK && put(&s, 2, 20) == STORE_OK);
    CHECK(store_commit(&s) == STORE_OK);
    CHECK(store_checkpoint(&s) == STORE_OK);
    base = disk;
    return 0;
}

static int modify(store *s)
{
    CHECK(store_begin(s) == STORE_OK);
    CHECK(put(s, 1, 11) == STORE_OK);
    CHECK(del(s, 2) == STORE_OK);
    CHECK(put(s, 3, 30) == STORE_OK);
    return 0;
}

static int crash_test(void)
{
    store s;
    unsigned cut, mode, max;
    int r;
    CHECK(prepare_base() == 0);
    disk = base;
    CHECK(store_open(&s, mio(&disk)) == STORE_OK);
    CHECK(modify(&s) == 0);
    disk.event = 0;
    CHECK(store_commit(&s) == STORE_OK);
    max = disk.event;
    crash(&disk);
    CHECK(store_open(&s, mio(&disk)) == STORE_OK && new_state(&s));
    for (mode = 0; mode < 5; mode++)
        for (cut = 1; cut <= max; cut++)
        {
            disk = base;
            CHECK(store_open(&s, mio(&disk)) == STORE_OK);
            CHECK(modify(&s) == 0);
            disk.event = 0;
            disk.cut = cut;
            disk.mode = mode;
            r = store_commit(&s);
            CHECK(r == STORE_IO || r == STORE_INDETERMINATE || r == STORE_OK);
            crash(&disk);
            CHECK(store_open(&s, mio(&disk)) == STORE_OK);
            CHECK(store_check(&s) == STORE_OK);
            CHECK(old_state(&s) || new_state(&s));
            if (r == STORE_OK)
                CHECK(new_state(&s));
            if (r == STORE_IO)
                CHECK(old_state(&s));
        }
    printf("commit crash schedules: %u; all five persistence modes\n", max * 5);
    return 0;
}

static int checkpoint_test(void)
{
    store s;
    unsigned max, cut, mode;
    CHECK(prepare_base() == 0);
    disk = base;
    CHECK(store_open(&s, mio(&disk)) == STORE_OK);
    CHECK(modify(&s) == 0);
    CHECK(store_commit(&s) == STORE_OK);
    snap = disk;
    disk.event = 0;
    CHECK(store_checkpoint(&s) == STORE_OK);
    max = disk.event;
    for (mode = 0; mode < 5; mode++)
        for (cut = 1; cut <= max; cut++)
        {
            disk = snap;
            CHECK(
                store_open(&s, mio(&disk)) ==
                STORE_OK); /* opening recovers; restore pending committed state for
                              explicit checkpoint */
            disk = snap;
            memset(&s, 0, sizeof(s));
            s.io = mio(&disk);
            s.generation = 2;
            s.checkpoint_generation = 1;
            s.wal_active = 1;
            disk.event = 0;
            disk.cut = cut;
            disk.mode = mode;
            (void)store_checkpoint(&s);
            crash(&disk);
            CHECK(store_open(&s, mio(&disk)) == STORE_OK);
            CHECK(new_state(&s) && store_check(&s) == STORE_OK);
        }
    printf("checkpoint crash schedules: %u\n", max * 5);
    return 0;
}

static int recovery_test(void)
{
    store s;
    unsigned max, cut, mode;
    CHECK(prepare_base() == 0);
    disk = base;
    CHECK(store_open(&s, mio(&disk)) == STORE_OK);
    CHECK(modify(&s) == 0);
    CHECK(store_commit(&s) == STORE_OK);
    crash(&disk);
    snap = disk;
    disk.event = 0;
    CHECK(store_open(&s, mio(&disk)) == STORE_OK);
    max = disk.event;
    for (mode = 0; mode < 5; mode++)
        for (cut = 1; cut <= max; cut++)
        {
            disk = snap;
            disk.event = 0;
            disk.cut = cut;
            disk.mode = mode;
            (void)store_open(&s, mio(&disk));
            crash(&disk);
            CHECK(store_open(&s, mio(&disk)) == STORE_OK && new_state(&s));
        }
    printf("recovery crash schedules: %u\n", max * 5);
    return 0;
}

static int corruption_test(void)
{
    store s;
    unsigned v, i;
    CHECK(prepare_base() == 0);
    disk = base;
    disk.stable[2][20] ^= 1;
    CHECK(store_open(&s, mio(&disk)) == STORE_CORRUPT);
    disk = base;
    disk.stable[0][4095] ^= 1;
    CHECK(store_open(&s, mio(&disk)) == STORE_OK && old_state(&s));
    disk = base;
    CHECK(store_open(&s, mio(&disk)) == STORE_OK);
    CHECK(modify(&s) == 0);
    CHECK(store_commit(&s) == STORE_OK);
    crash(&disk);
    disk.stable[34][100] ^= 1;
    CHECK(store_open(&s, mio(&disk)) == STORE_CORRUPT);
    disk = base;
    CHECK(store_open(&s, mio(&disk)) == STORE_OK);
    CHECK(get(&s, 1, &v) == STORE_OK);
    for (i = 0; i < 10; i++)
        CHECK(get(&s, 1, &v) == STORE_OK);
    CHECK(s.cache_hits > 0);
    return 0;
}

static int capacity_test(void)
{
    store s;
    unsigned i, v;
    int r = 0;
    CHECK(seed(&s) == 0);
    CHECK(store_begin(&s) == STORE_OK);
    for (i = 0; i < 1000; i++)
    {
        r = put(&s, i, i);
        if (r)
            break;
    }
    CHECK(r == STORE_FULL && i < 1000);
    CHECK(!s.active && get(&s, 0, &v) == STORE_NOT_FOUND);
    CHECK(store_check(&s) == STORE_OK);
    return 0;
}

static int file_test(void)
{
#if defined(__unix__)
    store s;
    store_file f;
    char path[] = "/tmp/cmastery-storage-XXXXXX";
    int fd = mkstemp(path);
    unsigned v;
    CHECK(fd >= 0);
    CHECK(ftruncate(fd, (off_t)STORE_IMAGE_PAGES * STORE_PAGE_SIZE) == 0);
    CHECK(fsync(fd) == 0);
    close(fd);
    CHECK(store_file_open(&f, path, STORE_IMAGE_PAGES) == STORE_OK);
    CHECK(store_format(store_file_io(&f)) == STORE_OK);
    CHECK(store_open(&s, store_file_io(&f)) == STORE_OK);
    CHECK(store_begin(&s) == STORE_OK);
    CHECK(put(&s, 9, 81) == STORE_OK);
    CHECK(store_commit(&s) == STORE_OK);
    store_file_close(&f);
    CHECK(store_file_open(&f, path, STORE_IMAGE_PAGES) == STORE_OK);
    CHECK(store_open(&s, store_file_io(&f)) == STORE_OK);
    CHECK(get(&s, 9, &v) == STORE_OK && v == 81);
    store_file_close(&f);
    CHECK(unlink(path) == 0);
    return 0;
#else
    fprintf(stderr, "POSIX file adapter unavailable\n");
    return 77;
#endif
}
#if defined(STORAGE_WITH_RUNTIME)
static enum cvm_status vm_call(store *s, uint8_t service, int32_t arg,
                               int32_t *out)
{
    struct cvm_cvm vm;
    struct cvm_program p = {0};
    struct cvm_value v;
    enum cvm_status status;
    uint32_t u = (uint32_t)arg;
    unsigned i;
    p.count = 1;
    p.length = 8;
    p.functions[0].length = 8;
    p.code[0] = OP_INT;
    for (i = 0; i < 4; i++)
        p.code[i + 1] = (uint8_t)(u >> (8 * i));
    p.code[5] = OP_HOST;
    p.code[6] = service;
    p.code[7] = OP_RET;
    cvm_init(&vm, 100, 100, 2);
    vm.host = cvm_store_host;
    vm.host_context = s;
    status = cvm_run(&vm, &p, &v);
    if (status == CVM_OK)
    {
        if (v.tag != CVM_INT)
            return CVM_TYPE;
        *out = v.integer;
    }
    return status;
}

static int vm_test(void)
{
    store s;
    int32_t out = 0;
    unsigned events;
    CHECK(seed(&s) == 0);
    base = disk;
    disk.event = 0;
    CHECK(vm_call(&s, 1, -91, &out) == CVM_OK && out == -91);
    events = disk.event;
    crash(&disk);
    CHECK(store_open(&s, mio(&disk)) == STORE_OK);
    CHECK(vm_call(&s, 2, 0, &out) == CVM_OK && out == -91);
    disk = base;
    CHECK(store_open(&s, mio(&disk)) == STORE_OK);
    disk.event = 0;
    disk.cut = events;
    disk.mode = 4;
    CHECK(vm_call(&s, 1, -99, &out) == CVM_HOST_ERROR);
    crash(&disk);
    CHECK(store_open(&s, mio(&disk)) == STORE_OK);
    CHECK(vm_call(&s, 2, 0, &out) == CVM_OK && out == -99);
    puts("actual VM bytecode -> host callback -> committed/recovered store; "
         "durable commit with lost response propagated as host error");
    return 0;
}
#endif
int main(int argc, char **argv)
{
    const char *n = argc == 2 ? argv[1] : "";
    if (!strcmp(n, "codec"))
        return codec_test();
    if (!strcmp(n, "initial_image"))
        return initial_image_test();
    if (!strcmp(n, "cache"))
        return cache_test();
    if (!strcmp(n, "lookup"))
        return lookup_test();
    if (!strcmp(n, "insertion"))
        return insertion_test();
    if (!strcmp(n, "deletion"))
        return deletion_test();
    if (!strcmp(n, "transaction"))
        return transaction_test();
    if (!strcmp(n, "publish"))
        return publish_test();
    if (!strcmp(n, "checkpoint_order"))
        return checkpoint_order_test();
    if (!strcmp(n, "format"))
        return format_test();
    if (!strcmp(n, "tree"))
        return tree_test();
    if (!strcmp(n, "differential"))
        return differential_test();
    if (!strcmp(n, "crash"))
        return crash_test();
    if (!strcmp(n, "checkpoint"))
        return checkpoint_test();
    if (!strcmp(n, "recovery"))
        return recovery_test();
    if (!strcmp(n, "corruption"))
        return corruption_test();
    if (!strcmp(n, "capacity"))
        return capacity_test();
    if (!strcmp(n, "file"))
        return file_test();
#if defined(STORAGE_WITH_RUNTIME)
    if (!strcmp(n, "vm"))
        return vm_test();
#endif
    fprintf(stderr, "unknown case\n");
    return 2;
}
