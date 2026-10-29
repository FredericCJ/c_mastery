#include "loader.h"
#include <stdio.h>
#define CHECK(x)                                          \
    do                                                    \
    {                                                     \
        if (!(x))                                         \
        {                                                 \
            fprintf(stderr, "OC_LOADER_ASSERT:%s\n", #x); \
            return 41;                                    \
        }                                                 \
    } while (0)
#ifdef OC_CLOSE_FAILURE
static int fail_close;
int __real_dlclose(void *);

int __wrap_dlclose(void *handle)
{
    if (fail_close)
    {
        fail_close = 0;
        return -1;
    }
    return __real_dlclose(handle);
}
#endif
int main(int argc, char **argv)
{
    struct oc_module m = {0};
    const struct oc_plugin *a = NULL, *b = NULL;
    int result = 77;
    if (argc != 5)
        return 2;
#ifdef OC_CLOSE_FAILURE
    fail_close = 1;
    CHECK(oc_open(&m, argv[2]) == OC_LOAD_BAD && m.handle && !m.table);
    CHECK(oc_close(&m) == OC_LOAD_OK && !m.handle);
#endif
    CHECK(oc_open(&m, argv[2]) == OC_LOAD_BAD && !m.handle && !m.table &&
          !m.borrows);
    CHECK(oc_open(&m, argv[3]) == OC_LOAD_BAD && !m.handle);
    CHECK(oc_open(&m, argv[4]) == OC_LOAD_BAD && !m.handle);
    CHECK(oc_open(&m, argv[1]) == OC_LOAD_OK);
    CHECK(oc_borrow(&m, &a) == OC_LOAD_OK && a->apply(9) == 12);
    CHECK(oc_borrow(&m, &b) == OC_LOAD_OK && b == a && m.borrows == 2);
    CHECK(oc_close(&m) == OC_LOAD_BUSY && m.handle && a->apply(8) == 11);
    CHECK(oc_return(&m, &a) == OC_LOAD_OK && !a && m.borrows == 1);
    CHECK(oc_close(&m) == OC_LOAD_BUSY && b->apply(7) == 10);
    CHECK(oc_return(&m, &b) == OC_LOAD_OK && !b && !m.borrows);
    CHECK(oc_function_lookup(&m, &result) == OC_LOAD_OK && result == 12);
#ifdef OC_CLOSE_FAILURE
    fail_close = 1;
    CHECK(oc_close(&m) == OC_LOAD_BAD && m.handle && m.table && !m.borrows);
#endif
    CHECK(oc_close(&m) == OC_LOAD_OK && !m.handle && !m.table);
    CHECK(oc_close(&m) == OC_LOAD_BAD);
    CHECK(oc_function_lookup(&m, &result) == OC_LOAD_BAD && result == 12);
    puts("OC_LOADER_PASS");
    return 0;
}
