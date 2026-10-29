#include "depth.h"
#include BF_B02_FACADE
#include "facade.h"
#ifndef CASE
#define CASE 0
#endif
#define A B
#define B 73
#define KIND int
#if CASE == 1
int probe(void)
{
    return B2_IDENTITY(2.5);
}
#elif CASE == 2
int probe(int *p)
{
    return B2_IDENTITY(p);
}
#elif CASE == 3
int probe(void)
{
    struct bf_record r = {1};
    return B2_IDENTITY(r);
}
#elif CASE == 4
int probe(void)
{
    int x = 1;
    return _Generic(x, int: x, default: x.member);
}
#elif CASE == 5
int probe(const struct bf_record *r)
{
    BF_BORROW(r)->value = 3;
    return 0;
}
#elif CASE == 6
int probe(void)
{
    return BF_BINARY(1, 2u);
}
#elif CASE == 7
BF_CAT_RAW(+, *)
#elif CASE == 8
int probe(void)
{
    return BF_CAT(b2_identity_, KIND)((struct bf_record){1});
}
#else
#include <stdio.h>
#include <string.h>
_Static_assert(_Generic(B2_IDENTITY(1), int: 1, default: 0),
               "identity int result");
_Static_assert(_Generic(B2_IDENTITY(1u), unsigned: 1, default: 0),
               "identity unsigned result");
_Static_assert(_Generic(BF_BINARY(1, 2), int: 1, default: 0),
               "binary int result");
_Static_assert(_Generic(BF_BINARY(1u, 2u), unsigned: 1, default: 0),
               "binary unsigned result");
_Static_assert(_Generic(BF_BORROW((struct bf_record *)0),
                   struct bf_record *: 1,
                   default: 0),
               "mutable borrow result");
_Static_assert(_Generic(BF_BORROW((const struct bf_record *)0),
                   const struct bf_record *: 1,
                   default: 0),
               "const borrow result");
static unsigned counts[2], n;
static char order[3];

static int left(void *p)
{
    (void)p;
    ++counts[0];
    order[n++] = 'L';
    return 7;
}

static int right(void *p)
{
    (void)p;
    ++counts[1];
    order[n++] = 'R';
    return 9;
}

static int same(int x)
{
    return b2_identity_int(x);
}

#define same(x) ((x) + 1)

int main(void)
{
    int i = 7, out = 0;
    unsigned u = 9;
    int (*fn)(int) = same;
    if (B2_IDENTITY(i++) != 7 || i != 8 || B2_IDENTITY(u++) != 9 || u != 10 ||
        fn(3) != 3 || same(3) != 4 || (same)(3) != 3)
        return 1;
    if (strcmp(B2_STRING_RAW(A), "A") || strcmp(B2_STRING(A), "73") ||
        BF_CAT(b2_identity_, KIND)(73) != 73)
        return 1;
    if (BF_BINARY(left(0), right(0)) != 709 || n != 2 || counts[0] != 1 ||
        counts[1] != 1)
        return 1;
    printf("direct=%c%c counts=%u,%u\n", order[0], order[1], counts[0],
           counts[1]);
    n = 0;
    counts[0] = counts[1] = 0;
    if (bf_sequence(left, 0, right, 0, &out) || out != 709 || n != 2 ||
        order[0] != 'L' || order[1] != 'R' || counts[0] != 1 || counts[1] != 1)
        return 1;
    printf("sequenced=%c%c counts=%u,%u raw=%s expanded=%s\n", order[0],
           order[1], counts[0], counts[1], B2_STRING_RAW(A), B2_STRING(A));
    return 0;
}
#endif
