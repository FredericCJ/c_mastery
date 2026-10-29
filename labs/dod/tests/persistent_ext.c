#include "dod_persistent.h"
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

static void zipper(void)
{
    for (unsigned index = 0; index < 8; ++index)
    {
        for (unsigned fail = 0; fail <= 4; ++fail)
        {
            struct d_tree t;
            d_tree_init(&t);
            unsigned old = 0, out = 99;
            CHECK(d_tree_update(&t, 0, 3, 17, &old) == D_OK);
            struct d_zipper z;
            CHECK(d_zip_open(&t, old, &z) == D_OK);
            for (unsigned depth = 0; depth < 3; ++depth)
            {
                CHECK(d_zip_down(&z, (index >> (2 - depth)) & 1) == D_OK);
            }
            CHECK(d_zip_down(&z, 0) == D_INVALID);
            t.fail_after = fail;
            unsigned before = t.used;
            int status = d_zip_write(&z, 29, &out);
            CHECK(status == (fail ? D_FAILED : D_OK));
            if (fail)
            {
                CHECK(out == 99 && t.used == before);
            }
            for (unsigned i = 0; i < 8; ++i)
            {
                int value = 91;
                CHECK(d_tree_get(&t, old, i, &value) == D_OK &&
                      value == (i == 3 ? 17 : 0));
                if (!fail)
                {
                    CHECK(d_tree_get(&t, out, i, &value) == D_OK &&
                          value == (i == index ? 29
                                    : i == 3   ? 17
                                               : 0));
                }
            }
            for (unsigned depth = 0; depth < 3; ++depth)
            {
                CHECK(d_zip_up(&z) == D_OK);
            }
            CHECK(z.focus == old && d_zip_up(&z) == D_INVALID);
            if (!fail)
            {
                unsigned roots[] = {old, out}, nodes = 0, shared = 0;
                CHECK(d_tree_census(&t, roots, 2, &nodes, &shared) == D_OK);
                CHECK(nodes == 8);
                CHECK(shared == (index == 3 ? 0 : 1));
            }
        }
    }
}

static void deletion(void)
{
    for (unsigned a = 0; a < 32; ++a)
    {
        for (unsigned b = 0; b < 32; ++b)
        {
            struct d_hamt h;
            d_hamt_init(&h);
            unsigned first = 0, second = 0, third = 99;
            CHECK(d_hamt_update(&h, 0, a, 10, &first) == D_OK);
            CHECK(d_hamt_update(&h, first, b, 20, &second) == D_OK);
            CHECK(d_hamt_delete(&h, second, a, &third) == D_OK);
            if (oracle_output)
            {
                printf("{\"kind\":\"delete\",\"a\":%u,\"b\":%u,\"entries\":[",
                       a, b);
            }
            unsigned printed = 0;
            for (unsigned key = 0; key < 32; ++key)
            {
                int value = 91;
                int status = d_hamt_get(&h, third, key, &value);
                if (oracle_output && status == D_OK)
                {
                    printf("%s[%u,%d]", printed ? "," : "", key, value);
                    ++printed;
                }
                if (key == b && a != b)
                {
                    CHECK(status == D_OK && value == 20);
                }
                else
                {
                    CHECK(status == D_STALE && value == 91);
                }
                CHECK(d_hamt_get(&h, second, key, &value) ==
                      ((key == a || key == b) ? D_OK : D_STALE));
                if (key == a || key == b)
                {
                    CHECK(value == (key == b ? 20 : 10));
                }
            }
            if (oracle_output)
            {
                puts("]}");
            }
            unsigned absent = 99;
            CHECK(d_hamt_delete(&h, third, a, &absent) == D_STALE &&
                  absent == 99);
            if (a != b)
            {
                unsigned empty = 99;
                CHECK(d_hamt_delete(&h, third, b, &empty) == D_OK &&
                      empty == 0);
            }
        }
    }
    for (unsigned fail = 1; fail <= 3; ++fail)
    {
        struct d_hamt h;
        d_hamt_init(&h);
        unsigned a, b, out = 99;
        CHECK(d_hamt_update(&h, 0, 1, 10, &a) == D_OK);
        CHECK(d_hamt_update(&h, a, 17, 20, &b) == D_OK);
        h.fail_after = fail;
        unsigned before = h.used;
        CHECK(d_hamt_delete(&h, b, 1, &out) == D_FAILED && out == 99 &&
              h.used == before);
        int value = 0;
        CHECK(d_hamt_get(&h, b, 1, &value) == D_OK && value == 10);
        CHECK(d_hamt_get(&h, b, 17, &value) == D_OK && value == 20);
    }
}

struct d_strings
{
    unsigned clones, drops, live, fail;
};

