#include "object_runtime.h"
#include <stdio.h>
#include <string.h>
#include <limits.h>
#define CHECK(c)                                               \
    do                                                         \
    {                                                          \
        if (!(c))                                              \
        {                                                      \
            fprintf(stderr, "OR_CHECK:%d:%s\n", __LINE__, #c); \
            return 1;                                          \
        }                                                      \
    } while (0)

static int value(void)
{
    uint64_t b;
    int32_t out = 17;
    uintptr_t word;
    void *p;
    _Alignas(16) unsigned char data[16];
    CHECK(or_number_bits(UINT64_C(0x7ff9000000000042)) ==
          UINT64_C(0x7ff8000000000000));
    CHECK(or_number_bits(UINT64_C(0xfff0000000000000)) ==
          UINT64_C(0xfff0000000000000));
    CHECK(or_number_bits(UINT64_C(0x8000000000000000)) ==
          UINT64_C(0x8000000000000000));
    b = or_integer(INT32_MIN);
    CHECK(or_integer_get(b, &out) == OR_OK && out == INT32_MIN);
    CHECK(or_integer_get(or_integer(INT32_MAX), &out) == OR_OK &&
          out == INT32_MAX);
    CHECK(or_integer_get(UINT64_C(0x7ff9000100000000), &out) == OR_INVALID &&
          out == INT32_MAX);
    CHECK(or_pointer_tag(data, 3, &word) == OR_OK);
    CHECK(or_pointer_untag(word, 3, &p) == OR_OK && p == data);
    CHECK(or_pointer_tag(data + 1, 1, &word) == OR_INVALID);
    return 0;
}

static int intern(void)
{
    struct or_intern t = {0}, before;
    uint8_t a = 0, b = 99;
    unsigned char x[] = {0, 1}, y[] = {0, 2};
    CHECK(or_intern(&t, x, 2, 0, &a) == OR_OK && a == 1);
    CHECK(or_intern(&t, y, 2, 0, &b) == OR_OK && b == 2);
    CHECK(or_intern(&t, x, 2, 1, &b) == OR_OK && b == a);
    before = t;
    b = 91;
    CHECK(or_intern(&t, (const unsigned char *)"new", 3, 1, &b) == OR_FULL &&
          b == 91 && !memcmp(&t, &before, sizeof t));
    CHECK(or_intern(&t, NULL, 0, 0, &b) == OR_OK);
    CHECK(or_intern(&t, NULL, 1, 0, &b) == OR_INVALID);
    return 0;
}

static int shapes(void)
{
    struct or_shapes s;
    struct or_object a = {0}, b = {0}, c = {0}, before;
    struct or_cache cache = {0};
    int32_t out;
    or_shapes_init(&s);
    CHECK(or_property_set(&s, &a, 1, 10) == OR_OK);
    CHECK(or_property_set(&s, &a, 2, 20) == OR_OK);
    CHECK(or_property_set(&s, &b, 1, 30) == OR_OK);
    CHECK(or_property_set(&s, &b, 2, 40) == OR_OK && a.shape == b.shape);
    CHECK(or_property_set(&s, &c, 2, 50) == OR_OK);
    CHECK(or_property_set(&s, &c, 1, 60) == OR_OK && c.shape != a.shape);
    CHECK(or_property_get(&s, &c, 1, &cache, &out) == OR_OK && out == 60);
    CHECK(or_property_set(&s, &a, 3, 1) == OR_OK);
    CHECK(or_property_set(&s, &a, 4, 1) == OR_OK);
    before = a;
    CHECK(or_property_set(&s, &a, 5, 1) == OR_FULL &&
          !memcmp(&a, &before, sizeof a));
    return 0;
}

static int cache(void)
{
    struct or_shapes s;
    struct or_object a = {0}, b = {0}, c = {0};
    struct or_cache cache = {0};
    int32_t out = 91;
    or_shapes_init(&s);
    CHECK(or_property_set(&s, &a, 1, 10) == OR_OK);
    CHECK(or_property_set(&s, &a, 2, 20) == OR_OK);
    CHECK(or_property_set(&s, &b, 2, 30) == OR_OK);
    CHECK(or_property_set(&s, &b, 1, 40) == OR_OK);
    CHECK(or_property_get(&s, &a, 1, &cache, &out) == OR_OK && out == 10);
    CHECK(or_property_get(&s, &b, 1, &cache, &out) == OR_OK && out == 40);
    CHECK(or_property_get(&s, &a, 1, &cache, &out) == OR_OK && cache.hits == 1);
    CHECK(or_property_get(&s, &a, 2, &cache, &out) == OR_OK && out == 20 &&
          cache.mega == 1);
    CHECK(or_property_get(&s, &c, 2, &cache, &out) == OR_MISS && out == 20);
    return 0;
}

static int mark(void)
{
    struct or_graph g = {0};
    unsigned n = 91;
    g.live = 15;
    g.roots = 3;
    CHECK(or_mark_begin(&g) == OR_OK);
    CHECK(or_mark_step(&g, 1, &n) == OR_OK && n == 1 && g.color[0] == 2);
    CHECK(or_graph_store(&g, 1, 0, 3) == OR_OK && g.color[2] == 1);
    CHECK(or_graph_root(&g, 4, 1) == OR_OK && g.color[3] == 1);
    CHECK(or_mark_step(&g, 0, &n) == OR_OK && n == 0);
    CHECK(or_mark_step(&g, 16, &n) == OR_DONE && n == 3);
    CHECK(g.color[0] == 2 && g.color[1] == 2 && g.color[2] == 2 &&
          g.color[3] == 2);
    return 0;
}

static int nursery(void)
{
    struct or_nursery h = {0}, before;
    unsigned n = 91;
    h.used = 3;
    h.old_used = 1;
    h.old[0].edge[0] = 2;
    h.roots[0] = 3;
    h.young[0][0].value = 99;
    h.young[0][1].value = 20;
    h.young[0][1].edge[0] = 2;
    h.young[0][2].value = 30;
    h.young[0][2].edge[0] = 2;
    CHECK(or_minor(&h, 8, 8, &n) == OR_OK && n == 2 && h.active == 1 &&
          h.used == 2 && h.roots[0] == 1 && h.old[0].edge[0] == 2);
    CHECK(h.young[1][0].value == 30 && h.young[1][1].value == 20 &&
          h.young[1][1].edge[0] == 2);
    before = h;
    n = 91;
    CHECK(or_minor(&h, 8, 1, &n) == OR_FULL && n == 91 &&
          !memcmp(&h, &before, sizeof h));
    CHECK(or_minor(&h, 8, 8, &n) == OR_OK && h.used == 0 && h.old_used == 3 &&
          h.roots[0] == 10 && h.old[0].edge[0] == 11);
    CHECK(h.old[2].edge[0] == 11);
    return 0;
}

static int cards(void)
{
    struct or_cards c = {0};
    uint16_t roots = 0;
    unsigned n = 0;
    CHECK(or_card_store(&c, 3, 1, 5) == OR_OK && c.dirty == 2);
    CHECK(or_card_store(&c, 2, 0, 7) == OR_OK);
    CHECK(or_card_scan(&c, &roots, &n) == OR_OK && roots == 80 && n == 4 &&
          c.dirty == 2);
    CHECK(or_card_store(&c, 3, 1, 9) == OR_OK);
    CHECK(or_card_scan(&c, &roots, &n) == OR_OK && roots == 64 && c.dirty == 2);
    CHECK(or_card_store(&c, 2, 0, 0) == OR_OK);
    CHECK(or_card_scan(&c, &roots, &n) == OR_OK && roots == 0 && n == 4 &&
          c.dirty == 0);
    CHECK(or_card_scan(&c, &roots, &n) == OR_OK && n == 0);
    return 0;
}

static int compact(void)
{
    struct or_compact h = {0}, before;
    uint8_t f[16] = {0};
    h.live = 10;
    h.cell[1].value = 22;
    h.cell[3].value = 44;
    h.cell[1].edge[0] = 3;
    h.handle[0] = 2;
    h.handle[1] = 2;
    h.handle[2] = 4;
    h.borrows = 1;
    before = h;
    CHECK(or_compact(&h, f) == OR_STATE && !memcmp(&h, &before, sizeof h));
    h.borrows = 0;
    CHECK(or_compact(&h, f) == OR_OK && h.live == 3 && h.handle[0] == 1 &&
          h.handle[1] == 1 && h.handle[2] == 2 && f[1] == 1 && f[3] == 2);
    CHECK(h.cell[0].value == 22 && h.cell[1].value == 44 &&
          h.cell[0].edge[0] == 3);
    return 0;
}

static int weak(void)
{
    struct or_weak h = {0};
    uint16_t dead = 99;
    unsigned id = 99;
    h.graph.live = 31;
    h.graph.roots = 1;
    h.key[0] = 2;
    h.value[0] = 3;
    h.key[1] = 1;
    h.value[1] = 2;
    h.key[2] = 4;
    h.value[2] = 5;
    h.graph.edge[4][0] = 4;
    h.weak[0] = 4;
    CHECK(or_weak_collect(&h, &dead) == OR_OK && dead == 24 &&
          h.graph.live == 7 && h.weak[0] == 0);
    h = (struct or_weak){0};
    h.graph.live = 3;
    h.graph.edge[0][0] = 2;
    h.finalizable = 1;
    h.weak[0] = 1;
    CHECK(or_weak_collect(&h, &dead) == OR_OK && dead == 0 && h.pending == 1 &&
          h.weak[0] == 0);
    CHECK(or_weak_collect(&h, &dead) == OR_STATE);
    CHECK(or_final_next(&h, &id) == OR_OK && id == 1 && h.finalized == 1);
    h.graph.roots = 1;
    CHECK(or_weak_collect(&h, &dead) == OR_OK && dead == 0);
    h.graph.roots = 0;
    CHECK(or_weak_collect(&h, &dead) == OR_OK && dead == 3 && h.pending == 0);
    return 0;
}

static int trial(void)
{
    struct or_counts h = {0};
    uint16_t dead = 91;
    h.live = 7;
    h.edge[0][0] = 2;
    h.edge[1][0] = 1;
    h.refs[0] = 1;
    h.refs[1] = 1;
    h.refs[2] = 1;
    CHECK(or_trial_delete(&h, 3, &dead) == OR_OK && dead == 3 && h.live == 4 &&
          h.refs[2] == 1);
    h = (struct or_counts){0};
    h.live = 3;
    h.edge[0][0] = 2;
    h.edge[1][0] = 1;
    h.refs[0] = 2;
    h.refs[1] = 1;
    CHECK(or_trial_delete(&h, 3, &dead) == OR_OK && dead == 0 && h.live == 3 &&
          h.refs[0] == 2 && h.refs[1] == 1);
    h.refs[1] = 0;
    CHECK(or_trial_delete(&h, 3, &dead) == OR_INVALID);
    return 0;
}

static int safepoint(void)
{
    struct or_frame f = {0}, before;
    uint8_t forward[16] = {0}, length[16] = {0};
    forward[1] = 5;
    length[1] = 8;
    f.initialized = 7;
    f.root_map = 3;
    f.at_safepoint = 1;
    f.slot[0] = (struct or_slot){1, 2, 0, 0};
    f.slot[1] = (struct or_slot){1, 2, 8, 0};
    f.slot[2] = (struct or_slot){0, 0, 0, 2};
    before = f;
    f.root_map = 1;
    CHECK(or_safepoint(&f, forward, length) == OR_INVALID);
    f = before;
    CHECK(or_safepoint(&f, forward, length) == OR_OK && f.slot[0].base == 5 &&
          f.slot[1].base == 5 && f.slot[1].offset == 8 &&
          f.slot[2].scalar == 2);
    f = before;
    f.slot[1].offset = 9;
    before = f;
    CHECK(or_safepoint(&f, forward, length) == OR_INVALID &&
          !memcmp(&f, &before, sizeof f));
    return 0;
}

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        return 2;
    }
#define CASE(name)               \
    if (!strcmp(argv[1], #name)) \
    return name()
    CASE(value);
    CASE(intern);
    CASE(shapes);
    CASE(cache);
    CASE(mark);
    CASE(nursery);
    CASE(cards);
    CASE(compact);
    CASE(weak);
    CASE(trial);
    CASE(safepoint);
    return 2;
}
