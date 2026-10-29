#include "metaprog.h"
#include <limits.h>
#include <stdio.h>
#include <string.h>
#define CHECK(x)                                                \
    do                                                          \
    {                                                           \
        if (!(x))                                               \
        {                                                       \
            fprintf(stderr, "MP_CHECK %d: %s\n", __LINE__, #x); \
            return 1;                                           \
        }                                                       \
    } while (0)

static int mono(void)
{
    struct mpu32_ring a = {{0}, 0, 0};
    struct mp_pair_ring b = {{{0, 0}}, 0, 0};
    uint32_t v = 91;
    struct mp_pair pair = {92, 93};
    unsigned i;
    for (i = 0; i < 4; ++i)
    {
        CHECK(mp_u32_push(&a, i));
    }
    CHECK(!mp_u32_push(&a, 9));
    CHECK(mp_u32_pop(&a, &v) && v == 0);
    CHECK(mp_u32_push(&a, 4));
    for (i = 1; i < 5; ++i)
    {
        CHECK(mp_u32_pop(&a, &v) && v == i);
    }
    CHECK(!mp_u32_pop(&a, &v) && v == 4);
    CHECK(mp_pair_push(&b, (struct mp_pair){3, 7}) &&
          mp_pair_push(&b, (struct mp_pair){9, 11}));
    CHECK(mp_pair_pop(&b, &pair) && pair.x == 3 && pair.y == 7);
    CHECK(mp_pair_pop(&b, &pair) && pair.x == 9 && pair.y == 11);
    return 0;
}

static int constraints(void)
{
    size_t out = 91;
    CHECK(mp_storage_bound(4, sizeof(uint32_t), 64, &out) &&
          out == 4 * sizeof(uint32_t));
    CHECK(!mp_storage_bound(SIZE_MAX, 2, SIZE_MAX, &out) &&
          out == 4 * sizeof(uint32_t));
    CHECK(!mp_storage_bound(4, 8, 31, &out));
    CHECK(!mp_storage_bound(0, 8, 31, &out));
    return 0;
}

static int policies(void)
{
    const uint32_t a[] = {7, 5};
    uint32_t out = 91;
    CHECK(!mp_reduce(a, 2, (struct mp_policy){10, 0}, &out) && out == 91);
    CHECK(mp_reduce(a, 2, (struct mp_policy){10, 1}, &out) && out == 10);
    CHECK(mp_reduce(NULL, 0, (struct mp_policy){10, 1}, &out) && out == 0);
    out = 91;
    CHECK(!mp_reduce_reject(a, 2, 10, &out) && out == 91);
    CHECK(mp_reduce_saturating(a, 2, 10, &out) && out == 10);
    CHECK(!mp_reduce(a, 2, (struct mp_policy){10, 2}, &out));
    return 0;
}

static int variadic(void)
{
    unsigned n, calls = 91;
    uint32_t out = 92;
    for (n = 0; n <= 4; ++n)
    {
        CHECK(mp_variadic(n, 7, &calls, &out) && calls == n &&
              out == n * 7 + n * (n - 1) / 2);
    }
    CHECK(!mp_variadic(5, 7, &calls, &out) && calls == 4 && out == 34);
    return 0;
}

static int generic(void)
{
    uint32_t mutable_value = 7, out = 91;
    const uint32_t fixed_value = 9;
    unsigned calls = 92;
    CHECK(mp_generic_receivers(&mutable_value, &fixed_value, &calls, &out));
    CHECK(out == 316 && calls == 2 && mutable_value == 7 && fixed_value == 9);
    return 0;
}

static int reflection(void)
{
    const struct mp_field *fields = NULL;
    unsigned char out[6] = {91, 91, 91, 91, 91, 91};
    CHECK(mp_fields(&fields) == 2 && fields != NULL);
    CHECK(!strcmp(fields[0].name, "code") &&
          fields[0].offset == offsetof(struct mp_record, code) &&
          fields[0].width == 2);
    CHECK(!strcmp(fields[1].name, "value") &&
          fields[1].offset == offsetof(struct mp_record, value) &&
          fields[1].width == 4);
    CHECK(!mp_record_encode((struct mp_record){0x1234, UINT32_C(0x89abcdef)},
                            out, 5) &&
          out[0] == 91);
    CHECK(mp_record_encode((struct mp_record){0x1234, UINT32_C(0x89abcdef)},
                           out, 6));
    CHECK(out[0] == 0x12 && out[1] == 0x34 && out[2] == 0x89 && out[5] == 0xef);
    return 0;
}

static int sum_below_five(struct mp_pair pair, void *context)
{
    (void)context;
    return pair.x + pair.y < 5;
}

static int reflexive(struct mp_pair pair, void *context)
{
    (void)context;
    return mp_equal((struct mp_key){pair.x, pair.y, 0},
                    (struct mp_key){pair.x, pair.y, 99});
}

static int laws(void)
{
    unsigned x, y;
    struct mp_pair witness = {91, 92};
    CHECK(!mp_equal((struct mp_key){1, 2, 0}, (struct mp_key){1, 3, 0}));
    for (x = 0; x < 4; ++x)
    {
        for (y = 0; y < 4; ++y)
        {
            struct mp_key a = {x, y, 0}, b = {x, y, 9};
            CHECK(mp_equal(a, b));
            CHECK(mp_hash(a) == mp_hash(b));
        }
    }
    CHECK(mp_find_counterexample(sum_below_five, NULL, 8, &witness) == 1 &&
          witness.x + witness.y == 5);
    CHECK(mp_find_counterexample(reflexive, NULL, 8, &witness) == 0);
    CHECK(mp_find_counterexample(reflexive, NULL, 256, &witness) == -1);
    return 0;
}

static int formatting(void)
{
    unsigned char buffer[12] = {91};
    struct mp_sink sink = {buffer, sizeof buffer, 0};
    uint32_t zero = 0;
    CHECK(mp_format(&sink, MP_ARG(zero)) && sink.used == 1 && buffer[0] == '0');
    CHECK(mp_format(&sink, MP_ARG("%n")) && sink.used == 3 &&
          buffer[1] == '%' && buffer[2] == 'n');
    CHECK(!mp_format(&sink, MP_ARG(UINT32_MAX)) && sink.used == 3 &&
          buffer[3] == 0);
    sink.used = 0;
    CHECK(mp_format(&sink, MP_ARG(UINT32_MAX)) && sink.used == 10 &&
          !memcmp(buffer, "4294967295", 10));
    return 0;
}

static int hygiene(void)
{
    struct mp_token pattern[] = {{2, 7, 0}, {1, 0, 0}, {2, 7, 0}},
                    arg[] = {{0, 7, 5}},
                    out[3] = {{0, 99, 99}, {0, 99, 99}, {0, 99, 99}};
    unsigned next = 12;
    size_t used = 91;
    CHECK(mp_expand(pattern, 3, arg, 1, &next, out, 3, &used) && next == 13 &&
          used == 3);
    CHECK(out[0].context == 12 && out[1].context == 5 && out[2].context == 12);
    next = UINT_MAX;
    CHECK(!mp_expand(pattern, 3, arg, 1, &next, out, 3, &used) &&
          next == UINT_MAX && out[1].context == 5);
    next = 14;
    CHECK(!mp_expand(pattern, 3, arg, 1, &next, out, 2, &used) && next == 14);
    pattern[1].symbol = 1;
    CHECK(!mp_expand(pattern, 3, arg, 1, &next, out, 3, &used) && next == 14);
    return 0;
}

static int buildgraph(void)
{
    const unsigned char a[] = {1, 2}, b[] = {3};
    struct mp_build_node nodes[] = {{0, a, 2, 10, 20}, {1, b, 1, 10, 20}};
    unsigned order[2] = {91, 92};
    uint64_t keys[2] = {91, 92}, old[2];
    char generated[256];
    size_t used = 91;
    CHECK(mp_build(nodes, 2, order, keys) && order[0] == 0 && order[1] == 1);
    old[0] = keys[0];
    old[1] = keys[1];
    nodes[0].configuration = 21;
    CHECK(mp_build(nodes, 2, order, keys) && keys[0] != old[0] &&
          keys[1] != old[1]);
    CHECK(mp_emit_table(keys, 2, generated, sizeof generated, &used) &&
          used > 50 && strstr(generated, "generated_keys") != NULL);
    old[0] = keys[0];
    old[1] = keys[1];
    nodes[0].dependencies = 2;
    CHECK(!mp_build(nodes, 2, order, keys) && keys[0] == old[0] &&
          keys[1] == old[1]);
    return 0;
}

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        return 2;
    }
    if (!strcmp(argv[1], "mono"))
    {
        return mono();
    }
    if (!strcmp(argv[1], "constraints"))
    {
        return constraints();
    }
    if (!strcmp(argv[1], "policies"))
    {
        return policies();
    }
    if (!strcmp(argv[1], "variadic"))
    {
        return variadic();
    }
    if (!strcmp(argv[1], "generic"))
    {
        return generic();
    }
    if (!strcmp(argv[1], "reflection"))
    {
        return reflection();
    }
    if (!strcmp(argv[1], "laws"))
    {
        return laws();
    }
    if (!strcmp(argv[1], "formatting"))
    {
        return formatting();
    }
    if (!strcmp(argv[1], "hygiene"))
    {
        return hygiene();
    }
    if (!strcmp(argv[1], "buildgraph"))
    {
        return buildgraph();
    }
    return 2;
}
