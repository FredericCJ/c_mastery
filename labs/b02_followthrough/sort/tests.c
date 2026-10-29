#include "sort.h"
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define CHECK(x)                                               \
    do                                                         \
    {                                                          \
        if (!(x))                                              \
        {                                                      \
            fprintf(stderr, "BF_CHECK:%d:%s\n", __LINE__, #x); \
            exit(1);                                           \
        }                                                      \
    } while (0)

static void check_sorted(const struct b2_item *a, size_t n)
{
    unsigned seen = 0;
    for (size_t i = 0; i < n; ++i)
    {
        CHECK(a[i].serial < n && !(seen & (1u << a[i].serial)));
        seen |= 1u << a[i].serial;
        if (i)
        {
            CHECK(a[i - 1].key < a[i].key ||
                  (a[i - 1].key == a[i].key && a[i - 1].serial < a[i].serial));
        }
    }
}

static void stable(void)
{
    struct b2_item a[4] = {{2, 0}, {1, 1}, {2, 2}, {1, 3}}, b[4], s[4];
    for (unsigned i = 0; i < 4; ++i)
    {
        b[i] = a[i];
    }
    CHECK(b2_sort(a, 4, s, 4) == B2_OK);
    check_sorted(a, 4);
    struct bf_work work;
    CHECK(bf_sort_bottom(b, 4, s, 4, &work) == B2_OK);
    check_sorted(b, 4);
    for (unsigned i = 0; i < 4; ++i)
    {
        CHECK(a[i].key == b[i].key && a[i].serial == b[i].serial);
    }
    struct b2_item extrema[5] = {{INT_MAX, 0},
                                 {INT_MIN, 1},
                                 {0, 2},
                                 {INT_MAX, 3},
                                 {INT_MIN, 4}},
                   space[5];
    CHECK(bf_sort_bottom(extrema, 5, space, 5, &work) == B2_OK);
    check_sorted(extrema, 5);
    puts(
        "BF stable validated actual-B02 bottom-up equality extrema odd-length");
}

static void admission(void)
{
    struct bf_work work = {99, 98, 97};
    struct b2_item a[2] = {{7, 0}, {-1, 1}}, s[2];
    CHECK(bf_sort_bottom(a, 2, s, 1, &work) == B2_FULL && a[0].key == 7 &&
          a[1].key == -1 && work.writes == 98);
    CHECK(b2_sort(a, 2, s, 1) == B2_FULL && a[0].key == 7 && a[1].key == -1);
    CHECK(bf_sort_bottom(NULL, 0, NULL, 0, &work) == B2_OK && !work.writes &&
          b2_sort(NULL, 0, NULL, 0) == B2_OK);
    CHECK(bf_sort_bottom(a, 1, NULL, 0, &work) == B2_OK && !work.writes &&
          b2_sort(a, 1, NULL, 0) == B2_OK);
    CHECK(bf_sort_bottom(a, 65, s, 2, &work) == B2_RANGE &&
          bf_sort_bottom(NULL, 1, s, 2, &work) == B2_INVALID);
    size_t next = 77;
    CHECK(bf_sort_width(SIZE_MAX, SIZE_MAX / 2, &next) == B2_OK &&
          next == SIZE_MAX - 1);
    CHECK(bf_sort_width(SIZE_MAX, SIZE_MAX - 1, &next) == B2_OK &&
          next == SIZE_MAX);
    next = 77;
    CHECK(bf_sort_width(SIZE_MAX, 0, &next) == B2_INVALID && next == 77 &&
          bf_sort_width(1, 1, &next) == B2_INVALID && next == 77);
    puts(
        "BF admission validated exact-short empty-singleton SIZE_MAX width without overflow");
}

static void indirect(void)
{
    struct bf_record a[4];
    size_t indices[4] = {77, 77, 77, 77}, s[4];
    int keys[4] = {2, 1, 2, 1};
    unsigned destroyed = 0;
    for (unsigned i = 0; i < 4; ++i)
    {
        a[i].key = keys[i];
        a[i].serial = i;
        a[i].self = &a[i];
        a[i].length = 96;
        a[i].owned = malloc(96);
        CHECK(a[i].owned);
        memset(a[i].owned, (int)(i + 1), 96);
    }
    struct bf_work work = {99, 98, 97};
    CHECK(bf_sort_indirect(a, 4, indices, 3, s, 4, &work) == B2_FULL &&
          indices[0] == 77 && work.writes == 98);
    CHECK(bf_sort_indirect(a, 4, indices, 4, s, 3, &work) == B2_FULL &&
          indices[3] == 77);
    CHECK(bf_sort_indirect(a, 4, indices, 4, s, 4, &work) == B2_OK &&
          !work.payload_writes);
    size_t expected[4] = {1, 3, 0, 2};
    for (unsigned i = 0; i < 4; ++i)
    {
        CHECK(indices[i] == expected[i] && a[i].self == &a[i] &&
              a[i].key == keys[i]);
        for (unsigned j = 0; j < 96; ++j)
        {
            CHECK(a[i].owned[j] == i + 1);
        }
        free(a[i].owned);
        a[i].owned = NULL;
        ++destroyed;
    }
    CHECK(destroyed == 4);
    CHECK(bf_sort_indirect(NULL, 0, NULL, 0, NULL, 0, &work) == B2_OK &&
          !work.writes);
    printf(
        "BF indirect validated stable-bijection owning-records-unmoved released%u payload-bytes384\n",
        destroyed);
}

static int oracle(void)
{
    unsigned n;
    char keys[10];
    while (scanf("%u %9s", &n, keys) == 2)
    {
        CHECK(n <= 7);
        struct b2_item a[7], b[7], s[7];
        struct bf_record r[7];
        size_t indices[7], scratch[7];
        struct bf_work bw, iw;
        for (unsigned i = 0; i < n; ++i)
        {
            a[i] = (struct b2_item){keys[i] - '1', i};
            b[i] = a[i];
            r[i] = (struct bf_record){a[i].key, i, &r[i], NULL, 0};
        }
        CHECK(b2_sort(n ? a : NULL, n, n > 1 ? s : NULL, n) == B2_OK &&
              bf_sort_bottom(n ? b : NULL, n, n > 1 ? s : NULL, n, &bw) ==
                  B2_OK &&
              bf_sort_indirect(n ? r : NULL, n, n ? indices : NULL, n,
                               n > 1 ? scratch : NULL, n, &iw) == B2_OK);
        printf("%u %zu %zu %zu %zu", n, bw.comparisons, bw.writes,
               iw.comparisons, iw.writes);
        for (unsigned i = 0; i < n; ++i)
        {
            CHECK(a[i].key == b[i].key && a[i].serial == b[i].serial &&
                  indices[i] == a[i].serial && r[i].self == &r[i] &&
                  r[i].key == keys[i] - '1');
            printf(" %u", a[i].serial);
        }
        putchar('\n');
    }
    return 0;
}

static int widths(void)
{
    size_t n, w;
    while (scanf("%zu %zu", &n, &w) == 2)
    {
        size_t out = 77;
        int rc = bf_sort_width(n, w, &out);
        printf("%d %zu\n", rc, out);
    }
    return 0;
}

static void metrics(void)
{
    struct b2_item a[16], s[16];
    struct bf_record r[16];
    size_t p[16], t[16];
    struct bf_work direct, index;
    for (unsigned i = 0; i < 16; ++i)
    {
        a[i] = (struct b2_item){(int)(i % 3) - 1, i};
        r[i] = (struct bf_record){a[i].key, i, &r[i], NULL, 0};
    }
    CHECK(bf_sort_bottom(a, 16, s, 16, &direct) == B2_OK &&
          bf_sort_indirect(r, 16, p, 16, t, 16, &index) == B2_OK);
    printf(
        "{\"n\":16,\"item_bytes\":%zu,\"index_bytes\":%zu,\"owner_descriptor_bytes\":%zu,\"record_writes\":%zu,\"index_writes\":%zu,\"scalar_bytes_written\":%zu,\"index_bytes_written\":%zu,\"owning_payload_writes\":%zu}\n",
        sizeof(struct b2_item), sizeof(size_t), sizeof(struct bf_record),
        direct.writes, index.writes, direct.writes * sizeof(struct b2_item),
        index.writes * sizeof(size_t), index.payload_writes);
}

int main(int argc, char **argv)
{
    CHECK(argc == 2);
    if (!strcmp(argv[1], "stable"))
    {
        stable();
    }
    else if (!strcmp(argv[1], "admission"))
    {
        admission();
    }
    else if (!strcmp(argv[1], "indirect"))
    {
        indirect();
    }
    else if (!strcmp(argv[1], "oracle"))
    {
        return oracle();
    }
    else if (!strcmp(argv[1], "widths"))
    {
        return widths();
    }
    else if (!strcmp(argv[1], "metrics"))
    {
        metrics();
    }
    else if (!strcmp(argv[1], "limits"))
    {
        printf("%zu\n", SIZE_MAX);
    }
    else
    {
        return 2;
    }
    return 0;
}
