#include "queues.h"
#include <stdio.h>
#include <string.h>
#define CHECK(x)                                               \
    do                                                         \
    {                                                          \
        if (!(x))                                              \
        {                                                      \
            fprintf(stderr, "SQ_CHECK:%s:%d\n", #x, __LINE__); \
            return 1;                                          \
        }                                                      \
    } while (0)

int main(void)
{
    for (unsigned coarse = 0; coarse < 2; coarse++)
    {
        struct sq_world w;
        struct sq_handle a, b, c, run;
        struct sq_event event = {0}, old;
        struct sq_transfer transfer = {0};
        struct sl_stats stats = {0};
        CHECK(sq_init(&w, 2, 2, coarse) == SQ_OK);
        CHECK(sq_create(&w, 0, 0, 0, &a, &event) == SQ_OK && a.generation == 1);
        CHECK(sq_create(&w, 0, 1, 1, &b, &event) == SQ_OK &&
              sq_create(&w, 0, 1, 2, &c, &event) == SQ_OK);
        old = event;
        CHECK(sq_transfer_begin(&w, 0, 0, 1, a, &transfer, &event, &stats) ==
                  SQ_FULL &&
              memcmp(&event, &old, sizeof old) == 0 && sq_invariant(&w));
        CHECK(sq_dispatch(&w, 1, &run, &event, &stats) == SQ_OK &&
              run.slot == b.slot && sq_invariant(&w));
        CHECK(sq_transfer_begin(&w, 0, 0, 1, a, &transfer, &event, &stats) ==
                  SQ_FULL &&
              sq_invariant(&w));
        CHECK(sq_finish(&w, 1, run, SQ_EXITED, &event, &stats) == SQ_OK);
        CHECK(sq_transfer_begin(&w, 0, 0, 1, a, &transfer, &event, &stats) ==
                  SQ_OK &&
              event.to == SQ_TRANSFER && sq_invariant(&w));
        CHECK(sq_dispatch(&w, 0, &run, &event, &stats) == SQ_IDLE);
        CHECK(sq_transfer_end(&w, 0, transfer, 0, &event, &stats) == SQ_OK &&
              sq_invariant(&w));
        CHECK(sq_transfer_end(&w, 0, transfer, 1, &event, &stats) == SQ_STALE);
        CHECK(sq_transfer_begin(&w, 0, 0, 1, a, &transfer, &event, &stats) ==
                  SQ_OK &&
              sq_transfer_end(&w, 0, transfer, 1, &event, &stats) == SQ_OK &&
              sq_invariant(&w));
        CHECK(sq_dispatch(&w, 1, &run, &event, &stats) == SQ_OK &&
              run.slot == c.slot);
        CHECK(sq_finish(&w, 1, run, SQ_BLOCKED, &event, &stats) == SQ_OK &&
              sq_invariant(&w));
        CHECK(sq_wake(&w, 0, 1, c, &event, &stats) == SQ_OK);
        CHECK(sq_dispatch(&w, 1, &run, &event, &stats) == SQ_OK &&
              run.slot == a.slot);
        CHECK(sq_finish(&w, 1, run, SQ_EXITED, &event, &stats) == SQ_OK);
        old = event;
        CHECK(sq_wake(&w, 0, 1, c, &event, &stats) == SQ_DUPLICATE &&
              memcmp(&event, &old, sizeof old) == 0 && sq_invariant(&w));
        CHECK(sq_dispatch(&w, 1, &run, &event, &stats) == SQ_OK &&
              run.slot == c.slot);
        CHECK(sq_finish(&w, 1, run, SQ_EXITED, &event, &stats) == SQ_OK &&
              sq_invariant(&w));
        struct sq_handle replacement;
        CHECK(sq_create(&w, 0, 1, c.slot, &replacement, &event) == SQ_OK &&
              replacement.generation == 2);
        CHECK(sq_dispatch(&w, 1, &run, &event, &stats) == SQ_OK);
        CHECK(sq_finish(&w, 1, c, SQ_EXITED, &event, &stats) == SQ_STALE &&
              sq_invariant(&w));
        CHECK(sq_finish(&w, 1, run, SQ_EXITED, &event, &stats) == SQ_OK);
        CHECK(sq_create(&w, 0, 0, 0, &a, &event) == SQ_OK);
        w.task[0].revision = UINT64_MAX - 1;
        CHECK(sq_dispatch(&w, 0, &run, &event, &stats) == SQ_FULL &&
              sq_invariant(&w));
        w.task[0].revision = UINT64_MAX - 2;
        CHECK(sq_dispatch(&w, 0, &run, &event, &stats) == SQ_OK);
        CHECK(sq_finish(&w, 0, run, SQ_EXITED, &event, &stats) == SQ_OK &&
              w.task[0].revision == UINT64_MAX && sq_invariant(&w));
    }
    puts("SQ_PASS:capacity-transfer-owner-generation-retirement");
    return 0;
}
