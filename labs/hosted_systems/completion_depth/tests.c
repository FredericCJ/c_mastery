#include "completion.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#define CHECK(x)                                                       \
    do                                                                 \
    {                                                                  \
        if (!(x))                                                      \
        {                                                              \
            fprintf(stderr, "COMPLETION-CHECK:%d:%s\n", __LINE__, #x); \
            exit(1);                                                   \
        }                                                              \
    } while (0)

int main(void)
{
    unsigned char storage[4][16], *buffers[4];
    for (unsigned i = 0; i < 4; ++i)
    {
        buffers[i] = storage[i];
    }
    struct co_pool p;
    uint64_t token, cancel, second;
    CHECK(co_init(&p, buffers, 16, 2) == 0 && co_quiescent(&p));
    CHECK(co_acquire(&p, 0, 16, &token) == 1 && token == 257 &&
          !co_quiescent(&p));
    CHECK(co_cancel(&p, token, &cancel) == 1 &&
          co_cancel(&p, token, &second) == 0);
    CHECK(co_complete(&p, cancel, 0, 0) == 1 && p.slot[0].state == CO_BUSY);
    CHECK(co_release(&p, token) == 0 && co_complete(&p, cancel, 0, 0) == 0);
    CHECK(co_complete(&p, token, -ECANCELED, 0) == 1 &&
          co_release(&p, token) == 1);
    CHECK(co_acquire(&p, 0, 8, &second) == 1 && second != token);
    CHECK(co_complete(&p, token, 1, 0) == 0 && p.slot[0].state == CO_BUSY);
    CHECK(co_complete(&p, second, 9, 0) == -1 &&
          co_complete(&p, second, 1, 2) == -1);
    CHECK(co_cancel(&p, second, &cancel) == 1);
    CHECK(co_complete(&p, second, 3, 0) == 1 && p.slot[0].result == 3);
    CHECK(co_release(&p, second) == 0 && !co_quiescent(&p));
    CHECK(co_complete(&p, cancel, -ENOENT, 0) == 1 &&
          co_release(&p, second) == 1);
    CHECK(co_acquire(&p, 0, 1, &token) == 0 && p.slot[0].state == CO_RETIRED &&
          co_quiescent(&p));
    unsigned permutations = 0;
    for (unsigned order = 0; order < 2; ++order)
    {
        for (unsigned outcome = 0; outcome < 3; ++outcome)
        {
            CHECK(co_init(&p, buffers, 16, 2) == 0);
            CHECK(co_acquire(&p, 1, 4, &token) == 1 &&
                  co_cancel(&p, token, &cancel) == 1);
            int ack = outcome == 0 ? 0 : outcome == 1 ? -ENOENT : -EALREADY;
            int result = outcome == 0 ? -ECANCELED : 2;
            if (order == 0)
            {
                CHECK(co_complete(&p, cancel, ack, 0) == 1);
                CHECK(co_release(&p, token) == 0);
                CHECK(co_complete(&p, token, result, 0) == 1);
            }
            else
            {
                CHECK(co_complete(&p, token, result, 0) == 1);
                CHECK(co_release(&p, token) == 0);
                CHECK(co_complete(&p, cancel, ack, 0) == 1);
            }
            CHECK(co_release(&p, token) == 1 && co_quiescent(&p));
            ++permutations;
        }
    }
    puts("COMPLETION-PASS:cancel_target_orders=6;retirement=1;duplicate_and_"
         "stale=1;pending_ack=1;");
    CHECK(permutations == 6);
}
