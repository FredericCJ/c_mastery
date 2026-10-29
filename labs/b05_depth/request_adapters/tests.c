#include "request.h"
#include <stdio.h>
#include <string.h>
#define CHECK(x, s)                              \
    do                                           \
    {                                            \
        if (!(x))                                \
        {                                        \
            fprintf(stderr, "RQ_CHECK:%s\n", s); \
            return 1;                            \
        }                                        \
    } while (0)
/* Independent allowed-transition table: no call into the implementation derives
 * the expected phase. -1 is a rejected transition. */
static const int next[7][7] = {
    {RQ_INFLIGHT, -1, -1, -1, -1, -1, -1},
    {-1, RQ_CANCELLED, RQ_UNKNOWN, RQ_DONE, RQ_RESETTING, -1, -1},
    {-1, RQ_CANCELLED, RQ_UNKNOWN, RQ_DONE, RQ_RESETTING, -1, -1},
    {-1, -1, -1, RQ_DONE, RQ_RESETTING, -1, -1},
    {-1, -1, -1, -1, -1, RQ_DONE, -1},
    {-1, -1, -1, -1, -1, -1, RQ_FREE},
    {-1, -1, -1, -1, -1, -1, -1}};

int main(void)
{
    unsigned observations = 0;
    for (unsigned phase = 0; phase < 7; phase++)
    {
        for (unsigned op = 0; op < 7; op++)
        {
            for (unsigned old = 0; old < 2; old++)
            {
                for (unsigned edge = 0; edge < 2; edge++)
                {
                    struct b5_rq r = {edge ? UINT32_MAX : 19, phase, 71,
                                      phase == RQ_DONE ? 1u : 0u},
                                 before = r;
                    int status = rq_apply(&r, op, r.generation - old, 37),
                        expect = next[phase][op];
                    if (old || expect < 0)
                    {
                        CHECK(status != RQ_OK, "reject transition");
                        CHECK(!memcmp(&r, &before, sizeof r),
                              "rejection unchanged");
                    }
                    else
                    {
                        CHECK(status == RQ_OK, "admit transition");
                        if (op == RQ_REAP && edge)
                        {
                            expect = RQ_RETIRED;
                        }
                        CHECK(r.phase == (unsigned)expect, "transition table");
                        CHECK(r.terminals == (op == RQ_SUBMIT
                                                  ? 0
                                                  : before.terminals +
                                                        (op == RQ_COMPLETE ||
                                                         op == RQ_RESET_ACK)),
                              "terminal count");
                        CHECK(r.result == (op == RQ_RESET_ACK || op == RQ_SUBMIT
                                               ? RQ_UNCERTAIN
                                           : op == RQ_COMPLETE ? 37
                                                               : 71),
                              "result contract");
                        CHECK(r.generation ==
                                  before.generation + (op == RQ_REAP && !edge),
                              "generation retirement");
                    }
                    CHECK(rq_owned(&r) == (r.phase == RQ_INFLIGHT ||
                                           r.phase == RQ_CANCELLED ||
                                           r.phase == RQ_UNKNOWN ||
                                           r.phase == RQ_RESETTING),
                          "ownership states");
                    observations++;
                }
            }
        }
    }
    struct b5_rq r = {0};
    CHECK(rq_apply(&r, RQ_SUBMIT, 0, 0) == RQ_OK, "submit");
    CHECK(rq_apply(&r, RQ_CANCEL, 0, 0) == RQ_OK && rq_owned(&r),
          "cancel retains ownership");
    CHECK(rq_apply(&r, RQ_REAP, 0, 0) == RQ_BUSY, "cancel cannot reap");
    CHECK(rq_apply(&r, RQ_TIMEOUT, 0, 0) == RQ_OK && rq_owned(&r),
          "timeout is uncertainty");
    CHECK(rq_apply(&r, RQ_RESET_BEGIN, 0, 0) == RQ_OK && rq_owned(&r),
          "reset intent retains ownership");
    CHECK(rq_apply(&r, RQ_COMPLETE, 0, 0) == RQ_BUSY && r.terminals == 0,
          "completion during reset ignored");
    CHECK(rq_apply(&r, RQ_RESET_ACK, 0, 0) == RQ_OK && !rq_owned(&r) &&
              r.result == RQ_UNCERTAIN,
          "ack releases ownership not outcome");
    CHECK(rq_apply(&r, RQ_REAP, 0, 0) == RQ_OK, "reap acknowledged reset");
    CHECK(rq_apply(&r, RQ_COMPLETE, 0, 0) == RQ_STALE, "old completion stale");
    printf("RQ_OK:%u transitions; reset receipt is not an I/O outcome\n",
           observations);
    return 0;
}
