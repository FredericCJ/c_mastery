/*
 * Test driver for the cursor laboratory.
 *
 * Usage: tests CASE, with CASE a decimal selector 0..4 (see main).
 * Exit status: 0 the case passed; 41 a check failed (a REQUIRE prints its
 * text to stderr after the marker OC_ASSERT:; the allocated-rows case prints
 * OC_ALLOCATED_ASSERT: on stdout); 2 usage error or allocation failure.
 */
#include "cursor.h"
#include "cm_parse.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

enum
{
    CASE_COUNT = 5
};

#define REQUIRE(x)                                 \
    do                                             \
    {                                              \
        if (!(x))                                  \
        {                                          \
            fprintf(stderr, "OC_ASSERT:%s\n", #x); \
            return 41;                             \
        }                                          \
    } while (0)

static int matrix(void)
{
    int a[2][3] = {{1, 2, 3}, {4, 5, 6}};
    int out = 77;
    struct oc_rows rows = {a, 2};
    struct oc_matrix m = {&rows, 2, 3, oc_rows_read};
    unsigned char bytes[sizeof a];
    REQUIRE(oc_read(&m, 0, 0, &out) == 0 && out == 1);
    REQUIRE(oc_read(&m, 1, 2, &out) == 0 && out == 6);
    REQUIRE(oc_read(&m, 2, 0, &out) == -1 && out == 6);
    REQUIRE(oc_read(&m, 0, 3, &out) == -1 && out == 6);
    REQUIRE(oc_representation(&a, bytes, sizeof bytes) == 0 &&
            !memcmp(bytes, &a, sizeof a));
    REQUIRE(oc_representation(&a, bytes, sizeof bytes - 1) == -1);
    {
        int first[3] = {7, 8, 9};
        int second[3] = {10, 11, 12};
        int *table[2] = {first, second};
        size_t lengths[2] = {3, 3};
        struct oc_table t = {table, lengths, 2};
        struct oc_matrix f = {&t, 2, 3, oc_table_read};
        REQUIRE(oc_read(&f, 1, 2, &out) == 0 && out == 12);
        REQUIRE(oc_read(&f, 0, 3, &out) == -1);
    }
    puts("OC_MATRIX_PASS");
    return 0;
}

static int allocated_rows(void)
{
    int *rows[2] = {NULL, NULL};
    size_t lengths[2] = {3, 3};
    struct oc_table table = {rows, lengths, 2};
    struct oc_matrix m = {&table, 2, 3, oc_table_read};
    int result = 0;
    int out = 77;
    rows[1] = malloc(3 * sizeof(int));
    rows[0] = malloc(3 * sizeof(int));
    if (!rows[0] || !rows[1])
    {
        result = 2;
        goto done;
    }
    for (size_t r = 0; r < 2; ++r)
    {
        for (size_t c = 0; c < 3; ++c)
        {
            rows[r][c] = (int)(10 * r + c);
        }
    }
    if (oc_read(&m, 1, 2, &out) || out != 12)
    {
        result = 41;
    }
    lengths[1] = 2;
    if (oc_read(&m, 1, 2, &out) != -1 || out != 12)
    {
        result = 41;
    }
done:
    free(rows[0]);
    free(rows[1]);
    if (result)
    {
        puts("OC_ALLOCATED_ASSERT:actual row table");
    }
    else
    {
        puts("OC_ALLOCATED_PASS");
    }
    return result;
}

static int cursors(void)
{
    const unsigned char a[4] = {10, 20, 30, 40};
    struct fd_cursor c = {a, 4, 0};
    struct fd_cursor empty = {0};
    struct fd_cursor sub;
    const unsigned char *p = a;
    REQUIRE(oc_take(&c, 1, &p) == FD_OK && p == a && c.position == 1);
    REQUIRE(oc_take(&c, 4, &p) == FD_RANGE && p == a && c.position == 1);
    REQUIRE(oc_subview(&c, 1, 2, &sub) == FD_OK && sub.base == a + 2 &&
            sub.length == 2 && sub.position == 0 && c.position == 1);
    REQUIRE(oc_subview(&c, 1, 2, &c) == FD_OK && c.base == a + 2 &&
            c.length == 2 && c.position == 0);
    REQUIRE(oc_take(&empty, 0, &p) == FD_OK && p == NULL);
    REQUIRE(oc_subview(&empty, 0, 0, &empty) == FD_OK && !empty.base);
    c = (struct fd_cursor){a, 4, 4};
    REQUIRE(oc_take(&c, 0, &p) == FD_OK && p == a + 4);
    REQUIRE(oc_previous(&c, 2, &p) == FD_OK && p == a + 2 && p[0] == 30 &&
            c.position == 2);
    REQUIRE(oc_previous(&c, 3, &p) == FD_RANGE && c.position == 2 &&
            p == a + 2);
    REQUIRE(oc_previous(&c, 2, &p) == FD_OK && p == a && c.position == 0);
    REQUIRE(oc_previous(&c, 1, &p) == FD_RANGE && c.position == 0);
    REQUIRE(oc_previous(&empty, 0, &p) == FD_OK && p == NULL);
    puts("OC_CURSORS_PASS");
    return 0;
}

static int order(void)
{
    unsigned char a[4] = {0};
    ptrdiff_t d = 77;
    int order = 9;
    size_t lengths[3] = {2, 0, 3};
    size_t overflow[2] = {SIZE_MAX, 1};
    REQUIRE(oc_difference(a, 4, 4, 0, &d) == 0 && d == 4);
    REQUIRE(oc_difference(a, 4, 0, 4, &d) == 0 && d == -4);
    REQUIRE(oc_difference(a, 4, 5, 0, &d) == -1 && d == -4);
    REQUIRE(oc_order(lengths, 3, (struct oc_position){0, 2},
                     (struct oc_position){2, 0}, &order) == 0 &&
            order == 0);
    REQUIRE(oc_order(lengths, 3, (struct oc_position){0, 1},
                     (struct oc_position){2, 0}, &order) == 0 &&
            order == -1);
    REQUIRE(oc_order(lengths, 3, (struct oc_position){2, 3},
                     (struct oc_position){0, 0}, &order) == 0 &&
            order == 1);
    REQUIRE(oc_order(overflow, 2, (struct oc_position){0, 0},
                     (struct oc_position){1, 0}, &order) == -1 &&
            order == 1);
    puts("OC_ORDER_PASS");
    return 0;
}

static int conversions(void)
{
    int values[3] = {11, 22, 33};
    int other = 77;
    int *p = 0;
    int *q = (void *)0;
    int *out = &other;
    unsigned char wire[4] = {0};
    unsigned char bad[4] = {0, 0, 0, 3};
    REQUIRE(p == q && !p);
    REQUIRE(oc_roundtrip(&other) == 0);
    REQUIRE(oc_roundtrip(NULL) == 0);
    REQUIRE(oc_encode_index(values, 3, NULL, wire) == 0 && wire[0] == 255 &&
            wire[3] == 255);
    REQUIRE(oc_decode_index(values, 3, wire, &out) == 0 && out == NULL);
    REQUIRE(oc_encode_index(values, 3, &values[2], wire) == 0 && wire[3] == 2);
    REQUIRE(oc_decode_index(values, 3, wire, &out) == 0 && out == &values[2] &&
            *out == 33);
    REQUIRE(oc_decode_index(values, 3, bad, &out) == -1 && out == &values[2]);
    REQUIRE(oc_encode_index(values, 3, &other, wire) == -1 && wire[3] == 2);
#ifdef UINTPTR_MAX
    {
        uintptr_t before = (uintptr_t)(void *)&other;
        void *round = (void *)before;
        REQUIRE(round == &other);
        printf("OC_UINTPTR_HISTORY:%" PRIuPTR "\n", before);
    }
#endif
    {
        int *first = malloc(sizeof(int));
        int *next = malloc(sizeof(int));
        if (!first || !next)
        {
            free(first);
            free(next);
            return 2;
        }
        *first = 7;
        *next = *first;
#ifdef UINTPTR_MAX
        uintptr_t history = (uintptr_t)(void *)first;
#endif
        free(first);
        first = NULL;
#ifdef UINTPTR_MAX
        printf("OC_RELOCATION_HISTORY:%" PRIuPTR " fresh:%" PRIuPTR "\n",
               history, (uintptr_t)(void *)next);
#endif
        int valid = *next == 7;
        free(next);
        next = NULL;
        REQUIRE(valid);
    }
    puts("OC_CONVERSIONS_PASS");
    return 0;
}

int main(int argc, char **argv)
{
    unsigned selected = 0;
    if (argc != 2 ||
        cm_parse_unsigned(argv[1], 0, CASE_COUNT - 1, &selected) != CM_PARSE_OK)
    {
        return 2;
    }
    switch (selected)
    {
        case 0:
            return matrix();
        case 1:
            return allocated_rows();
        case 2:
            return cursors();
        case 3:
            return order();
        case 4:
            return conversions();
        default:
            return 2;
    }
}
