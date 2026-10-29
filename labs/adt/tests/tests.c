#include "adt.h"
#include <limits.h>
#include <stdio.h>
#include <string.h>
#define CHECK(x)                                                 \
    do                                                           \
    {                                                            \
        if (!(x))                                                \
        {                                                        \
            fprintf(stderr, "ADT_CHECK %d: %s\n", __LINE__, #x); \
            return 1;                                            \
        }                                                        \
    } while (0)

static int sums(void)
{
    struct ad_value v = {AD_PAIR, {.pair = {UINT32_MAX, 5}}};
    uint64_t sum = 91;
    uint32_t n = 92;
    CHECK(ad_measure(v, &sum) && sum == (uint64_t)UINT32_MAX + 5);
    CHECK(!ad_number(v, &n) && n == 92);
    v.tag = (enum ad_tag)99;
    CHECK(!ad_measure(v, &sum) && sum == (uint64_t)UINT32_MAX + 5);
    v.tag = AD_EMPTY;
    CHECK(ad_measure(v, &sum) && sum == 0);
    return 0;
}

static int quantities(void)
{
    struct ad_millis out = {91};
    struct ad_seconds in = {42};
    CHECK(ad_milliseconds(in, &out) && out.value == 42000);
    in.value = UINT32_MAX / 1000u + 1;
    CHECK(!ad_milliseconds(in, &out) && out.value == 42000);
    in.value = 0;
    CHECK(ad_milliseconds(in, &out) && out.value == 0);
    return 0;
}

static int flags(void)
{
    struct ad_flags out = {91};
    CHECK(!ad_flags(8, &out) && out.bits == 91);
    CHECK(ad_flags(0, &out) && out.bits == 0);
    CHECK(ad_flags(7, &out) && out.bits == 7);
    return 0;
}

static int text(void)
{
    const unsigned char good[] = {0x41, 0xe2, 0x82, 0xac,
                                  0xf4, 0x8f, 0xbf, 0xbf};
    const unsigned char over[] = {0xc0, 0x80}, surrogate[] = {0xed, 0xa0, 0x80},
                        high[] = {0xf4, 0x90, 0x80, 0x80};
    struct ad_text out = {NULL, 91, 92};
    CHECK(ad_text(good, sizeof good, &out) && out.scalars == 3 &&
          out.bytes == good);
    CHECK(!ad_text(over, sizeof over, &out) && out.scalars == 3);
    CHECK(!ad_text(surrogate, sizeof surrogate, &out));
    CHECK(!ad_text(high, sizeof high, &out));
    CHECK(!ad_text(good, 3, &out));
    CHECK(ad_text(NULL, 0, &out) && out.scalars == 0);
    return 0;
}

static int brands(void)
{
    struct ad_pool a, b;
    struct ad_brand first, other, next;
    uint32_t out = 91;
    unsigned i;
    ad_pool_init(&a);
    ad_pool_init(&b);
    CHECK(ad_pool_acquire(&a, 5, &first) && ad_pool_acquire(&b, 7, &other));
    CHECK(!ad_pool_get(&b, first, &out) && out == 91);
    CHECK(ad_pool_release(&a, first) && ad_pool_acquire(&a, 9, &next));
    CHECK(!ad_pool_get(&a, first, &out));
    CHECK(ad_pool_get(&a, next, &out) && out == 9);
    CHECK(!ad_pool_release(&a, first));
    CHECK(ad_pool_release(&a, next));
    a.slot[0].generation = UINT_MAX;
    CHECK(ad_pool_acquire(&a, 1, &next));
    CHECK(ad_pool_release(&a, next));
    CHECK(a.slot[0].retired);
    for (i = 0; i < 3; ++i)
        CHECK(ad_pool_acquire(&a, i, &next));
    CHECK(!ad_pool_acquire(&a, 8, &next));
    return 0;
}

static int views(void)
{
    uint32_t a[] = {1, 2, 99, 3, 4};
    uint32_t out = 91;
    struct ad_view view;
    CHECK(ad_view(a, 5, 2, 2, 3, 1, &view));
    CHECK(ad_view_get(view, 1, 1, &out) && out == 4);
    CHECK(!ad_view(a, 4, 2, 2, 3, 1, &view));
    CHECK(!ad_view(a, 5, SIZE_MAX, 2, 2, 1, &view));
    CHECK(ad_view(NULL, 0, 0, 3, 0, 0, &view));
    CHECK(!ad_view_get(view, 0, 0, &out));
    CHECK(ad_view(a, 5, 2, 2, 1, 1, &view));
    CHECK(ad_view_get(view, 1, 0, &out) && out == 2);
    return 0;
}

static int arena(void)
{
    struct ad_node nodes[] = {
        {0, 7, {0}, {0}}, {0, 9, {0}, {0}}, {1, 0, {0}, {1}}};
    uint32_t out = 91;
    struct ad_node_index root = {2};
    CHECK(ad_eval(nodes, 3, root, &out) && out == 16);
    nodes[2].left.index = 2;
    CHECK(!ad_eval(nodes, 3, root, &out) && out == 16);
    nodes[2].left.index = 0;
    nodes[0].value = UINT32_MAX;
    CHECK(!ad_eval(nodes, 3, root, &out) && out == 16);
    return 0;
}

static int optics(void)
{
    uint32_t x, y, a, b, v;
    struct ad_value empty = {AD_EMPTY, {.number = 0}};
    for (x = 0; x < 4; ++x)
        for (y = 0; y < 4; ++y)
            for (a = 0; a < 4; ++a)
                for (b = 0; b < 4; ++b)
                {
                    struct ad_pair s = {x, y}, t = ad_lens_x(s, a),
                                   identity = ad_lens_x(s, s.x);
                    struct ad_pair two = ad_lens_x(t, b), one = ad_lens_x(s, b);
                    CHECK(t.x == a && t.y == y && identity.x == x &&
                          identity.y == y);
                    CHECK(two.x == one.x && two.y == one.y);
                    CHECK(ad_preview_number(ad_review_number(a), &v) && v == a);
                }
    v = 91;
    CHECK(!ad_preview_number(empty, &v) && v == 91);
    return 0;
}

static int evolution(void)
{
    unsigned char future[] = {9, 3, 4, 5, 6}, malformed[] = {1, 1, 7},
                  critical[] = {128, 0}, copy[8];
    size_t used = 91;
    struct ad_evolving value;
    CHECK(ad_evolve_read(future, sizeof future, &value));
    CHECK(value.tag == 9 && value.length == 3 && value.payload == future + 2);
    CHECK(ad_evolve_write(value, copy, sizeof copy, &used) &&
          used == sizeof future && !memcmp(copy, future, used));
    CHECK(!ad_evolve_read(malformed, sizeof malformed, &value));
    CHECK(!ad_evolve_read(critical, sizeof critical, &value));
    CHECK(!ad_evolve_write(value, copy, 1, &used) && used == sizeof future);
    return 0;
}

static int packet(void)
{
    unsigned char data[] = {1, 0, 2, 7, 8}, ack[] = {2, 0, 0},
                  trailing[] = {1, 0, 1, 7, 8}, badack[] = {2, 0, 1, 7},
                  unknown[] = {3, 0, 0};
    struct ad_packet out = {91, NULL, 92};
    CHECK(ad_packet(data, sizeof data, &out) == AD_PACKET_OK);
    CHECK(out.kind == 1 && out.length == 2 && out.payload == data + 3);
    CHECK(ad_packet(trailing, sizeof trailing, &out) == AD_PACKET_LENGTH &&
          out.payload == data + 3);
    CHECK(ad_packet(badack, sizeof badack, &out) == AD_PACKET_ACK);
    CHECK(ad_packet(unknown, sizeof unknown, &out) == AD_PACKET_KIND);
    CHECK(ad_packet(ack, sizeof ack, &out) == AD_PACKET_OK && out.length == 0);
    CHECK(ad_packet(NULL, 0, &out) == AD_PACKET_HEADER);
    return 0;
}

int main(int argc, char **argv)
{
    if (argc != 2)
        return 2;
    if (!strcmp(argv[1], "sums"))
        return sums();
    if (!strcmp(argv[1], "quantities"))
        return quantities();
    if (!strcmp(argv[1], "flags"))
        return flags();
    if (!strcmp(argv[1], "text"))
        return text();
    if (!strcmp(argv[1], "brands"))
        return brands();
    if (!strcmp(argv[1], "views"))
        return views();
    if (!strcmp(argv[1], "arena"))
        return arena();
    if (!strcmp(argv[1], "optics"))
        return optics();
    if (!strcmp(argv[1], "evolution"))
        return evolution();
    if (!strcmp(argv[1], "packet"))
        return packet();
    return 2;
}
