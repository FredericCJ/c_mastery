#include "ring.h"
#include <stdio.h>
#include <string.h>
#define CHECK(v)                                        \
    do                                                  \
    {                                                   \
        if (!(v))                                       \
        {                                               \
            fprintf(stderr, "CR_CHECK:%d\n", __LINE__); \
            return 1;                                   \
        }                                               \
    } while (0)

static int lookup(void)
{
    struct cr_token t[] = {{100, 3, 0}, {100, 2, 0}, {20, 1, 0}, {100, 2, 1}};
    struct cr_ring r;
    unsigned out = 99;
    struct cr_cost c = {99, 99};
    CHECK(cr_build(t, 4, 1, &r) == CR_OK);
    CHECK(cr_owner(&r, 100, 1, &out, &c) == CR_OK && out == 2);
    CHECK(cr_owner(&r, 101, 1, &out, &c) == CR_OK && out == 1);
    CHECK(cr_owner(&r, 65535, 1, &out, &c) == CR_OK && out == 1);
    CHECK(cr_owner(&r, 20, 1, &out, &c) == CR_OK && out == 1);
    struct cr_ring old = r;
    t[3] = t[1];
    CHECK(cr_build(t, 4, 1, &r) == CR_INVALID && !memcmp(&r, &old, sizeof r));
    CHECK(cr_build(NULL, 0, 1, &r) == CR_OK);
    out = 99;
    c = (struct cr_cost){99, 99};
    CHECK(cr_owner(&r, 0, 1, &out, &c) == CR_EMPTY && out == 99 &&
          c.comparisons == 99);
    return 0;
}

static int membership(void)
{
    struct cr_token t[] = {{10, 1, 0}, {30, 2, 0}},
                    add[] = {{20, 3, 0}, {40, 3, 1}};
    struct cr_ring r, old;
    unsigned out = 99;
    struct cr_cost c = {99, 99};
    CHECK(cr_build(t, 2, 7, &r) == CR_OK);
    old = r;
    for (size_t cut = 0; cut <= 4; cut++)
    {
        CHECK(cr_change(&r, 3, add, 2, cut) == CR_ABORTED &&
              !memcmp(&r, &old, sizeof r));
    }
    CHECK(cr_change(&r, 3, add, 2, SIZE_MAX) == CR_OK && r.epoch == 8);
    CHECK(cr_owner(&r, 15, 7, &out, &c) == CR_STALE && out == 99);
    for (unsigned key = 0; key < 65536; key++)
    {
        unsigned a, b;
        CHECK(cr_owner(&old, (uint16_t)key, 7, &a, &c) == CR_OK &&
              cr_owner(&r, (uint16_t)key, 8, &b, &c) == CR_OK);
        CHECK(a == b || b == 3);
    }
    uint16_t keys[] = {0, 10, 11, 20, 21, 30, 31, 40, 41};
    struct cr_move moves[9], prior[9];
    size_t n = 99;
    memset(moves, 99, sizeof moves);
    memcpy(prior, moves, sizeof moves);
    CHECK(cr_plan(&old, &r, keys, 9, moves, 0, &n) == CR_FULL && n == 99);
    CHECK(cr_plan(&old, &r, keys, 9, moves, 1, &n) == CR_FULL && n == 99 &&
          !memcmp(moves, prior, sizeof moves));
    CHECK(cr_plan(&old, &r, keys, 9, moves, 9, &n) == CR_OK && n == 4);
    for (size_t i = 0; i < n; i++)
    {
        CHECK(moves[i].after == 3);
    }
    CHECK(cr_change(&r, 3, NULL, 0, SIZE_MAX) == CR_OK && r.epoch == 9);
    for (unsigned key = 0; key < 65536; key++)
    {
        unsigned a, b;
        CHECK(cr_owner(&old, (uint16_t)key, 7, &a, &c) == CR_OK &&
              cr_owner(&r, (uint16_t)key, 9, &b, &c) == CR_OK && a == b);
    }
    CHECK(cr_change(&r, 3, NULL, 0, SIZE_MAX) == CR_INVALID);
    r.epoch = UINT32_MAX;
    old = r;
    CHECK(cr_change(&r, 3, add, 2, SIZE_MAX) == CR_FULL &&
          !memcmp(&old, &r, sizeof r));
    CHECK(cr_build(t, 1, 1, &r) == CR_OK &&
          cr_change(&r, 1, NULL, 0, SIZE_MAX) == CR_OK && r.count == 0);
    return 0;
}

static int replicas(void)
{
    struct cr_token t[] = {{10, 1, 0}, {20, 1, 1}, {30, 2, 0}, {40, 3, 0}};
    struct cr_ring r;
    unsigned owners[3] = {99, 99, 99};
    struct cr_cost c = {99, 99};
    CHECK(cr_build(t, 4, 1, &r) == CR_OK);
    CHECK(cr_replicas(&r, 0, 1, 3, 0, owners, &c) == CR_OK && owners[0] == 1 &&
          owners[1] == 2 && owners[2] == 3);
    CHECK(cr_replicas(&r, 0, 1, 2, 1, owners, &c) == CR_OK && owners[0] == 2 &&
          owners[1] == 3);
    owners[0] = 99;
    c = (struct cr_cost){99, 99};
    CHECK(cr_replicas(&r, 0, 1, 3, 1, owners, &c) == CR_UNAVAILABLE &&
          owners[0] == 99 && c.visited == 99);
    CHECK(cr_replicas(&r, 65535, 1, 1, 0, owners, &c) == CR_OK &&
          owners[0] == 1);
    return 0;
}

static int hash(void *ctx, const unsigned char *bytes, size_t n,
                unsigned char out[32])
{
    (void)bytes;
    (void)n;
    for (unsigned i = 0; i < 32; i++)
    {
        out[i] = 0;
    }
    out[0] = 1;
    out[1] = 2;
    return ctx != NULL;
}

static int routing(void)
{
    struct cr_token t[] = {{300, 1, 0}, {600, 2, 0}};
    struct cr_ring r;
    unsigned out = 99;
    struct cr_cost c = {99, 99};
    CHECK(cr_build(t, 2, 1, &r) == CR_OK);
    CHECK(cr_route(&r, 1, NULL, 0, hash, NULL, &out, &c) == CR_OK && out == 1);
    out = 99;
    c = (struct cr_cost){99, 99};
    CHECK(cr_route(&r, 1, NULL, 0, hash, &r, &out, &c) == CR_HASH_FAILED &&
          out == 99 && c.visited == 99);
    CHECK(cr_route(&r, 1, NULL, 1, hash, NULL, &out, &c) == CR_INVALID &&
          out == 99);
    return 0;
}

int main(int argc, char **argv)
{
    CHECK(argc == 2);
    int s = !strcmp(argv[1], "lookup")       ? lookup()
            : !strcmp(argv[1], "membership") ? membership()
            : !strcmp(argv[1], "replicas")   ? replicas()
            : !strcmp(argv[1], "routing")    ? routing()
                                             : 1;
    if (!s)
    {
        printf("CR_CHECK:%s passed\n", argv[1]);
    }
    return s;
}
