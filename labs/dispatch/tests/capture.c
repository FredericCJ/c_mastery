#include "capture.h"
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

static void destroyed(void *p)
{
    ++*(unsigned *)p;
}

static void codec(void)
{
    const int32_t values[] = {INT32_MIN, INT32_MAX, 1, -1, 2, -2, 3, -3};
    for (size_t n = 1; n <= 8; ++n)
        for (unsigned fail = 0; fail < 2; ++fail)
        {
            struct ow_allocator pool = {0};
            pool.fail_at = fail;
            struct ds_erased owner, moved;
            ds_erased_init(&owner, ds_counted_bytes(&pool));
            ds_erased_init(&moved, ds_counted_bytes(&pool));
            unsigned drops = 0;
            int result = ds_capture_i32(&owner, values, n, destroyed, &drops);
            if (fail && n > 4)
            {
                CHECK(result == DS_FAILED && !owner.active && !drops &&
                      pool.live == 0);
                continue;
            }
            CHECK(result == DS_OK && owner.active && !drops &&
                  pool.live == (n > 4));
            int out = 91, borrowed = 92;
            int a = ds_erased_call(&owner, 3, &out),
                b = ds_borrowed_i32(values, n, 3, &borrowed);
            CHECK(a == b);
            if (a == DS_OK)
                CHECK(out == borrowed);
            else
                CHECK(out == 91 && borrowed == 92);
            CHECK(ds_erased_move(&moved, &owner) == DS_OK && !owner.active &&
                  moved.active);
            ds_erased_drop(&owner);
            CHECK(!drops);
            ds_erased_drop(&moved);
            ds_erased_drop(&moved);
            CHECK(drops == 1 && pool.live == 0 && !pool.invalid_releases &&
                  pool.releases == (n > 4));
        }

    struct ds_self
    {
        int local;
        int *pointer;
    } a = {7, NULL};

    a.pointer = &a.local;
    struct ds_self b = a;
    CHECK(b.pointer == &a.local &&
          b.pointer != &b.local); /* Copy does not relocate a self-reference. */
    struct ow_allocator pool = {0};
    struct ds_erased owners[9];
    unsigned drops = 0;
    for (unsigned i = 0; i < 9; ++i)
    {
        ds_erased_init(&owners[i], ds_counted_bytes(&pool));
        int result = ds_capture_i32(&owners[i], values, 8, destroyed, &drops);
        CHECK(result == (i < 8 ? DS_OK : DS_FAILED));
    }
    CHECK(pool.live == 8 && pool.attempts == 9 && !drops && !owners[8].active);
    for (unsigned i = 0; i < 9; ++i)
        ds_erased_drop(&owners[i]);
    CHECK(pool.live == 0 && pool.releases == 8 && drops == 8 &&
          !pool.invalid_releases);
    struct d_allocator bytes = ds_counted_bytes(&pool);
    CHECK(!bytes.allocate(bytes.context, 0) &&
          !bytes.allocate(bytes.context, 65));
    unsigned char *whole = bytes.allocate(bytes.context, 64);
    CHECK(whole && pool.live == 1);
    for (unsigned i = 0; i < 64; ++i)
        whole[i] = (unsigned char)i;
    bytes.release(bytes.context, whole);
    CHECK(pool.live == 0 && pool.releases == 9 && !pool.invalid_releases);
    struct ds_erased invalid;
    ds_erased_init(&invalid, ds_counted_bytes(&pool));
    CHECK(ds_capture_i32(&invalid, values, 0, destroyed, &drops) == DS_INVALID);
    CHECK(ds_capture_i32(&invalid, values, 9, destroyed, &drops) == DS_INVALID);
    CHECK(!invalid.active && drops == 8);
}

static void typed(void)
{
    for (size_t n = 1; n <= 8; ++n)
        for (unsigned fail = 0; fail < 2; ++fail)
        {
            int32_t values[8];
            for (unsigned i = 0; i < 8; ++i)
                values[i] = (int32_t)i - 3;
            struct ow_allocator pool = {0};
            pool.fail_at = fail;
            struct ds_typed_capture c, moved;
            ds_typed_init(&c, ow_allocator(&pool));
            ds_typed_init(&moved, ow_allocator(&pool));
            int status = ds_typed_assign(&c, values, n);
            if (fail && n > 4)
            {
                CHECK(status == DS_FAILED && c.count == 0 && pool.live == 0);
                continue;
            }
            CHECK(status == DS_OK && c.spilled == (n > 4) &&
                  pool.live == (n > 4));
            int out = 91, want = 0;
            CHECK(ds_borrowed_i32(values, n, 9, &want) == DS_OK);
            CHECK(ds_typed_call(&c, 9, &out) == DS_OK && out == want);
            CHECK(ds_typed_move(&moved, &c) == DS_OK && c.count == 0);
            ds_typed_drop(&c);
            CHECK(pool.live == (n > 4));
            ds_typed_drop(&moved);
            ds_typed_drop(&moved);
            CHECK(!pool.live && !pool.invalid_releases &&
                  pool.releases == (n > 4));
        }
    printf("capture footprints: DSmall=%zu DSErased=%zu typed_union=%zu\n",
           sizeof(struct d_small), sizeof(struct ds_erased),
           sizeof(struct ds_typed_capture));
}

static void oracle(void)
{
    const int32_t values[] = {INT32_MIN, INT32_MAX, 1, -1, 2, -2, 3, -3};
    for (size_t n = 1; n <= 8; ++n)
        for (unsigned fail = 0; fail < 2; ++fail)
            for (unsigned ai = 0; ai < 3; ++ai)
            {
                const int args[] = {INT_MIN, 3, INT_MAX};
                struct ow_allocator pool = {0};
                pool.fail_at = fail;
                struct ds_erased owner;
                unsigned drops = 0;
                ds_erased_init(&owner, ds_counted_bytes(&pool));
                int status =
                        ds_capture_i32(&owner, values, n, destroyed, &drops),
                    out = 91;
                int called = status == DS_OK
                                 ? ds_erased_call(&owner, args[ai], &out)
                                 : DS_EMPTY;
                printf("%zu %u %d %d %d %d %zu %zu ", n, fail, args[ai], status,
                       called, out, pool.attempts, pool.live);
                if (owner.active)
                {
                    const unsigned char *p = d_small_data(&owner.capture);
                    for (size_t i = 0; i < owner.capture.length; ++i)
                        printf("%02x", p[i]);
                }
                else
                    printf("-");
                ds_erased_drop(&owner);
                printf(" %zu %u %zu\n", pool.releases, drops, pool.live);
            }
}

int main(int argc, char **argv)
{
    CHECK(argc == 2);
    test_name = argv[1];
    if (!strcmp(argv[1], "capture_codec"))
        codec();
    else if (!strcmp(argv[1], "typed_capture"))
        typed();
    else if (!strcmp(argv[1], "oracle"))
        oracle();
    else
        CHECK(0);
    return 0;
}
