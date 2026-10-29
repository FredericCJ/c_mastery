#include "codec.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define CHECK(x)                                                      \
    do                                                                \
    {                                                                 \
        if (!(x))                                                     \
        {                                                             \
            fprintf(stderr, "SFC_CHECK line %d: %s\n", __LINE__, #x); \
            return 1;                                                 \
        }                                                             \
    } while (0)
#define GEN UINT64_C(0x0102030405060708)

static void w16(uint8_t *b, unsigned x)
{
    b[0] = (uint8_t)x;
    b[1] = (uint8_t)(x >> 8);
}

static void w32(uint8_t *b, uint32_t x)
{
    for (unsigned i = 0; i < 4; i++)
        b[i] = (uint8_t)(x >> (8 * i));
}

static void seal(uint8_t *b)
{
    uint32_t c = UINT32_MAX;
    for (unsigned i = 0; i < 4092; i++)
    {
        c ^= b[i];
        for (unsigned j = 0; j < 8; j++)
            c = (c >> 1) ^ (UINT32_C(0xedb88320) & (0u - (c & 1u)));
    }
    w32(b + 4092, ~c);
}

static store_node sample(unsigned n)
{
    store_node s = {0};
    s.leaf = 1;
    s.n = (uint8_t)n;
    s.next = n ? 31 : 0;
    for (unsigned i = 0; i < n; i++)
    {
        s.klen[i] = 16;
        s.vlen[i] = 64;
        for (unsigned j = 0; j < 16; j++)
            s.key[i][j] = (uint8_t)(16 * i + j);
        for (unsigned j = 0; j < 64; j++)
            s.value[i][j] = (uint8_t)(i + j);
    }
    return s;
}

static int same_node(const store_node *a, const store_node *b)
{
    return a->leaf == b->leaf && a->n == b->n && a->next == b->next &&
           memcmp(a->klen, b->klen, sizeof a->klen) == 0 &&
           memcmp(a->vlen, b->vlen, sizeof a->vlen) == 0 &&
           memcmp(a->child, b->child, sizeof a->child) == 0 &&
           memcmp(a->key, b->key, sizeof a->key) == 0 &&
           memcmp(a->value, b->value, sizeof a->value) == 0;
}

static int original(void)
{
    uint8_t b[4096], unaligned[4097], bad[4096];
    store_node n = sample(7), out, before;
    CHECK(store_page_encode(7, GEN, &n, b) == STORE_OK);
    memcpy(unaligned + 1, b, 4096);
    CHECK(store_page_decode(7, GEN, unaligned + 1, &out) == STORE_OK);
    CHECK(same_node(&n, &out));
    const unsigned offsets[] = {0, 8, 16, 21, 32, 40, 24};
    const unsigned values[] = {0, 0, 32, 8, 17, 65, 32};
    for (unsigned i = 0; i < 7; i++)
    {
        memcpy(bad, b, 4096);
        bad[offsets[i]] = (uint8_t)values[i];
        seal(bad);
        memset(&out, 0xa5, sizeof out);
        before = out;
        CHECK(store_page_decode(7, GEN, bad, &out) == STORE_CORRUPT);
        CHECK(memcmp(&out, &before, sizeof out) == 0);
    }
    n = sample(1);
    n.leaf = 0;
    n.next = 0;
    n.child[0] = 1;
    n.child[1] = 2;
    CHECK(store_page_encode(7, GEN, &n, b) == STORE_OK);
    w32(b + 708, 32);
    seal(b);
    memset(&out, 0xa5, sizeof out);
    before = out;
    CHECK(store_page_decode(7, GEN, b, &out) == STORE_CORRUPT);
    CHECK(memcmp(&out, &before, sizeof out) == 0);
    /* Every gate exercises the learner's own new implementation too. */
    n = sample(0);
    CHECK(sf_leaf_encode(7, GEN, &n, b) == STORE_OK);
    puts("SFC original validated");
    return 0;
}

static int migrate(void)
{
    uint8_t old[4096], newer[4096], again[4096], unaligned[4097];
    store_node n, out;
    for (unsigned count = 0; count <= 7; count++)
    {
        n = sample(count);
        CHECK(store_page_encode(7, GEN, &n, old) == STORE_OK);
        CHECK(sf_leaf_upgrade(7, GEN, old, 4096, newer) == STORE_OK);
        CHECK(newer[4] == 3 && newer[8] == 8 && newer[15] == 1);
        CHECK(sf_leaf_downgrade(7, GEN, newer, 4096, again) == STORE_OK);
        CHECK(memcmp(old, again, 4096) == 0);
        memcpy(unaligned + 1, newer, 4096);
        CHECK(sf_leaf_decode(7, GEN, unaligned + 1, 4096, &out) == STORE_OK);
        CHECK(same_node(&out, &n));
    }
    n = sample(1);
    n.leaf = 0;
    n.next = 0;
    n.child[0] = 1;
    n.child[1] = 2;
    CHECK(store_page_encode(7, GEN, &n, old) == STORE_OK);
    memset(newer, 0xa5, 4096);
    memcpy(again, newer, 4096);
    CHECK(sf_leaf_upgrade(7, GEN, old, 4096, newer) == STORE_INVALID);
    CHECK(memcmp(newer, again, 4096) == 0);
    puts("SFC migration validated");
    return 0;
}

static int variable(void)
{
    store_node n = sample(1), out, before;
    uint8_t b[4096], good[4096];
    n.klen[0] = 1;
    n.vlen[0] = 1;
    n.key[0][0] = 'A';
    n.value[0][0] = 'v';
    CHECK(sf_leaf_encode(7, GEN, &n, good) == STORE_OK);
    memcpy(b, good, 4096);
    w16(b + 36, 128);
    b[129] = 0;
    seal(b);
    memset(&out, 0xa5, sizeof out);
    before = out;
    CHECK(sf_leaf_decode(7, GEN, b, 4096, &out) == STORE_CORRUPT);
    CHECK(memcmp(&out, &before, sizeof out) == 0);
    memcpy(b, good, 4096);
    b[128] = b[129] = 0;
    w16(b + 32, 4091);
    w16(b + 36, 128);
    b[4091] = 'A';
    b[128] = 'v';
    seal(b);
    CHECK(sf_leaf_decode(7, GEN, b, 4096, &out) == STORE_OK);
    CHECK(out.key[0][0] == 'A' && out.value[0][0] == 'v');
    n.vlen[0] = 0;
    CHECK(sf_leaf_encode(7, GEN, &n, b) == STORE_OK);
    w16(b + 36, 4092);
    seal(b);
    CHECK(sf_leaf_decode(7, GEN, b, 4096, &out) == STORE_OK &&
          out.vlen[0] == 0);
    for (unsigned len = 0; len < 4096; len++)
    {
        before = out;
        CHECK(sf_leaf_decode(7, GEN, b, len, &out) == STORE_INVALID);
        CHECK(memcmp(&out, &before, sizeof out) == 0);
    }
    puts("SFC variable validated");
    return 0;
}

static int atomic_case(void)
{
    store_node n = sample(2), out, before;
    uint8_t b[4096], prior[4096];
    CHECK(sf_leaf_encode(7, GEN, &n, b) == STORE_OK);
    /* Two length16 keys, payload starts128 and208. Make equality, retain CRC. */
    memcpy(b + 208, b + 128, 16);
    seal(b);
    memset(&out, 0xa5, sizeof out);
    before = out;
    CHECK(sf_leaf_decode(7, GEN, b, 4096, &out) == STORE_CORRUPT);
    CHECK(memcmp(&out, &before, sizeof out) == 0);
    n = sample(1);
    CHECK(sf_leaf_encode(7, GEN, &n, b) == STORE_OK);
    memcpy(prior, b, 4096);
    n.klen[0] = 17;
    CHECK(sf_leaf_encode(7, GEN, &n, b) == STORE_INVALID);
    CHECK(memcmp(prior, b, 4096) == 0);

    union
    {
        store_node node;
        uint8_t bytes[4096];
    } overlap;

    memcpy(overlap.bytes, b, 4096);
    n = sample(1);
    CHECK(sf_leaf_decode(7, GEN, overlap.bytes, 4096, &overlap.node) ==
          STORE_OK);
    CHECK(same_node(&overlap.node, &n));
    CHECK(sf_leaf_encode(7, GEN, &n, b) == STORE_OK);
    b[4] = 4;
    seal(b);
    before = out;
    CHECK(sf_leaf_decode(7, GEN, b, 4096, &out) == STORE_CORRUPT);
    CHECK(memcmp(&out, &before, sizeof out) == 0);
    puts("SFC atomic validated");
    return 0;
}

struct store_image
{
    uint8_t bytes[STORE_IMAGE_PAGES][4096];
    unsigned event, fail, reads, read_fail, writes, discards, acquires;
    uint64_t identity;
    int acquired;
};
static struct store_image image, established;

static int rd(void *p, uint32_t page, void *out)
{
    struct store_image *m = p;
    if (page >= STORE_IMAGE_PAGES)
        return 1;
    m->reads++;
    if (m->read_fail == m->reads)
    {
        memcpy(out, m->bytes[page], 137);
        return 1;
    }
    memcpy(out, m->bytes[page], 4096);
    return 0;
}

static int wr(void *p, uint32_t page, const void *in)
{
    struct store_image *m = p;
    if (page >= STORE_IMAGE_PAGES)
        return 1;
    m->event++;
    m->writes++;
    if (m->event == m->fail)
    {
        memcpy(m->bytes[page], in, 137);
        return 1;
    }
    memcpy(m->bytes[page], in, 4096);
    return 0;
}

static int fl(void *p)
{
    struct store_image *m = p;
    m->event++;
    return m->event == m->fail;
}

static store_io io(struct store_image *m)
{
    store_io r = {m, rd, wr, fl, STORE_IMAGE_PAGES};
    return r;
}

static int acquire(void *p, struct sf_image *out)
{
    struct store_image *m = p;
    m->acquires++;
    if (m->acquired)
        return 0;
    m->acquired = 1;
    m->identity++;
    memset(m->bytes, 0xa5, sizeof m->bytes);
    m->event = m->writes = m->reads = 0;
    out->io = io(m);
    out->identity = m->identity;
    return 1;
}

static void discard(void *p, struct sf_image out)
{
    struct store_image *m = p;
    if (out.identity != m->identity || out.io.ctx != m || !m->acquired)
        abort();
    m->discards++;
    m->acquired = 0;
}

static int fresh(void)
{
    struct sf_factory f = {&image, acquire, discard};
    struct sf_image out = {0}, before;
    unsigned cases = 0;
    memset(&image, 0, sizeof image);
    CHECK(sf_format_fresh(f, &out) == STORE_OK);
    CHECK(image.event == 38 && out.identity == 1);
    unsigned verification_reads = image.reads;
    CHECK(verification_reads > 0);
    discard(&image, out);
    for (unsigned cut = 1; cut <= 38; cut++)
    {
        image.fail = cut;
        out = (struct sf_image){.identity = UINT64_MAX};
        before = out;
        unsigned discarded = image.discards;
        CHECK(sf_format_fresh(f, &out) == STORE_IO);
        CHECK(memcmp(&out, &before, sizeof out) == 0 &&
              image.discards == discarded + 1 && !image.acquired);
        uint64_t failed_id = image.identity;
        image.fail = 0;
        CHECK(sf_format_fresh(f, &out) == STORE_OK);
        CHECK(out.identity == failed_id + 1);
        store opened;
        CHECK(store_open(&opened, out.io) == STORE_OK && opened.root == 1 &&
              opened.used == 3);
        discard(&image, out);
        cases++;
    }
    for (unsigned read = 1; read <= verification_reads; read++)
    {
        image.read_fail = read;
        before = out;
        unsigned discarded = image.discards;
        CHECK(sf_format_fresh(f, &out) == STORE_IO &&
              image.discards == discarded + 1);
        CHECK(memcmp(&out, &before, sizeof out) == 0);
    }
    image.read_fail = 0;
    unsigned d;
    image.acquired = 1;
    before = out;
    d = image.discards;
    CHECK(sf_format_fresh(f, &out) == STORE_FULL && image.discards == d &&
          memcmp(&out, &before, sizeof out) == 0);
    image.acquired = 0;
    printf("SFC fresh validated cuts=%u reads=%u\n", cases, verification_reads);
    return 0;
}

static int reopen_case(void)
{
    memset(&established, 0, sizeof established);
    CHECK(store_format(io(&established)) == STORE_OK);
    store s;
    CHECK(store_open(&s, io(&established)) == STORE_OK);
    CHECK(store_begin(&s) == STORE_OK);
    CHECK(store_put(&s, "key", 3, "value", 5) == STORE_OK);
    CHECK(store_commit(&s) == STORE_OK);
    CHECK(store_checkpoint(&s) == STORE_OK);
    uint8_t *snapshot = malloc(sizeof established.bytes);
    CHECK(snapshot != NULL);
    memcpy(snapshot, established.bytes, sizeof established.bytes);
    for (unsigned test = 0; test < 4; test++)
    {
        memcpy(established.bytes, snapshot, sizeof established.bytes);
        established.reads = established.writes = 0;
        established.read_fail = 0;
        if (test == 0)
        {
            established.bytes[0][0] ^= 1;
            established.bytes[1][0] ^= 1;
        }
        if (test == 1)
        {
            w32(established.bytes[3] + 16, 31);
            seal(established.bytes[3]);
        }
        if (test == 2)
            established.read_fail = 1;
        if (test == 3)
            established.bytes[0][4095] ^=
                1; /* Valid newest checkpoint remains. */
        struct store_image *saved = malloc(sizeof *saved);
        CHECK(saved != NULL);
        memcpy(saved, &established, sizeof *saved);
        int r = store_open(&s, io(&established));
        CHECK(r == (test == 2   ? STORE_IO
                    : test == 3 ? STORE_OK
                                : STORE_CORRUPT));
        CHECK(established.writes == 0 && memcmp(saved->bytes, established.bytes,
                                                sizeof established.bytes) == 0);
        free(saved);
        if (test == 3)
        {
            uint8_t value[8];
            size_t n = 0;
            CHECK(store_get(&s, "key", 3, value, sizeof value, &n) ==
                      STORE_OK &&
                  n == 5 && memcmp(value, "value", 5) == 0);
        }
    }
    free(snapshot);
    struct sf_image out = {0};
    image.fail = 0;
    CHECK(sf_format_fresh((struct sf_factory){&image, acquire, discard},
                          &out) == STORE_OK);
    discard(&image, out);
    puts("SFC reopen validated");
    return 0;
}

static int oracle(void)
{
    uint8_t b[4096];
    size_t n;
    while ((n = fread(b, 1, 4096, stdin)) != 0)
    {
        if (n != 4096)
            return 2;
        store_node out, before;
        memset(&out, 0xa5, sizeof out);
        before = out;
        int r = sf_leaf_decode(7, GEN, b, 4096, &out);
        printf("%d %u %u %u %d\n", r, out.n, out.klen[0], out.vlen[0],
               memcmp(&out, &before, sizeof out) == 0);
    }
    return ferror(stdin) ? 2 : 0;
}

int main(int argc, char **argv)
{
    if (argc != 2)
        return 2;
    if (!strcmp(argv[1], "oracle"))
        return oracle();
    if (!strncmp(argv[1], "dump", 4))
    {
        store_node n = sample(strstr(argv[1], "max") ? 7 : 0);
        uint8_t b[4096];
        int r = argv[1][4] == '2' ? store_page_encode(7, GEN, &n, b)
                                  : sf_leaf_encode(7, GEN, &n, b);
        if (r)
            return 1;
        return fwrite(b, 1, 4096, stdout) == 4096 ? 0 : 2;
    }
    if (!strcmp(argv[1], "original"))
        return original();
    if (!strcmp(argv[1], "migration"))
        return migrate();
    if (!strcmp(argv[1], "variable"))
        return variable();
    if (!strcmp(argv[1], "atomic"))
        return atomic_case();
    if (!strcmp(argv[1], "fresh"))
        return fresh();
    if (!strcmp(argv[1], "reopen"))
        return reopen_case();
    return 2;
}
