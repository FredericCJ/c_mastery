#include "dod_text.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
static const char *test_name = "arguments";
static unsigned oracle_output;
#define CHECK(x)                                                              \
    do                                                                        \
    {                                                                         \
        if (!(x))                                                             \
        {                                                                     \
            fprintf(stderr, "DOD_CHECK:%s:%d:%s\n", test_name, __LINE__, #x); \
            exit(1);                                                          \
        }                                                                     \
    } while (0)

static unsigned depth(const struct d_rope *r, unsigned root)
{
    if (!root)
        return 0;
    const struct d_rope_node *n = &r->nodes[root];
    if (!n->left)
        return 1;
    unsigned a = depth(r, n->left), b = depth(r, n->right);
    return 1 + (a > b ? a : b);
}

static void rope_edit(void)
{
    for (size_t at = 0; at <= 8; ++at)
    {
        struct d_rope r;
        d_rope_init(&r);
        unsigned a, b, root, left = 99, right = 98;
        CHECK(d_rope_leaf(&r, (const unsigned char *)"abcd", 4, &a) == D_OK);
        CHECK(d_rope_concat(&r, a, a, &root) == D_OK);
        CHECK(d_rope_split(&r, root, at, &left, &right) == D_OK);
        unsigned char bytes[128];
        size_t n = 0;
        CHECK(d_rope_flatten(&r, left, bytes, 128, &n) == D_OK && n == at &&
              !memcmp(bytes, "abcdabcd", at));
        CHECK(d_rope_flatten(&r, right, bytes, 128, &n) == D_OK &&
              n == 8 - at && !memcmp(bytes, &"abcdabcd"[at], n));
        CHECK(d_rope_concat(&r, left, right, &b) == D_OK);
        CHECK(d_rope_flatten(&r, b, bytes, 128, &n) == D_OK && n == 8 &&
              !memcmp(bytes, "abcdabcd", 8));
        for (size_t i = 0; i < 8; ++i)
        {
            unsigned char x = 0;
            CHECK(d_rope_index(&r, root, i, &x) == D_OK &&
                  x == (unsigned char)"abcdabcd"[i]);
        }
    }
    for (unsigned spare = 0; spare < 2; ++spare)
    {
        struct d_rope r;
        d_rope_init(&r);
        unsigned root, junk;
        CHECK(d_rope_leaf(&r, (const unsigned char *)"abcd", 4, &root) == D_OK);
        while (r.used < 64 - spare)
            CHECK(d_rope_leaf(&r, (const unsigned char *)"x", 1, &junk) ==
                  D_OK);
        unsigned before = r.used, left = 99, right = 98;
        CHECK(d_rope_split(&r, root, 2, &left, &right) == D_FULL &&
              r.used == before && left == 99 && right == 98);
    }
    struct d_rope r;
    d_rope_init(&r);
    unsigned root = 0;
    for (unsigned i = 0; i < 16; ++i)
    {
        unsigned leaf;
        unsigned char byte = (unsigned char)i;
        CHECK(d_rope_leaf(&r, &byte, 1, &leaf) == D_OK);
        CHECK(d_rope_concat(&r, root, leaf, &root) == D_OK);
    }
    unsigned balanced;
    CHECK(depth(&r, root) == 16);
    CHECK(d_rope_balance(&r, root, &balanced) == D_OK &&
          depth(&r, balanced) == 2);
    for (unsigned i = 0; i < 16; ++i)
    {
        unsigned char a, b;
        CHECK(d_rope_index(&r, root, i, &a) == D_OK &&
              d_rope_index(&r, balanced, i, &b) == D_OK && a == b);
    }
}

static void editors(void)
{
    const unsigned char data[] = {'a', 0, 'b'};
    for (unsigned choice = 0; choice < 243; ++choice)
    {
        unsigned script = choice;
        struct d_rope rope;
        d_rope_init(&rope);
        unsigned roots[6] = {0};
        unsigned char snapshots[6][128] = {{0}};
        size_t sizes[6] = {0};
        struct d_gap gap;
        d_gap_init(&gap);
        struct d_piece_table table;
        unsigned char backing[128], flat[128], g[128], p[128];
        size_t length = 0;
        CHECK(d_piece_init(&table, NULL, 0, backing, 128) == D_OK);
        for (unsigned step = 0; step < 5; ++step)
        {
            unsigned op = script % 3;
            script /= 3;
            size_t at = length ? step % (length + 1) : 0;
            if (op < 2)
            {
                size_t n = op + 1;
                CHECK(d_rope_splice(&rope, roots[step], at, 0, data, n,
                                    &roots[step + 1]) == D_OK);
                CHECK(d_gap_insert(&gap, at, data, n) == D_OK &&
                      d_piece_insert(&table, at, data, n) == D_OK);
                for (size_t i = length; i > at; --i)
                    flat[i + n - 1] = flat[i - 1];
                for (size_t i = 0; i < n; ++i)
                    flat[at + i] = data[i];
                length += n;
            }
            else
            {
                size_t n = at < length ? 1 : 0;
                CHECK(d_rope_splice(&rope, roots[step], at, n, NULL, 0,
                                    &roots[step + 1]) == D_OK);
                CHECK(d_gap_delete(&gap, at, n) == D_OK &&
                      d_piece_delete(&table, at, n) == D_OK);
                for (size_t i = at + n; i < length; ++i)
                    flat[i - n] = flat[i];
                length -= n;
            }
            sizes[step + 1] = length;
            memcpy(snapshots[step + 1], flat, length);
            for (unsigned version = 0; version <= step + 1; ++version)
            {
                unsigned char observed[128];
                size_t rn = 99;
                CHECK(d_rope_flatten(&rope, roots[version], observed, 128,
                                     &rn) == D_OK &&
                      rn == sizes[version] &&
                      !memcmp(observed, snapshots[version], rn));
            }
            size_t gn = 99, pn = 99;
            CHECK(d_gap_flatten(&gap, g, 128, &gn) == D_OK &&
                  d_piece_flatten(&table, p, 128, &pn) == D_OK);
            CHECK(gn == length && pn == length && !memcmp(flat, g, length) &&
                  !memcmp(flat, p, length));
            if (oracle_output)
            {
                printf(
                    "{\"kind\":\"edit\",\"script\":%u,\"step\":%u,\"bytes\":[",
                    choice, step);
                for (size_t i = 0; i < pn; ++i)
                    printf("%s%u", i ? "," : "", (unsigned)p[i]);
                puts("]}");
            }
            CHECK(d_gap_delete(&gap, length + 1, 0) == D_RANGE &&
                  d_piece_delete(&table, length + 1, 0) == D_RANGE);
        }
        unsigned char compact[128], next_add[128];
        struct d_piece_table next;
        CHECK(d_piece_compact(&table, compact, length, next_add, 128, &next) ==
              D_OK);
        CHECK(d_piece_insert(&next, 0, (const unsigned char *)"z", 1) == D_OK);
        size_t n = 99;
        CHECK(d_piece_flatten(&table, p, 128, &n) == D_OK && n == length &&
              !memcmp(p, flat, n));
        CHECK(d_piece_flatten(&next, p, 128, &n) == D_OK && n == length + 1 &&
              p[0] == 'z');
    }
    struct d_gap gap;
    d_gap_init(&gap);
    unsigned char full[128] = {0};
    CHECK(d_gap_insert(&gap, 0, full, 128) == D_OK);
    CHECK(d_gap_insert(&gap, 128, data, 1) == D_FULL);
    size_t n = 99;
    unsigned char out = 91;
    CHECK(d_gap_flatten(&gap, &out, 1, &n) == D_FULL && n == 99 && out == 91);
    unsigned char add[1];
    struct d_piece_table p;
    CHECK(d_piece_init(&p, full, 128, add, 1) == D_OK);
    CHECK(d_piece_insert(&p, 0, data, 1) == D_FULL && p.length == 128 &&
          p.add_length == 0);
}

static void splice(void)
{
    unsigned failed = 0, passed = 0;
    for (unsigned spare = 0; spare <= 16; ++spare)
    {
        struct d_rope r;
        d_rope_init(&r);
        unsigned leaf, root, junk, out = 999;
        CHECK(d_rope_leaf(&r, (const unsigned char *)"abcd", 4, &leaf) ==
                  D_OK &&
              d_rope_concat(&r, leaf, leaf, &root) == D_OK);
        while (r.used < 64 - spare)
            CHECK(d_rope_leaf(&r, (const unsigned char *)"x", 1, &junk) ==
                  D_OK);
        unsigned before = r.used;
        int result = d_rope_splice(&r, root, 2, 2,
                                   (const unsigned char *)"12345678", 8, &out);
        unsigned char bytes[128];
        size_t n = 99;
        if (result == D_FULL)
        {
            ++failed;
            CHECK(out == 999 && r.used == before);
        }
        else
        {
            ++passed;
            CHECK(result == D_OK &&
                  d_rope_flatten(&r, out, bytes, 128, &n) == D_OK && n == 14 &&
                  !memcmp(bytes, "ab12345678abcd", 14));
        }
        CHECK(d_rope_flatten(&r, root, bytes, 128, &n) == D_OK && n == 8 &&
              !memcmp(bytes, "abcdabcd", 8));
    }
    CHECK(failed && passed);
    struct d_rope r;
    d_rope_init(&r);
    unsigned root = 0;
    unsigned char full[128];
    for (unsigned i = 0; i < 128; ++i)
        full[i] = (unsigned char)i;
    CHECK(d_rope_splice(&r, 0, 0, 0, full, 128, &root) == D_OK);
    unsigned before = r.used, out = 999;
    CHECK(d_rope_splice(&r, root, 128, 0, full, 1, &out) == D_FULL &&
          out == 999 && r.used == before);
    CHECK(d_rope_splice(&r, root, 129, 0, NULL, 0, &out) == D_RANGE &&
          out == 999);
    CHECK(d_rope_splice(&r, root, 0, 129, NULL, 0, &out) == D_RANGE &&
          out == 999);
    CHECK(d_rope_splice(&r, root, 128, 0, NULL, 0, &out) == D_OK &&
          out == root && r.used == before);
    CHECK(d_rope_splice(&r, root, 0, 128, NULL, 0, &out) == D_OK && out == 0);
}

static void utf8(void)
{
    const unsigned char valid[] = {0x61, 0xc2, 0xa2, 0xe2, 0x82,
                                   0xac, 0xf0, 0x9f, 0x98, 0x80};
    const size_t positions[] = {0, 1, 3, 6, 10};
    for (size_t i = 0; i < 5; ++i)
    {
        size_t out = 99;
        CHECK(d_utf8_offset(valid, sizeof(valid), i, &out) == D_OK &&
              out == positions[i]);
    }
    const unsigned char invalid[][4] = {{0xc0, 0x80, 0, 0},
                                        {0xed, 0xa0, 0x80, 0},
                                        {0xf4, 0x90, 0x80, 0x80},
                                        {0xf0, 0x80, 0x80, 0x80},
                                        {0xe2, 0x82, 0, 0}};
    const size_t sizes[] = {2, 3, 4, 4, 2};
    for (unsigned i = 0; i < 5; ++i)
    {
        size_t out = 99;
        CHECK(d_utf8_offset(invalid[i], sizes[i], 0, &out) == D_INVALID &&
              out == 99);
    }
    size_t out = 99;
    CHECK(d_utf8_offset(valid, sizeof(valid), 5, &out) == D_RANGE && out == 99);
    struct d_gap gap;
    d_gap_init(&gap);
    CHECK(d_gap_insert(&gap, 0, valid, sizeof(valid)) == D_OK);
    unsigned char add[128], flat[128];
    struct d_piece_table table;
    CHECK(d_piece_init(&table, valid, sizeof(valid), add, 128) == D_OK);
    const unsigned char insert[] = {0xc3, 0xa9};
    CHECK(d_gap_scalar_edit(&gap, 1, 2, insert, 2) == D_OK &&
          d_piece_scalar_edit(&table, 1, 2, insert, 2) == D_OK);
    const unsigned char expect[] = {0x61, 0xc3, 0xa9, 0xf0, 0x9f, 0x98, 0x80};
    size_t size = 0;
    CHECK(d_gap_flatten(&gap, flat, 128, &size) == D_OK &&
          size == sizeof(expect) && !memcmp(flat, expect, size));
    CHECK(d_piece_flatten(&table, flat, 128, &size) == D_OK &&
          size == sizeof(expect) && !memcmp(flat, expect, size));
    CHECK(d_gap_scalar_edit(&gap, 0, 1, invalid[1], 3) == D_INVALID &&
          d_piece_scalar_edit(&table, 0, 1, invalid[1], 3) == D_INVALID);
    CHECK(d_gap_flatten(&gap, flat, 128, &size) == D_OK &&
          size == sizeof(expect) && !memcmp(flat, expect, size));
    CHECK(d_piece_flatten(&table, flat, 128, &size) == D_OK &&
          size == sizeof(expect) && !memcmp(flat, expect, size));
    /* Scalar1 begins at byte1 but scalar2 begins at byte3; a byte-index edit at2
       cuts the two-byte scalar. This is independent of grapheme segmentation. */
}

int main(int argc, char **argv)
{
    CHECK(argc == 2);
    test_name = argv[1];
    if (!strcmp(argv[1], "rope_edit"))
        rope_edit();
    else if (!strcmp(argv[1], "editors"))
        editors();
    else if (!strcmp(argv[1], "utf8"))
        utf8();
    else if (!strcmp(argv[1], "rope_splice"))
        splice();
    else if (!strcmp(argv[1], "oracle"))
    {
        oracle_output = 1;
        editors();
    }
    else
        CHECK(0);
    return 0;
}
