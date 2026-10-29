#include "startup.h"
#include <stdio.h>
#include <string.h>
#define SSC(x)                                          \
    do                                                  \
    {                                                   \
        if (!(x))                                       \
        {                                               \
            fprintf(stderr, "SS_CHECK:%d\n", __LINE__); \
            return 1;                                   \
        }                                               \
    } while (0)

int main(void)
{
    struct ss_world w = {0};
    struct ss_ticket first = {9, 9}, next = {9, 9};
    struct ss_evidence evidence = {2, SS_VERIFIED, 0x21000 - 8};
    SSC(ss_begin(&w, 0, 1, 2, 0x8000, 0x20000, 0x21000, &first) == SS_OK &&
        first.generation == 1);
    struct ss_world old = w;
    SSC(ss_begin(&w, 1, 2, 6, 0x9000, 0x22000, 0x23000, &next) == SS_BUSY &&
        next.slot == 9 && memcmp(&old, &w, sizeof w) == 0);
    SSC(ss_timeout(&w, first) == SS_OK);
    SSC(ss_release(&w, first) == SS_NOT_QUIESCENT);
    SSC(ss_begin(&w, 0, 1, 2, 0x8000, 0x20000, 0x21000, &next) == SS_BUSY);
    SSC(ss_ack(&w, (struct ss_ticket){0, 0}, evidence) == SS_STALE);
    SSC(ss_ack(&w, first, evidence) == SS_LATE);
    SSC(ss_ack(&w, first, evidence) == SS_DUPLICATE);
    SSC(ss_quiescent(&w, first, 0) == SS_NOT_QUIESCENT);
    SSC(ss_release(&w, first) == SS_NOT_QUIESCENT);
    SSC(ss_quiescent(&w, first, 1) == SS_OK);
    SSC(ss_release(&w, first) == SS_OK);
    SSC(ss_begin(&w, 0, 1, 2, 0x8000, 0x20000, 0x21000, &next) == SS_OK &&
        next.generation == 2);
    SSC(ss_ack(&w, first, evidence) == SS_STALE);
    SSC(ss_ack(&w, next, (struct ss_evidence){6, 15, 0x21000 - 8}) ==
        SS_INVALID);
    SSC(ss_ack(&w, next, (struct ss_evidence){2, 7, 0x21000 - 8}) ==
        SS_INVALID);
    SSC(ss_ack(&w, next, (struct ss_evidence){2, 15, 0x21000}) == SS_INVALID);
    SSC(ss_ack(&w, next, evidence) == SS_OK);
    SSC(ss_timeout(&w, next) == SS_BUSY);
    SSC(ss_quiescent(&w, next, 1) == SS_OK);
    SSC(ss_release(&w, next) == SS_OK);
    w.parallel = 1;
    SSC(ss_begin(&w, 0, 1, 2, 0x8000, 0x20000, 0x22000, &first) == SS_OK);
    SSC(ss_begin(&w, 1, 2, 6, 0x9000, 0x9000, 0xa000, &next) == SS_INVALID);
    SSC(ss_begin(&w, 1, 2, 6, 0x9000, 0x8000, 0x9000, &next) == SS_BUSY);
    SSC(ss_begin(&w, 1, 2, 6, 0x20000, 0x23000, 0x24000, &next) == SS_BUSY);
    SSC(ss_begin(&w, 1, 2, 6, 0x9000, 0x21000, 0x23000, &next) == SS_BUSY);
    SSC(ss_begin(&w, 1, 2, 6, 0x8000, 0x23000, 0x24000, &next) == SS_BUSY);
    SSC(ss_begin(&w, 1, 2, 6, 0x9000, 0x23000, 0x24000, &next) == SS_OK);
    struct ss_ticket third;
    SSC(ss_begin(&w, 2, 3, 7, 0xa000, 0x24000, 0x25000, &third) == SS_OK);
    SSC(ss_ack(&w, third, (struct ss_evidence){7, 15, 0x25000 - 8}) == SS_OK);
    SSC(ss_ack(&w, first, (struct ss_evidence){2, 15, 0x22000 - 8}) == SS_OK);
    SSC(ss_ack(&w, next, (struct ss_evidence){6, 15, 0x24000 - 8}) == SS_OK);
    SSC(ss_quiescent(&w, first, 1) == SS_OK && ss_release(&w, first) == SS_OK);
    w.slot[0].generation = UINT32_MAX;
    SSC(ss_begin(&w, 0, 1, 2, 0x8000, 0x20000, 0x21000, &first) == SS_FULL);
    SSC(ss_quiescent(&w, next, 1) == SS_OK && ss_release(&w, next) == SS_OK);
    SSC(ss_quiescent(&w, third, 1) == SS_OK && ss_release(&w, third) == SS_OK);
    puts("SS_PASS:generation-timeout-retention-and-parallel-ownership");
    return 0;
}
