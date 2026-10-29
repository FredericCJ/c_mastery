#include "refs.h"
#define main tree_main
#include "tree_cases.c"
#undef main

static int fill(store *s, unsigned count)
{
    for (unsigned i = 0; i < 128; i++)
    {
        model[i] = -1;
    }
    for (unsigned i = 0; i < count; i++)
    {
        CHECK(update(s, i, (int)i) == STORE_OK);
    }
    CHECK(verify(s) == 0);
    return 0;
}

static int clear(store *s, unsigned count)
{
    CHECK(store_begin(s) == STORE_OK);
    for (unsigned i = 0; i < count; i++)
    {
        CHECK(update(s, i, -1) == STORE_OK && verify(s) == 0);
    }
    CHECK(store_commit(s) == STORE_OK && s->used == 3);
    return 0;
}

static int reuse(void)
{
    store s, other;
    CHECK(empty(&s) == 0 && fill(&s, 8) == 0 && store_commit(&s) == STORE_OK);
    struct store_ref old, now;
    CHECK(store_ref_make(&s, 2, &old) == STORE_OK && old.incarnation == 1);
    store_node out, before;
    memset(&out, 0xa5, sizeof out);
    before = out;
    CHECK(clear(&s, 8) == 0 && store_ref_read(old, &out) == STORE_STALE &&
          memcmp(&out, &before, sizeof out) == 0);
    CHECK(store_begin(&s) == STORE_OK && fill(&s, 8) == 0 &&
          store_commit(&s) == STORE_OK);
    CHECK(store_ref_make(&s, 2, &now) == STORE_OK && now.incarnation == 2 &&
          store_ref_read(old, &out) == STORE_STALE &&
          memcmp(&out, &before, sizeof out) == 0 &&
          store_ref_read(now, &out) == STORE_OK && out.incarnation == 2);
    CHECK(store_checkpoint(&s) == STORE_OK &&
          store_open(&other, s.io) == STORE_OK && other.incarnations[2] == 2);
    puts("REF reuse validated same-page-new-incarnation");
    return 0;
}

static int retirement(void)
{
    store s, r;
    CHECK(empty(&s) == 0);
    s.incarnations[2] = UINT64_MAX - 1;
    CHECK(store_commit(&s) == STORE_OK && store_begin(&s) == STORE_OK &&
          fill(&s, 8) == 0 && store_commit(&s) == STORE_OK &&
          s.incarnations[2] == UINT64_MAX);
    struct store_ref old;
    CHECK(store_ref_make(&s, 2, &old) == STORE_OK &&
          old.incarnation == UINT64_MAX);
    CHECK(clear(&s, 8) == 0 && store_begin(&s) == STORE_OK &&
          fill(&s, 8) == 0 && store_commit(&s) == STORE_OK);
    CHECK(!(s.used & (UINT32_C(1) << 2)) && (s.used & (UINT32_C(1) << 3)) &&
          s.incarnations[2] == UINT64_MAX && s.incarnations[3] == 2);
    store_node out;
    CHECK(store_ref_read(old, &out) == STORE_STALE &&
          store_checkpoint(&s) == STORE_OK &&
          store_open(&r, s.io) == STORE_OK && r.incarnations[2] == UINT64_MAX &&
          store_check(&r) == STORE_OK);
    puts("REF retirement validated skip-exhausted-slot");
    return 0;
}

static uint32_t checksum(const uint8_t *p, size_t n)
{
    uint32_t c = UINT32_MAX;
    for (size_t i = 0; i < n; i++)
    {
        c ^= p[i];
        for (unsigned j = 0; j < 8; j++)
        {
            c = (c >> 1) ^ (UINT32_C(0xedb88320) & (0u - (c & 1u)));
        }
    }
    return ~c;
}

static void seal_page(uint8_t *p)
{
    uint32_t c = checksum(p, 4092);
    for (unsigned j = 0; j < 4; j++)
    {
        p[4092 + j] = (uint8_t)(c >> (8 * j));
    }
}

