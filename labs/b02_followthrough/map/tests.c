#include "map.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define CHECK(x)                                               \
    do                                                         \
    {                                                          \
        if (!(x))                                              \
        {                                                      \
            fprintf(stderr, "BF_CHECK:%d:%s\n", __LINE__, #x); \
            exit(1);                                           \
        }                                                      \
    } while (0)
static unsigned visits;

void bf_probe(void)
{
    ++visits;
    CHECK(visits <= 7);
}

static void same(const struct b2_map *a, const struct b2_map *b)
{
    CHECK(a->count == b->count);
    for (unsigned i = 0; i < 7; ++i)
    {
        CHECK(a->slots[i].state == b->slots[i].state &&
              a->slots[i].key == b->slots[i].key &&
              a->slots[i].value == b->slots[i].value);
    }
}

static int get(struct b2_map *m, uint16_t key, int *out)
{
    visits = 0;
    return b2_map_get(m, key, out);
}

static int put(struct b2_map *m, uint16_t key, int value)
{
    visits = 0;
    return b2_map_put(m, key, value);
}

static int remove_key(struct b2_map *m, uint16_t key)
{
    visits = 0;
    return b2_map_remove(m, key);
}

static void reachability(void)
{
    struct b2_map m;
    struct b2_entry scratch[7];
    b2_map_init(&m);
    CHECK(bf_map_rehash(&m, scratch, 7, 0) == B2_OK);
    CHECK(put(&m, 0, 1) == B2_OK && put(&m, 7, 2) == B2_OK &&
          remove_key(&m, 0) == B2_OK && put(&m, 7, 3) == B2_OK && m.count == 1);
    int out = 7777;
    CHECK(get(&m, 7, &out) == B2_OK && out == 3);
    b2_map_init(&m);
    for (unsigned i = 0; i < 7; ++i)
    {
        CHECK(put(&m, (uint16_t)(6 + 7 * i), (int)i) == B2_OK);
    }
    struct b2_map saved = m;
    CHECK(put(&m, 65535, 99) == B2_FULL);
    same(&m, &saved);
    CHECK(put(&m, 13, 99) == B2_OK && m.count == 7);
    for (unsigned i = 0; i < 7; ++i)
    {
        CHECK(remove_key(&m, (uint16_t)(6 + 7 * i)) == B2_OK);
    }
    out = 7777;
    CHECK(get(&m, 65535, &out) == B2_ABSENT && out == 7777);
#ifdef BF_INSTRUMENTED
    CHECK(visits == 7);
#endif
    CHECK(remove_key(&m, 13) == B2_ABSENT && put(&m, 65535, -31) == B2_OK &&
          get(&m, 65535, &out) == B2_OK && out == -31);
    puts(
        "BF reachability validated retained-three-probe-loops capacity7 full-update all-tombstones wrap65535");
}

static void rehash(void)
{
    struct b2_map m;
    struct b2_entry scratch[9];
    b2_map_init(&m);
    for (unsigned i = 0; i < 7; ++i)
    {
        CHECK(put(&m, (uint16_t)(6 + 7 * i), (int)(i + 10)) == B2_OK);
    }
    CHECK(remove_key(&m, 13) == B2_OK && remove_key(&m, 27) == B2_OK);
    struct b2_map saved = m;
    scratch[7] = (struct b2_entry){99, 99, 99};
    scratch[8] = scratch[7];
    CHECK(bf_map_rehash(&m, scratch, 6, 0) == B2_INVALID);
    same(&m, &saved);
    CHECK(bf_map_rehash(&m, scratch, 9, 8) == B2_INVALID);
    same(&m, &saved);
    for (unsigned i = 1; i <= 5; ++i)
    {
        CHECK(bf_map_rehash(&m, scratch, 9, i) == B2_FAILED);
        same(&m, &saved);
    }
    CHECK(bf_map_rehash(&m, scratch, 9, 0) == B2_OK && m.count == 5 &&
          scratch[7].state == 99 && scratch[8].key == 99);
    for (unsigned i = 0; i < 7; ++i)
    {
        CHECK(m.slots[i].state != 2);
        int out = 7777;
        int rc = get(&m, (uint16_t)(6 + 7 * i), &out);
        CHECK(rc == ((i == 1 || i == 3) ? B2_ABSENT : B2_OK));
        CHECK(out == ((i == 1 || i == 3) ? 7777 : (int)(i + 10)));
    }
    CHECK(bf_map_rehash(NULL, scratch, 7, 0) == B2_INVALID &&
          bf_map_rehash(&m, NULL, 7, 0) == B2_INVALID);
    puts(
        "BF rehash validated caller-typed-scratch five-staging-refusals no-old-publication no-resize-claim");
}

struct bf_fixed
{
    struct cd_bucket slots[16];
    unsigned inuse, fail;
};

static struct cd_bucket *take(void *v, unsigned n)
{
    struct bf_fixed *f = v;
    if (f->fail || f->inuse || n > 16)
    {
        return NULL;
    }
    f->inuse = 1;
    return f->slots;
}

static void release(void *v, struct cd_bucket *p)
{
    struct bf_fixed *f = v;
    CHECK(f->inuse && p == f->slots);
    f->inuse = 0;
}

static void keys(void)
{
    struct bf_borrow_map borrowed = {0};
    struct cd_byte_map copied = {0};
    unsigned char text[2] = {65, 0}, saved[2] = {65, 0};
    int out = 7777;
    CHECK(bf_borrow_put(&borrowed, text, 2, 9) == B2_OK &&
          bf_borrow_get(&borrowed, text, 2, &out) == B2_OK && out == 9);
    CHECK(cd_bytemap_put(&copied, text, 2, 9) == CD_OK);
    text[0] = 66;
    out = 7777;
    CHECK(bf_borrow_get(&borrowed, text, 2, &out) == B2_ABSENT && out == 7777);
    CHECK(bf_borrow_get(&borrowed, saved, 2, &out) == B2_ABSENT && out == 7777);
    CHECK(cd_bytemap_get(&copied, saved, 2, &out) == CD_OK && out == 9 &&
          cd_bytemap_get(&copied, text, 2, &out) == CD_MISSING);
    unsigned char collision[2] = {69, 0};
    CHECK(cd_bytemap_put(&copied, collision, 2, -4) == CD_OK &&
          cd_bytemap_get(&copied, saved, 2, &out) == CD_OK && out == 9 &&
          cd_bytemap_get(&copied, collision, 2, &out) == CD_OK && out == -4);
    CHECK(cd_bytemap_put(&copied, NULL, 0, 17) == CD_OK &&
          cd_bytemap_get(&copied, NULL, 0, &out) == CD_OK && out == 17);
    const unsigned char immutable[2] = {65, 0};
    struct bf_borrow_map valid = {0};
    CHECK(bf_borrow_put(&valid, immutable, 2, 31) == B2_OK &&
          bf_borrow_get(&valid, immutable, 2, &out) == B2_OK && out == 31);
    CHECK(bf_borrow_get(&valid, NULL, 1, &out) == B2_INVALID);
    puts(
        "BF keys validated live-mutable-borrow-counterexample actual-copied-provider immutable-owner embedded-zero collision");
}

static int oracle(void)
{
    unsigned n;
    char text[8];
    while (scanf("%u %7s", &n, text) == 2)
    {
        CHECK(n <= 5);
        struct b2_map m;
        b2_map_init(&m);
        struct cd_chain chain;
        cd_chain_init(&chain);
        struct bf_fixed backing = {0};
        struct cd_hash h;
        CHECK(cd_hash_init(&h, 7,
                           (struct cd_hash_allocator){&backing, take, release},
                           0) == CD_OK);
        printf("%u", n);
        for (unsigned step = 0; step < n; ++step)
        {
            unsigned op = (unsigned)(text[step] - '0');
            CHECK(op < 9);
            uint16_t key = (uint16_t)((op % 3) * 7);
            unsigned action = op / 3, probes = 0;
            int value = (int)step + 1, out = 7777, co = 7777, ho = 7777;
            int rc, cr, hr;
            if (action == 0)
            {
                rc = put(&m, key, value);
                cr = cd_chain_put(&chain, key, value);
                hr = cd_hash_put(&h, key, value, &probes);
            }
            else if (action == 1)
            {
                rc = remove_key(&m, key);
                cr = cd_chain_delete(&chain, key);
                hr = cd_hash_delete(&h, key);
            }
            else
            {
                rc = get(&m, key, &out);
                cr = cd_chain_get(&chain, key, &co, &probes);
                CHECK(probes <= 3);
                hr = cd_hash_get(&h, key, &ho, &probes);
                CHECK(co == out && ho == out);
            }
            CHECK(cr == (rc == B2_OK ? CD_OK : CD_MISSING) && hr == cr &&
                  h.live == m.count && chain.count == m.count && probes <= 7);
            printf(" %d %d %zu %u", rc, out, m.count, visits);
            for (unsigned k = 0; k < 3; ++k)
            {
                int v = 7777;
                int s = get(&m, (uint16_t)(k * 7), &v);
                CHECK(s == B2_OK || s == B2_ABSENT);
                printf(" %d", v);
            }
        }
        CHECK(cd_hash_destroy(&h) == CD_OK && !backing.inuse);
        putchar('\n');
    }
    return 0;
}

int main(int argc, char **argv)
{
    CHECK(argc == 2);
    if (!strcmp(argv[1], "reachability"))
    {
        reachability();
    }
    else if (!strcmp(argv[1], "rehash"))
    {
        rehash();
    }
    else if (!strcmp(argv[1], "keys"))
    {
        keys();
    }
    else if (!strcmp(argv[1], "oracle"))
    {
        return oracle();
    }
    else
    {
        return 2;
    }
    return 0;
}
