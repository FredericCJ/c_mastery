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

static int test_iov(void)
{
    size_t a[] = {0, 3, 0, 4};
    struct li_position p = {0, 0};
    CHECK(li_advance(a, 4, &p, 4) && p.index == 3 && p.offset == 1);
    CHECK(!li_advance(a, 4, &p, 4) && p.offset == 1);
    CHECK(li_advance(a, 4, &p, 3) && p.index == 4 && p.offset == 0);
    return 0;
}

static int test_readiness(void)
{
    CHECK(li_ready_after(3, 0));
    CHECK(li_ready_after(3, 1));
    CHECK(!li_ready_after(-2, 0));
    CHECK(!li_ready_after(0, 1));
    CHECK(!li_ready_after(-1, 0));
    return 0;
}

static int test_ring(void)
{
    uint32_t o = 99;
    CHECK(li_slot(UINT32_MAX - 1, 0, 4, &o) && o == 0);
    CHECK(!li_slot(0, 4, 4, &o) && o == 0);
    CHECK(!li_slot(0, 1, 3, &o));
    CHECK(!li_slot(0, 0, 0, &o));
    return 0;
}

static int test_completion(void)
{
    struct li_request r = {7, 0, 0};
    CHECK(li_event(&r, 7, 0, 0));
    CHECK(!li_event(&r, 6, 2, 3) && r.state == 1);
    CHECK(li_event(&r, 7, 1, 0));
    CHECK(!li_event(&r, 7, 3, 0));
    CHECK(li_event(&r, 7, 2, -1));
    CHECK(li_event(&r, 7, 3, 0));
    CHECK(!li_event(&r, 7, 2, 4));
    return 0;
}

static int test_cancel(void)
{
    CHECK(!li_releasable(0, 0));
    CHECK(!li_releasable(1, 0));
    CHECK(li_releasable(0, 1));
    CHECK(li_releasable(1, 1));
    CHECK(!li_releasable(2, 1));
    return 0;
}

static int test_deadline(void)
{
    unsigned o = 9;
    CHECK(li_expired(UINT32_MAX - 4, 2, 7, &o) && o == 1);
    CHECK(li_expired(UINT32_MAX - 4, 0, 7, &o) && o == 0);
    CHECK(li_expired(UINT32_MAX - 4, UINT32_MAX - 3, 7, &o) && o == 0);
    CHECK(!li_expired(0, 0, 0, &o) && o == 0);
    return 0;
}

static int test_durability(void)
{
    CHECK(li_durable(1, 1, 1, 1));
    CHECK(!li_durable(1, 1, 1, 0));
    CHECK(!li_durable(1, 0, 1, 1));
    CHECK(!li_durable(2, 1, 1, 1));
    return 0;
}

static int test_zerocopy(void)
{
    CHECK(li_buffer_release(0, 0, 0));
    CHECK(!li_buffer_release(1, 0, 0));
    CHECK(!li_buffer_release(1, 1, 1));
    CHECK(li_buffer_release(1, 1, 0));
    CHECK(!li_buffer_release(2, 1, 0));
    return 0;
}

int main(int argc, char **argv)
{
    if (argc != 2)
        return 2;
    if (strcmp(argv[1], "iov") == 0)
        return test_iov();
    if (strcmp(argv[1], "readiness") == 0)
        return test_readiness();
    if (strcmp(argv[1], "ring") == 0)
        return test_ring();
    if (strcmp(argv[1], "completion") == 0)
        return test_completion();
    if (strcmp(argv[1], "cancel") == 0)
        return test_cancel();
    if (strcmp(argv[1], "deadline") == 0)
        return test_deadline();
    if (strcmp(argv[1], "durability") == 0)
        return test_durability();
    if (strcmp(argv[1], "zerocopy") == 0)
        return test_zerocopy();
    return 2;
}
