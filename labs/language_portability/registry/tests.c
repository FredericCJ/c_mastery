#include "registry.h"
#include <stdio.h>
#include <string.h>
#define CHECK(x)                                  \
    do                                            \
    {                                             \
        if (!(x))                                 \
        {                                         \
            fprintf(stderr, "LP_CHECK:%s\n", #x); \
            return 1;                             \
        }                                         \
    } while (0)

static int unknown(void)
{
    int x = 77;
    const char *s = "sentinel";
    uint32_t id = 88;
    CHECK(lr_dispatch(3, 0, &x) == -1 && x == 77);
    CHECK(lr_dispatch(1, 101, &x) == -1 && x == 77);
    CHECK(lr_name(3, &s) == -1 && !strcmp(s, "sentinel"));
    CHECK(lr_id("unknown", &id) == -1 && id == 88);
    CHECK(lr_at(3, &id, &s) == -1 && id == 88);
    CHECK(lr_dispatch(1, 0, &x) == 0 && x == 1);
    puts("LP unknown validated");
    return 0;
}

static int association(void)
{
    const uint32_t ids[] = {1, 4, 7};
    const char *words[] = {"start", "stop", "status"};
    CHECK(lr_count() == 3);
    for (size_t i = 0; i < 3; ++i)
    {
        int value;
        uint32_t id;
        const char *name;
        CHECK(lr_at(i, &id, &name) == 0 && id == ids[i] &&
              !strcmp(name, words[i]));
        CHECK(lr_name(id, &name) == 0 && !strcmp(name, words[i]));
        CHECK(lr_id(name, &id) == 0 && id == ids[i]);
        CHECK(lr_dispatch(id, 10, &value) == 0 && value == 10 + (int)id);
        CHECK(lr_probes(id) <= 2);
    }
    puts("LP association validated");
    return 0;
}

static int generic(void)
{
    struct lr_generic r;
    CHECK(lr_generic(0, &r) == 0 && r.value == 4);
    CHECK(lr_generic(1, &r) == 0 && r.real == 3.5);
    CHECK(lr_generic(2, &r) == 0 && r.value == 11);
    CHECK(lr_generic(3, &r) == 0 && r.value == 3);
    CHECK(lr_generic(4, &r) == 0 && r.value == 7);
    CHECK(lr_generic(5, &r) == 0 && r.value == 14);
    CHECK(lr_generic(6, &r) == 0 && r.value == 4 && r.evaluated == 1);
    CHECK(lr_generic(7, &r) == 0 && r.value == 4 && r.evaluated == 1);
    CHECK(lr_generic(8, &r) == 0 && r.value == 10);
    r.value = 99;
    CHECK(lr_generic(9, &r) == -1 && r.value == 99);
    puts("LP generic validated");
    return 0;
}

/* Dump every dispatch result for ids 0..15 and inputs 0..3, then every
   generic scenario, for comparison with an independent oracle. */
static int oracle(void)
{
    for (uint32_t id = 0; id < 16; ++id)
    {
        for (int x = 0; x < 4; ++x)
        {
            int out = 77;
            int status = lr_dispatch(id, x, &out);
            printf("D %u %d %d %d\n", id, x, status, out);
        }
    }
    for (unsigned n = 0; n < 9; ++n)
    {
        struct lr_generic r;
        CHECK(lr_generic(n, &r) == 0);
        printf("G %u %d %d %.1f %u\n", n, r.selection, r.value, r.real,
               r.evaluated);
    }
    return 0;
}

static int metrics(void)
{
    printf(
        "{\"entries\":%zu,\"pointer_bytes\":%zu,\"generic_bytes\":%zu,\"allocations\":0}\n",
        lr_count(), sizeof(void (*)(void)), sizeof(struct lr_generic));
    return 0;
}

struct lp_case
{
    const char *name;
    int (*run)(void);
};

static const struct lp_case cases[] = {
    {"unknown", unknown}, {"association", association}, {"generic", generic},
    {"oracle", oracle},   {"metrics", metrics},
};

/* Exit status: 0 pass, 1 failed check, 2 usage error or unknown case. */
int main(int argc, char **argv)
{
    if (argc != 2)
    {
        return 2;
    }
    for (size_t i = 0; i < sizeof cases / sizeof cases[0]; i++)
    {
        if (!strcmp(argv[1], cases[i].name))
        {
            return cases[i].run();
        }
    }
    return 2;
}
