#include "ipc.h"
#include <errno.h>
#include <stdio.h>
#include <string.h>

struct rd_clock
{
    unsigned tick, mode, pending, writes, reads, waits;
};

static int now(void *p, struct timespec *t)
{
    struct rd_clock *c = p;
    *t = (struct timespec){0, (long)c->tick * 1000000};
    return 0;
}

static ssize_t transfer(void *p, int sending, unsigned char *bytes, size_t n)
{
    struct rd_clock *c = p;
    (void)n;
    if (c->mode && !c->pending)
    {
        c->pending = 1;
        if (c->mode == 2)
        {
            ++c->tick;
        }
        errno = c->mode == 1 ? EAGAIN : EINTR;
        return -1;
    }
    c->pending = 0;
    ++c->tick;
    if (sending)
    {
        if (*bytes != c->writes++)
        {
            errno = EPROTO;
            return -1;
        }
    }
    else
    {
        *bytes = (unsigned char)(20 + c->reads++);
    }
    return 1;
}

static int wait_ready(void *p, int sending, struct timespec left)
{
    struct rd_clock *c = p;
    (void)sending;
    if (left.tv_sec < 0 || (left.tv_sec == 0 && left.tv_nsec <= 0))
    {
        errno = EPROTO;
        return -1;
    }
    ++c->tick;
    ++c->waits;
    return 1;
}

#define CHECK(c)                                                       \
    do                                                                 \
    {                                                                  \
        if (!(c))                                                      \
        {                                                              \
            fprintf(stderr, "IPC-BUDGET-CHECK:%s:%d\n", #c, __LINE__); \
            return 1;                                                  \
        }                                                              \
    } while (0)

int main(void)
{
    unsigned cases = 0;
    const unsigned char request[8] = {0, 1, 2, 3, 4, 5, 6, 7},
                        expected[4] = {20, 21, 22, 23};
    for (unsigned mode = 0; mode < 3; ++mode)
    {
        for (unsigned budget = 0; budget <= 26; ++budget)
        {
            struct rd_clock c = {.mode = mode};
            struct ei_io ops = {now, transfer, wait_ready, &c};
            struct ei_progress p = {0};
            unsigned char out[4] = {99, 99, 99, 99};
            int r =
                ei_exchange(ops, request, out,
                            (struct timespec){0, (long)budget * 1000000}, &p);
            unsigned cost = mode ? 2 : 1, completed = budget / cost;
            if (completed > 12)
            {
                completed = 12;
            }
            CHECK(p.sent == (completed < 8 ? completed : 8) &&
                  p.received == (completed > 8 ? completed - 8 : 0));
            if (budget > 12 * cost)
            {
                CHECK(r == 0 && !memcmp(out, expected, 4));
            }
            else
            {
                CHECK(r == -1 && errno == ETIMEDOUT && out[0] == 99 &&
                      out[1] == 99 && out[2] == 99 && out[3] == 99);
            }
            ++cases;
        }
    }
    struct rd_clock c = {0};
    struct ei_io ops = {now, transfer, wait_ready, &c};
    struct ei_progress invalid = {.sent = 9};
    unsigned char out[4] = {99, 99, 99, 99};
    CHECK(ei_exchange(ops, request, out, (struct timespec){1, 0}, &invalid) ==
              -1 &&
          errno == EINVAL && invalid.sent == 9 && !c.tick && out[0] == 99);
    printf("IPC-BUDGET:independent_cells=%u;one_absolute_budget=1;partial_"
           "progress_retained=1;\n",
           cases);
    return 0;
}