static int clone(void *context, const unsigned char *p, size_t n,
                 unsigned char **out)
{
    struct d_strings *s = context;
    ++s->clones;
    if (s->fail == s->clones)
    {
        return 0;
    }
    unsigned char *copy = malloc(n + 1);
    CHECK(copy);
    for (size_t i = 0; i < n; ++i)
    {
        copy[i] = p[i];
    }
    copy[n] = 0;
    *out = copy;
    ++s->live;
    return 1;
}

static void drop(void *context, unsigned char *p)
{
    struct d_strings *s = context;
    CHECK(s->live);
    --s->live;
    ++s->drops;
    free(p);
}

static void refcount(void)
{
    for (unsigned fail = 0; fail <= 5; ++fail)
    {
        struct d_strings s = {0};
        struct d_rc_tree t;
        d_rc_init(&t, (struct d_text_ops){&s, clone, drop});
        unsigned a = 0, b = 0, c = 99;
        CHECK(d_rc_update(&t, 0, 0, (const unsigned char *)"left", 4, &a) ==
              D_OK);
        CHECK(d_rc_update(&t, a, 7, (const unsigned char *)"right", 5, &b) ==
              D_OK);
        CHECK(t.live == 8 && s.live == 2);
        t.fail_at = fail <= 4 ? fail : 0;
        if (fail == 5)
        {
            s.fail = s.clones + 1;
        }
        int result = d_rc_update(&t, b, 7, (const unsigned char *)"new", 3, &c);
        CHECK(result == (fail ? D_FAILED : D_OK));
        if (fail)
        {
            CHECK(c == 99 && t.live == 8 && s.live == 2);
        }
        const unsigned char *text = NULL;
        size_t n = 0;
        CHECK(d_rc_get(&t, b, 7, &text, &n) == D_OK && n == 5 &&
              !memcmp(text, "right", 5));
        CHECK(d_rc_release(&t, a) == D_OK);
        CHECK(d_rc_get(&t, b, 0, &text, &n) == D_OK && n == 4 &&
              !memcmp(text, "left", 4));
        if (!fail)
        {
            CHECK(d_rc_release(&t, b) == D_OK);
            CHECK(d_rc_get(&t, c, 0, &text, &n) == D_OK && n == 4);
            CHECK(d_rc_release(&t, c) == D_OK);
        }
        else
        {
            CHECK(d_rc_release(&t, b) == D_OK);
        }
        CHECK(t.live == 0 && s.live == 0 && s.drops == s.clones - (fail == 5));
    }
    struct d_strings s = {0};
    struct d_rc_tree t;
    d_rc_init(&t, (struct d_text_ops){&s, clone, drop});
    unsigned root;
    CHECK(d_rc_update(&t, 0, 3, NULL, 0, &root) == D_OK);
    for (unsigned i = 1; i < 32; ++i)
    {
        CHECK(d_rc_retain(&t, root) == D_OK);
    }
    CHECK(d_rc_retain(&t, root) == D_FULL);
    for (unsigned i = 0; i < 32; ++i)
    {
        CHECK(d_rc_release(&t, root) == D_OK);
    }
    CHECK(!t.live && !s.live && d_rc_release(&t, root) == D_INVALID);
    d_rc_init(&t, (struct d_text_ops){&s, clone, drop});
    CHECK(d_rc_update(&t, 0, 3, (const unsigned char *)"owned", 5, &root) ==
          D_OK);
    CHECK(d_rc_retain(&t, root) == D_OK);
    struct d_rc_retire work = {0};
    unsigned done = 99;
    CHECK(d_rc_retire(&t, root, &work) == D_OK);
    CHECK(d_rc_drain(&work, 0, &done) == D_OK && !done && t.live == 4);
    CHECK(d_rc_drain(&work, 1, &done) == D_OK && done && t.live == 4);
    CHECK(d_rc_retire(&t, root, &work) == D_OK);
    unsigned steps = 0;
    do
    {
        unsigned before = t.reclaimed;
        CHECK(d_rc_drain(&work, 1, &done) == D_OK);
        CHECK(t.reclaimed <= before + 1);
        ++steps;
    } while (!done);
    CHECK(steps == 4 && !t.live && !s.live);
    CHECK(d_rc_drain(&work, 0, &done) == D_OK && done);
}

int main(int argc, char **argv)
{
    CHECK(argc == 2);
    test_name = argv[1];
    if (!strcmp(argv[1], "zipper"))
    {
        zipper();
    }
    else if (!strcmp(argv[1], "hamt_delete"))
    {
        deletion();
    }
    else if (!strcmp(argv[1], "refcount"))
    {
        refcount();
    }
    else if (!strcmp(argv[1], "oracle"))
    {
        oracle_output = 1;
        deletion();
    }
    else
    {
        CHECK(0);
    }
    return 0;
}
