#include "dispatch_resolution.h"
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
static const char *test_name = "arguments";
#define CHECK(x)                                                             \
    do                                                                       \
    {                                                                        \
        if (!(x))                                                            \
        {                                                                    \
            fprintf(stderr, "DS_CHECK:%s:%d:%s\n", test_name, __LINE__, #x); \
            exit(1);                                                         \
        }                                                                    \
    } while (0)

static int add(int a, int b, int *out)
{
    CHECK(a >= -100 && a <= 100 && b >= -100 && b <= 100);
    *out = a + b;
    return DS_OK;
}

static int subtract(int a, int b, int *out)
{
    CHECK(a >= -100 && a <= 100 && b >= -100 && b <= 100);
    *out = a - b;
    return DS_OK;
}

static int fail(int a, int b, int *out)
{
    (void)a;
    (void)b;
    *out = 999;
    return DS_FAILED;
}

static void resolution(void)
{
    struct ds_resolution m, other;
    struct ds_resolution_cache c;
    ds_resolution_init(&m);
    ds_resolution_init(&other);
    ds_resolution_cache_init(&c);
    int out = 91;
    CHECK(ds_resolution_call(&m, &c, 3, 3, 10, 2, &out) == DS_UNSUPPORTED &&
          out == 91);
    CHECK(ds_resolution_set(&m, 0, 0, add, NULL) == DS_OK);
    CHECK(ds_resolution_call(&m, &c, 3, 3, 10, 2, &out) == DS_OK && out == 12);
    CHECK(ds_resolution_set(&m, 1, 2, subtract, NULL) == DS_OK);
    CHECK(ds_resolution_set(&m, 2, 1, add, NULL) == DS_OK);
    out = 91;
    CHECK(ds_resolution_call(&m, &c, 3, 3, 10, 2, &out) == DS_AMBIGUOUS &&
          out == 91);
    CHECK(ds_resolution_set(&m, 3, 3, subtract, NULL) == DS_OK);
    CHECK(ds_resolution_call(&m, &c, 3, 3, 10, 2, &out) == DS_OK && out == 8);
    CHECK(ds_resolution_call(&m, &c, 1, 2, 10, 2, &out) == DS_OK && out == 8);
    CHECK(ds_resolution_call(&m, &c, 2, 1, 10, 2, &out) == DS_OK && out == 12);
    CHECK(ds_resolution_set(&m, 3, 3, NULL, NULL) == DS_OK);
    out = 91;
    CHECK(ds_resolution_call(&m, &c, 3, 3, 10, 2, &out) == DS_AMBIGUOUS &&
          out == 91);
    CHECK(ds_resolution_set(&m, 3, 3, fail, NULL) == DS_OK);
    CHECK(ds_resolution_call(&m, &c, 3, 3, 10, 2, &out) == DS_FAILED &&
          out == 91);
    CHECK(ds_resolution_call(&other, &c, 3, 3, 10, 2, &out) == DS_UNSUPPORTED &&
          out == 91);
    ds_resolution_cache_clear(&c);
    CHECK(!c.entries[0].owner && !c.entries[1].owner);
    ds_resolution_cache_init(&c);
    CHECK(ds_resolution_set(&other, 0, 0, add, NULL) == DS_OK);
    for (unsigned i = 0; i < 2; ++i)
        CHECK(ds_resolution_call(&other, &c, i, i, 10, 2, &out) == DS_OK);
    CHECK(c.hits == 0 && c.misses == 2 && c.next == 0);
    CHECK(ds_resolution_call(&other, &c, 0, 0, 10, 2, &out) == DS_OK &&
          c.hits == 1 && c.next == 0);
    CHECK(ds_resolution_call(&other, &c, 2, 2, 10, 2, &out) == DS_OK &&
          c.misses == 3 && c.next == 1);
    CHECK(ds_resolution_call(&other, &c, 1, 1, 10, 2, &out) == DS_OK &&
          c.hits == 2);
    c.hits = UINT_MAX;
    CHECK(ds_resolution_call(&other, &c, 1, 1, 10, 2, &out) == DS_OK &&
          c.hits == UINT_MAX);
    struct ds_resolution unequal;
    ds_resolution_init(&unequal);
    CHECK(ds_resolution_set(&unequal, 3, 1, add, NULL) == DS_OK &&
          ds_resolution_set(&unequal, 2, 2, subtract, NULL) == DS_OK);
    out = 91;
    CHECK(ds_resolution_uncached(&unequal, 3, 3, 10, 2, &out) == DS_AMBIGUOUS &&
          out == 91);
    ds_resolution_clear(&unequal);
    m.epoch = 31;
    ds_combine_fn old = m.entries[0][0].method;
    CHECK(ds_resolution_set(&m, 0, 0, subtract, NULL) == DS_FULL &&
          m.entries[0][0].method == old);
    CHECK(ds_resolution_call(&m, &c, 0, 0, 10, 2, &out) == DS_OK);
    ds_resolution_clear(&m);
    out = 91;
    CHECK(ds_resolution_call(&m, &c, 0, 0, 10, 2, &out) == DS_UNSUPPORTED &&
          out == 91 && m.epoch == 32);
    ds_resolution_cache_clear(&c);
    ds_resolution_clear(&m);
    ds_resolution_clear(&other);
}

static void plugin_cache(const char *path)
{
    struct ds_loaded module;
    ds_loaded_init(&module);
    CHECK(ds_loaded_open(&module, path) == DS_OK);
    struct ds_resolution m;
    struct ds_resolution_cache c;
    ds_resolution_init(&m);
    ds_resolution_cache_init(&c);
    int out = 91;
    CHECK(ds_resolution_call(&m, &c, 3, 3, 10, 2, &out) == DS_UNSUPPORTED);
    CHECK(ds_resolution_set(&m, 0, 0, NULL, &module) == DS_OK &&
          module.borrows == 1);
    CHECK(ds_resolution_call(&m, &c, 3, 3, 10, 2, &out) == DS_OK && out == 12 &&
          module.borrows == 2);
    CHECK(ds_resolution_call(&m, &c, 1, 1, 10, 2, &out) == DS_OK &&
          module.borrows == 3);
    CHECK(ds_loaded_close(&module) == DS_BUSY);
    const struct ds_plugin *held;
    CHECK(ds_loaded_acquire(&module, &held) == DS_OK && module.borrows == 4);
    out = 91;
    CHECK(ds_resolution_call(&m, &c, 2, 2, 10, 2, &out) == DS_FULL &&
          out == 91 && module.borrows == 4);
    held = NULL;
    CHECK(ds_loaded_release(&module) == DS_OK);
    CHECK(ds_resolution_set(&m, 0, 0, NULL, NULL) == DS_OK &&
          module.borrows == 2);
    CHECK(ds_loaded_close(&module) == DS_BUSY);
    CHECK(ds_resolution_call(&m, &c, 3, 3, 10, 2, &out) == DS_UNSUPPORTED);
    ds_resolution_cache_clear(&c);
    CHECK(!c.entries[0].owner && !c.entries[1].owner && !module.borrows);
    CHECK(ds_loaded_close(&module) == DS_OK && !module.handle);
    ds_resolution_clear(&m);
    CHECK(ds_loaded_open(&module, path) == DS_OK);
    ds_resolution_init(&m);
    CHECK(ds_resolution_call(&m, &c, 3, 3, 10, 2, &out) == DS_UNSUPPORTED);
    CHECK(ds_resolution_set(&m, 3, 3, NULL, &module) == DS_OK);
    CHECK(ds_resolution_call(&m, &c, 3, 3, 10, 2, &out) == DS_OK && out == 12);
    ds_resolution_cache_clear(&c);
    ds_resolution_clear(&m);
    CHECK(ds_loaded_close(&module) == DS_OK);
    (void)held;
}

static void oracle(void)
{
    const unsigned keys[4][2] = {{0, 0}, {1, 2}, {2, 1}, {3, 3}};
    for (unsigned bits = 0; bits < 16; ++bits)
    {
        struct ds_resolution m;
        struct ds_resolution_cache c;
        ds_resolution_init(&m);
        ds_resolution_cache_init(&c);
        for (unsigned i = 0; i < 4; ++i)
            if (bits & (1u << i))
                CHECK(ds_resolution_set(&m, keys[i][0], keys[i][1],
                                        i % 2 ? subtract : add, NULL) == DS_OK);
        for (unsigned l = 0; l < 4; ++l)
            for (unsigned r = 0; r < 4; ++r)
            {
                int value = 91, direct = 91;
                int status = ds_resolution_call(&m, &c, l, r, 10, 2, &value);
                CHECK(status ==
                          ds_resolution_uncached(&m, l, r, 10, 2, &direct) &&
                      value == direct);
                printf("R %u %u %u %d %d\n", bits, l, r, status, value);
            }
        ds_resolution_cache_clear(&c);
        ds_resolution_clear(&m);
    }
}

int main(int argc, char **argv)
{
    CHECK(argc >= 2);
    test_name = argv[1];
    if (!strcmp(argv[1], "resolution"))
        resolution();
    else if (!strcmp(argv[1], "plugin_cache"))
    {
        CHECK(argc == 3);
        plugin_cache(argv[2]);
    }
    else if (!strcmp(argv[1], "oracle"))
        oracle();
    else
        CHECK(0);
    return 0;
}
