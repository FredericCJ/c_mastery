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

static int test_stack(void)
{
    uint64_t x = 99;
    CHECK(po_stack(4096, 9, &x) && x == 4080);
    CHECK(po_stack(4096, 1, &x) && x == 4080);
    CHECK(!po_stack(8, 9, &x) && x == 4080);
    CHECK(po_stack(8, 0, &x) && x == 0);
    return 0;
}

static int test_saved(void)
{
    CHECK(po_saved(0, 2));
    CHECK(po_saved(0, 27));
    CHECK(!po_saved(0, 28));
    CHECK(po_saved(1, 31));
    CHECK(po_saved(1, 29));
    CHECK(!po_saved(1, 18));
    CHECK(!po_saved(2, 19));
    return 0;
}

static int test_args(void)
{
    unsigned r = 99, o = 99;
    CHECK(po_arg(7, &r, &o) && r == 7 && o == 0);
    CHECK(po_arg(8, &r, &o) && r == UINT_MAX && o == 0);
    CHECK(po_arg(9, &r, &o) && o == 8);
    CHECK(!po_arg(16, &r, &o) && o == 8);
    return 0;
}

static int test_context(void)
{
    struct po_context c;
    uint64_t a[32], b[32];
    unsigned i;
    for (i = 0; i < 32; ++i)
    {
        a[i] = i + 100;
        b[i] = 999;
    }
    CHECK(po_save(&c, 0, a));
    CHECK(po_restore(&c, 0, b));
    CHECK(b[8] == 108 && b[10] == 999);
    CHECK(!po_restore(&c, 1, b));
    return 0;
}

static int test_immediate(void)
{
    CHECK(po_sign12(0xfff) == -1);
    CHECK(po_sign12(0x800) == -2048);
    CHECK(po_sign12(0x17ff) == 2047);
    CHECK(po_branch(-4096, 13, 2));
    CHECK(po_branch(4094, 13, 2));
    CHECK(!po_branch(4096, 13, 2));
    CHECK(!po_branch(3, 13, 2));
    return 0;
}

static int test_trap(void)
{
    struct po_status s = {1, 0, 0};
    CHECK(po_trap(&s, 1) && s.sie == 0 && s.spie == 1 && s.spp == 1);
    CHECK(po_return(&s) && s.sie == 1 && s.spie == 1 && s.spp == 0);
    CHECK(!po_trap(&s, 2));
    return 0;
}

static int test_privilege(void)
{
    CHECK(po_access(1, 0, 0, 0, 1, 0, 0, 0));
    CHECK(!po_access(0, 0, 0, 0, 1, 1, 1, 0));
    CHECK(!po_access(1, 1, 1, 0, 1, 1, 1, 2));
    CHECK(po_access(1, 1, 1, 1, 0, 0, 1, 0));
    CHECK(!po_access(1, 1, 0, 1, 1, 1, 1, 0));
    return 0;
}

static int test_closure(void)
{
    CHECK(po_closure(0, 0));
    CHECK(po_closure(3, 7));
    CHECK(!po_closure(3, 1));
    CHECK(!po_closure(16, 31));
    return 0;
}

int main(int argc, char **argv)
{
    if (argc != 2)
        return 2;
    if (strcmp(argv[1], "stack") == 0)
        return test_stack();
    if (strcmp(argv[1], "saved") == 0)
        return test_saved();
    if (strcmp(argv[1], "args") == 0)
        return test_args();
    if (strcmp(argv[1], "context") == 0)
        return test_context();
    if (strcmp(argv[1], "immediate") == 0)
        return test_immediate();
    if (strcmp(argv[1], "trap") == 0)
        return test_trap();
    if (strcmp(argv[1], "privilege") == 0)
        return test_privilege();
    if (strcmp(argv[1], "closure") == 0)
        return test_closure();
    return 2;
}