static int stale(void)
{
    store s, bad;
    CHECK(empty(&s) == 0 && fill(&s, 20) == 0);
    store_node *root = &s.scratch[s.root];
    uint64_t saved = root->child_incarnation[0];
    root->child_incarnation[0]++;
    uint8_t key = 0, value = 0xa5;
    size_t n = 777;
    CHECK(store_get(&s, &key, 1, &value, 1, &n) == STORE_CORRUPT &&
          value == 0xa5 && n == 777 && store_check(&s) == STORE_CORRUPT);
    root->child_incarnation[0] = saved;
    uint32_t leaf = root->child[0];
    while (!s.scratch[leaf].leaf)
    {
        leaf = s.scratch[leaf].child[0];
    }
    s.scratch[leaf].next_incarnation++;
    CHECK(store_get(&s, &key, 1, &value, 1, &n) == STORE_CORRUPT);
    s.scratch[leaf].next_incarnation--;
    s.root_incarnation++;
    CHECK(store_check(&s) == STORE_CORRUPT);
    s.root_incarnation--;
    s.scratch[leaf].incarnation++;
    CHECK(store_check(&s) == STORE_CORRUPT);
    s.scratch[leaf].incarnation--;
    CHECK(verify(&s) == 0 && store_commit(&s) == STORE_OK &&
          store_checkpoint(&s) == STORE_OK);
    uint8_t *pagebytes = disk[2 + s.root];
    pagebytes[784] ^= 2;
    seal_page(pagebytes);
    CHECK(store_open(&bad, s.io) == STORE_CORRUPT && bad.poisoned);
    puts("REF stale validated root-child-next-own-and-resealed-page");
    return 0;
}

static int rollback(void)
{
    store s, r;
    CHECK(empty(&s) == 0 && fill(&s, 7) == 0 && store_commit(&s) == STORE_OK);
    CHECK(store_begin(&s) == STORE_OK);
    allocations = 0;
    fail_at = 2;
    uint8_t key = 7, value = 7;
    CHECK(store_put(&s, &key, 1, &value, 1) == STORE_FULL && !s.active &&
          s.incarnations[2] == 0 && s.root_incarnation == 1);
    fail_at = 0;
    CHECK(store_begin(&s) == STORE_OK &&
          store_put(&s, &key, 1, &value, 1) == STORE_OK);
    model[7] = 7;
    struct store_ref out = {0}, before = out;
    CHECK(store_ref_make(&s, 2, &out) == STORE_STATE &&
          memcmp(&out, &before, sizeof out) == 0);
    CHECK(store_commit(&s) == STORE_OK && s.incarnations[2] == 1 &&
          clear(&s, 8) == 0 && store_checkpoint(&s) == STORE_OK &&
          store_open(&r, s.io) == STORE_OK && r.incarnations[2] == 1);
    CHECK(store_begin(&r) == STORE_OK && fill(&r, 8) == 0 &&
          store_commit(&r) == STORE_OK && r.incarnations[2] == 2);
    puts(
        "REF rollback validated unpublished-only-and-persistent-free-counters");
    return 0;
}

static int tagged_page(void)
{
    store_node n = {0};
    n.leaf = 1;
    n.n = 1;
    n.klen[0] = 1;
    n.vlen[0] = 1;
    n.key[0][0] = 0x80;
    n.value[0][0] = 0xa5;
    n.incarnation = 19;
    uint8_t bytes[4096];
    CHECK(store_page_encode(1, 7, &n, bytes) == STORE_OK);
    for (unsigned i = 0; i < 4096; i++)
    {
        printf("%02x", bytes[i]);
    }
    putchar('\n');
    return 0;
}

static int meta(void)
{
    store s;
    CHECK(empty(&s) == 0);
    for (unsigned i = 0; i < 4096; i++)
    {
        printf("%02x", disk[2][i]);
    }
    putchar('\n');
    return 0;
}

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        return 2;
    }
    if (!strcmp(argv[1], "reuse"))
    {
        return reuse();
    }
    if (!strcmp(argv[1], "retirement"))
    {
        return retirement();
    }
    if (!strcmp(argv[1], "stale"))
    {
        return stale();
    }
    if (!strcmp(argv[1], "rollback"))
    {
        return rollback();
    }
    if (!strcmp(argv[1], "page"))
    {
        return tagged_page();
    }
    if (!strcmp(argv[1], "meta"))
    {
        return meta();
    }
    return tree_main(argc, argv);
}
