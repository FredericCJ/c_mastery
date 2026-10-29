#include "owned_stream.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define C(x)                                                   \
    do                                                         \
    {                                                          \
        if (!(x))                                              \
        {                                                      \
            fprintf(stderr, "CF_CHECK:%d:%s\n", __LINE__, #x); \
            return 1;                                          \
        }                                                      \
    } while (0)

struct co_alloc
{
    unsigned calls, live, frees, fail_at, callback_fail;
};

static void *allocate(void *v, size_t n)
{
    struct co_alloc *a = v;
    a->calls++;
    if (a->calls == a->fail_at)
        return NULL;
    void *p = malloc(n);
    if (p)
        a->live++;
    return p;
}

static void release(void *v, void *p)
{
    struct co_alloc *a = v;
    a->frees++;
    a->live--;
    free(p);
}

static int map(void *v, uint32_t value, struct cf_item *out)
{
    struct co_alloc *a = v;
    unsigned char *p = allocate(a, 4);
    if (!p)
        return 1;
    *out = (struct cf_item){p, 4, {a, allocate, release}};
    for (unsigned i = 0; i < 4; i++)
        p[i] = (unsigned char)(value >> (i * 8));
    return value == a->callback_fail;
}

static uint32_t value(const unsigned char *p)
{
    uint32_t n = 0;
    for (unsigned i = 0; i < 4; i++)
        n |= (uint32_t)p[i] << (8 * i);
    return n;
}

static int mapper(void)
{
    uint32_t vals[] = {1, 2, 3, 4, 5};
    for (unsigned fail = 1; fail <= 5; fail++)
    {
        struct co_alloc a = {0, 0, 0, 0, fail};
        struct cf_source s = {vals, 5, 0};
        struct cf_item out = {0};
        for (unsigned i = 0; i < 5; i++)
        {
            int r = cf_map_next(&s, map, &a, &out);
            C(s.index == i + 1);
            if (i + 1 == fail)
                C(r == CO_IO && !out.data && !a.live);
            else
            {
                C(r == CO_OK && value(out.data) == i + 1);
                C(cf_item_drop(&out) == CO_OK && !a.live);
            }
        }
        C(cf_map_next(&s, map, &a, &out) == CO_DONE && a.calls == 5);
    }
    for (size_t na = 0; na <= 5; na++)
        for (size_t nb = 0; nb <= 5; nb++)
        {
            struct co_alloc a = {0};
            a.callback_fail = 99;
            struct cf_source x = {vals, na, 0}, y = {vals, nb, 0};
            struct cf_item u = {0}, v = {0};
            size_t count = na < nb ? na : nb;
            for (size_t i = 0; i < count; i++)
            {
                C(cf_zip_next(&x, &y, map, &a, &u, &v) == CO_OK);
                C(value(u.data) == i + 1 && value(v.data) == i + 1);
                cf_item_drop(&u);
                cf_item_drop(&v);
            }
            C(cf_zip_next(&x, &y, map, &a, &u, &v) == CO_DONE &&
              x.index == count && y.index == count && a.calls == 2 * count &&
              !a.live);
        }
    struct co_alloc a = {0, 0, 0, 2, 99};
    struct cf_source x = {vals, 5, 0}, y = {vals, 5, 0};
    struct cf_item u = {0}, v = {0};
    C(cf_zip_next(&x, &y, map, &a, &u, &v) == CO_IO && !a.live &&
      x.index == 1 && y.index == 1 && !u.data && !v.data);
    return 0;
}

static int generator(void)
{
    for (unsigned count = 0; count <= 47; count++)
    {
        struct co_alloc a = {0};
        struct cf_generator *g = NULL;
        C(cf_gen_create((struct co_allocator){&a, allocate, release}, count,
                        &g) == CO_OK);
        uint64_t x = 0, y = 1;
        for (unsigned i = 0; i < count; i++)
        {
            C(cf_gen_next(g) == CO_OK);
            const unsigned char *p = NULL;
            size_t n = 91;
            C(cf_gen_borrow(g, &p, &n) == CO_OK && n == 4 && value(p) == x);
            struct cf_item clone = {0};
            C(cf_gen_clone(g, &clone) == CO_OK && clone.data != p &&
              value(clone.data) == x);
            cf_item_drop(&clone);
            if (i % 2)
            {
                struct cf_item moved = {0};
                C(cf_gen_take(g, &moved) == CO_OK && !g->current.data &&
                  value(moved.data) == x);
                C(cf_gen_next(g) == (i + 1 < count ? CO_OK : CO_DONE));
                C(value(moved.data) == x);
                cf_item_drop(&moved);
                if (i + 1 < count)
                {
                    uint64_t z = x + y;
                    x = y;
                    y = z;
                    i++;
                    C(value(g->current.data) == x);
                }
            }
            uint64_t z = x + y;
            x = y;
            y = z;
        }
        C(cf_gen_next(g) == CO_DONE);
        struct cf_generator copy = *g;
        C(cf_gen_next(&copy) == CO_INVALID);
        C(cf_gen_destroy(&g) == CO_OK && !g && !a.live);
        C(cf_gen_destroy(&g) == CO_OK);
    }
    struct co_alloc a = {0, 0, 0, 1, 0};
    struct cf_generator *g = NULL;
    C(cf_gen_create((struct co_allocator){&a, allocate, release}, 3, &g) ==
          CO_RANGE &&
      !g && !a.live);
    a.fail_at = 0;
    C(cf_gen_create((struct co_allocator){&a, allocate, release}, 3, &g) ==
      CO_OK);
    a.fail_at = a.calls + 1;
    struct cf_generator old = *g;
    C(cf_gen_next(g) == CO_RANGE && !memcmp(g, &old, sizeof old));
    a.fail_at = 0;
    C(cf_gen_next(g) == CO_OK);
    struct cf_item out = {0};
    a.fail_at = a.calls + 1;
    C(cf_gen_clone(g, &out) == CO_RANGE && !out.data && g->current.data);
    C(cf_gen_destroy(&g) == CO_OK && !a.live);
    return 0;
}

int main(int argc, char **argv)
{
    if (argc != 2)
        return 2;
    if (!strcmp(argv[1], "mapper"))
        return mapper();
    if (!strcmp(argv[1], "generator"))
        return generator();
    return 2;
}
