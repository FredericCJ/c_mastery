#include "core.h"
#include <stdio.h>
#include <string.h>
#define CHECK(x)                                                              \
    do                                                                        \
    {                                                                         \
        if (!(x))                                                             \
        {                                                                     \
            fprintf(stderr, "B06_CHECK %s:%d: %s\n", __FILE__, __LINE__, #x); \
            return 1;                                                         \
        }                                                                     \
    } while (0)

static int test_add(void)
{
    uint32_t o = 17;
    unsigned a, b;
    CHECK(!vf_add(UINT32_MAX, 1, &o) && o == 17);
    for (a = 0; a < 256; ++a)
        for (b = 0; b < 256; ++b)
        {
            CHECK(vf_add(a, b, &o) && o == a + b);
        }
    CHECK(vf_add(UINT32_MAX, 0, &o) && o == UINT32_MAX);
    return 0;
}

static int test_search(void)
{
    uint32_t a[] = {1, 2, 2, 4};
    CHECK(vf_lower_bound(a, 4, 2) == 1);
    CHECK(vf_lower_bound(a, 4, 0) == 0);
    CHECK(vf_lower_bound(a, 4, 5) == 4);
    CHECK(vf_lower_bound(0, 0, 4) == 0);
    return 0;
}

static int test_ring(void)
{
    struct vf_ring r = {0, 0, 1};
    CHECK(!vf_ring_step(&r, 1));
    CHECK(vf_ring_step(&r, 0) && r.size == 1);
    CHECK(!vf_ring_step(&r, 0) && r.size == 1);
    CHECK(vf_ring_step(&r, 1) && r.head == 0 && r.size == 0);
    r.capacity = 0;
    CHECK(!vf_ring_step(&r, 0));
    return 0;
}

static int test_frame(void)
{
    unsigned char a[] = {2, 3, 5, 6};
    unsigned n = 99;
    CHECK(vf_frame(a, 4, &n) && n == 2);
    a[3] = 7;
    n = 99;
    CHECK(!vf_frame(a, 4, &n) && n == 99);
    CHECK(!vf_frame(a, 3, &n));
    a[0] = 0;
    a[1] = 0;
    CHECK(vf_frame(a, 2, &n) && n == 0);
    return 0;
}

static int test_shrink(void)
{
    unsigned char a[] = {1, 1, 9};
    size_t n = vf_shrink(a, 3, 9);
    CHECK(n == 1 && a[0] == 9);
    a[0] = 1;
    CHECK(vf_shrink(a, 1, 9) == 1);
    return 0;
}

static int test_schedule(void)
{
    struct vf_request r = {0, 0, 0, 0};
    CHECK(vf_request(&r, 0));
    CHECK(vf_request(&r, 1));
    CHECK(!vf_request(&r, 3));
    CHECK(vf_request(&r, 2));
    CHECK(vf_request(&r, 3));
    CHECK(!vf_request(&r, 3));
    return 0;
}

static int test_reset(void)
{
    unsigned char a[] = {2, 3, 5, 6}, b[] = {1, 2, 2};
    uint32_t x, y, z;
    CHECK(vf_digest(a, 4, &x));
    CHECK(vf_digest(b, 3, &y));
    CHECK(vf_digest(a, 4, &z) && x == z && x == 104);
    return 0;
}

static int test_claims(void)
{
    CHECK(vf_claim(1, 1, 1, 0));
    CHECK(!vf_claim(1, 0, 1, 0));
    CHECK(!vf_claim(1, 1, 0, 0));
    CHECK(!vf_claim(1, 1, 1, 1));
    CHECK(!vf_claim(2, 1, 1, 0));
    return 0;
}

int main(int argc, char **argv)
{
    if (argc != 2)
        return 2;
    if (strcmp(argv[1], "add") == 0)
        return test_add();
    if (strcmp(argv[1], "search") == 0)
        return test_search();
    if (strcmp(argv[1], "ring") == 0)
        return test_ring();
    if (strcmp(argv[1], "frame") == 0)
        return test_frame();
    if (strcmp(argv[1], "shrink") == 0)
        return test_shrink();
    if (strcmp(argv[1], "schedule") == 0)
        return test_schedule();
    if (strcmp(argv[1], "reset") == 0)
        return test_reset();
    if (strcmp(argv[1], "claims") == 0)
        return test_claims();
    return 2;
}
