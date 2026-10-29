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

static int test_unwind(void)
{
    CHECK(kl_unwind(0) == 0);
    CHECK(kl_unwind(3) == 21);
    CHECK(kl_unwind(7) == 321);
    CHECK(kl_unwind(8) == 0);
    return 0;
}

static int test_copy(void)
{
    unsigned char a[4] = {9, 9, 9, 9}, b[4] = {1, 2, 3, 4};
    CHECK(kl_copy(a, 4, b, 4, 2) == 2);
    CHECK(a[1] == 2 && a[2] == 9);
    CHECK(kl_copy(a, 2, b, 4, 4) == -1);
    CHECK(kl_copy(a, 4, b, 0, 0) == 0);
    return 0;
}

static int test_refs(void)
{
    struct kl_ref r = {1, 0};
    CHECK(kl_ref(&r, 0) == 1 && r.refs == 2);
    CHECK(kl_ref(&r, 1) == 1 && !r.dead);
    CHECK(kl_ref(&r, 1) == 1 && r.dead);
    CHECK(!kl_ref(&r, 0));
    r.refs = UINT_MAX;
    r.dead = 0;
    CHECK(!kl_ref(&r, 0));
    return 0;
}

static int test_locks(void)
{
    CHECK(kl_lock_allowed(0, 0, 0));
    CHECK(!kl_lock_allowed(1, 0, 0));
    CHECK(kl_lock_allowed(1, 1, 0));
    CHECK(!kl_lock_allowed(1, 1, 1));
    CHECK(kl_lock_allowed(1, 2, 1));
    CHECK(!kl_lock_allowed(4, 2, 0));
    return 0;
}

static int test_work(void)
{
    struct kl_work w = {0, 0, 0, 0};
    CHECK(kl_work(&w, 0));
    CHECK(kl_work(&w, 1));
    CHECK(kl_work(&w, 3));
    CHECK(!kl_work(&w, 4));
    CHECK(kl_work(&w, 2));
    CHECK(kl_work(&w, 4));
    CHECK(!kl_work(&w, 0));
    CHECK(!kl_work(&w, 4));
    return 0;
}

static int test_grace(void)
{
    struct kl_grace g = {0, 0};
    CHECK(kl_grace(&g, 0, 2));
    CHECK(kl_grace(&g, 2, 0));
    CHECK(!kl_grace(&g, 3, 0));
    CHECK(!kl_grace(&g, 0, 3));
    CHECK(kl_grace(&g, 1, 2));
    CHECK(kl_grace(&g, 3, 0));
    return 0;
}

static int test_abi(void)
{
    unsigned out = 99;
    unsigned char p[4] = {1, 2, 0x34, 0x12};
    CHECK(kl_decode(p, 4, &out) && out == 0x1234);
    p[1] = 3;
    out = 99;
    CHECK(!kl_decode(p, 4, &out) && out == 99);
    CHECK(!kl_decode(p, 3, &out));
    return 0;
}

static int test_unload(void)
{
    CHECK(!kl_can_unload(0, 0, 0));
    CHECK(!kl_can_unload(1, 0, 1));
    CHECK(!kl_can_unload(0, 1, 1));
    CHECK(kl_can_unload(0, 0, 1));
    CHECK(!kl_can_unload(0, 0, 2));
    return 0;
}

int main(int argc, char **argv)
{
    if (argc != 2)
        return 2;
    if (strcmp(argv[1], "unwind") == 0)
        return test_unwind();
    if (strcmp(argv[1], "copy") == 0)
        return test_copy();
    if (strcmp(argv[1], "refs") == 0)
        return test_refs();
    if (strcmp(argv[1], "locks") == 0)
        return test_locks();
    if (strcmp(argv[1], "work") == 0)
        return test_work();
    if (strcmp(argv[1], "grace") == 0)
        return test_grace();
    if (strcmp(argv[1], "abi") == 0)
        return test_abi();
    if (strcmp(argv[1], "unload") == 0)
        return test_unload();
    return 2;
}
