#include "notifications.h"
#include <stdio.h>
#include <string.h>
#define CHECK(c)                                                \
    do                                                          \
    {                                                           \
        if (!(c))                                               \
        {                                                       \
            fprintf(stderr, "CDN_CHECK:%d:%s\n", __LINE__, #c); \
            return 1;                                           \
        }                                                       \
    } while (0)

static int setup(cmx_kernel *k, struct cd_notify *n, struct cdn_handle *h,
                 unsigned maximum)
{
    cmx_init(k, CMX_FIXED);
    if (cmx_task_add(k, 5, 0, 0x1000, 256) != 0)
    {
        return 0;
    }
    cdn_init(n, k);
    return cdn_attach(n, 0, maximum, h) == CMX_OK;
}

static int ledger(void)
{
    cmx_kernel k;
    struct cd_notify n;
    struct cdn_handle h;
    struct cdn_post post = {99, 98, 97};
    uint32_t value = 999;
    struct cdn_fault fault = {999, 999, 999};
    CHECK(setup(&k, &n, &h, 8));
    CHECK(cdn_post(h, 3, &post) == CMX_OK && !post.woke &&
          post.object_probes == 1);
    CHECK(cdn_post(h, 2, &post) == CMX_OK);
    for (unsigned i = 0; i < 4; i++)
    {
        CHECK(cdn_wait(h, 1, cmx_forever()) == CMX_OK);
        CHECK(cdn_result(h, &value) == CMX_OK && value == 1 &&
              cdn_invariant(&n));
    }
    CHECK(n.accepted == 5 && n.consumed == 4 && k.task[0].notifications == 1);
    CHECK(cdn_post(h, 6, &post) == CMX_OK && k.task[0].notifications == 7);
    post = (struct cdn_post){99, 98, 97};
    CHECK(cdn_post(h, 2, &post) == CMX_FULL && k.task[0].notifications == 7 &&
          n.accepted == 11);
    CHECK(post.woke == 99 && post.defer_schedule == 98 &&
          post.object_probes == 97);
    CHECK(cdn_fault(h, &fault) == CMX_OK && fault.sequence == 1 &&
          fault.count == 2 && fault.reason == CMX_FULL);
    CHECK(n.rejected == 2 && cdn_post(h, 1, &post) == CDN_FAULT_PENDING);
    CHECK(cdn_ack_fault(h, 0) == CDN_STALE &&
          cdn_ack_fault(h, fault.sequence) == CMX_OK);
    CHECK(cdn_post(h, 1, &post) == CMX_OK && k.task[0].notifications == 8);
    CHECK(cdn_close(h) == CMX_OK && n.cancelled == 8 && n.consumed == 4 &&
          n.accepted == 12);
    CHECK(cdn_invariant(&n) && cdn_detach(h) == CMX_OK);
    CHECK(setup(&k, &n, &h, UINT32_MAX));
    CHECK(cdn_post(h, UINT32_MAX, &post) == CMX_OK);
    CHECK(cdn_post(h, 1, &post) == CMX_FULL &&
          k.task[0].notifications == UINT32_MAX);
    CHECK(cdn_fault(h, &fault) == CMX_OK && fault.count == 1);
    CHECK(cdn_ack_fault(h, fault.sequence) == CMX_OK);
    CHECK(cdn_wait(h, UINT32_MAX, cmx_forever()) == CMX_OK);
    CHECK(cdn_result(h, &value) == CMX_OK && value == UINT32_MAX &&
          cdn_invariant(&n));
    CHECK(setup(&k, &n, &h, 8));
    n.accepted = n.consumed =
        UINT64_MAX; /* Reachable empty, exhausted ledger. */
    post = (struct cdn_post){99, 98, 97};
    CHECK(cdn_invariant(&n) && cdn_post(h, 1, &post) == CDN_RETIRED &&
          post.woke == 99);
    CHECK(cdn_close(h) == CMX_OK && cdn_detach(h) == CMX_OK);
    CHECK(setup(&k, &n, &h, 8));
    n.rejected = UINT64_MAX; /* Previous refused attempts were acknowledged. */
    CHECK(cdn_post(h, 1, &post) == CDN_RETIRED && n.accepted == 0 &&
          cdn_invariant(&n));
    CHECK(cdn_close(h) == CMX_OK && cdn_detach(h) == CMX_OK);
    CHECK(setup(&k, &n, &h, 8));
    n.fault_sequence = UINT64_MAX;
    CHECK(cdn_post(h, 1, &post) == CDN_RETIRED && cdn_close(h) == CMX_OK &&
          cdn_detach(h) == CMX_OK);
    puts(
        "CDN_LEDGER:post3-post2-take4-remain1;7plus2-refuses2;fault-ack-backpressure;full-width-all");
    return 0;
}

static int handoff(void)
{
    cmx_kernel k;
    struct cd_notify n;
    struct cdn_handle h;
    struct cdn_post post;
    uint32_t out = 999;
    CHECK(setup(&k, &n, &h, 8));
    CHECK(cdn_wait(h, 1, cmx_forever()) == CMX_BLOCKED);
    CHECK(cdn_post(h, 3, &post) == CMX_OK && post.woke && post.defer_schedule);
    CHECK(n.claimed == 1 && k.task[0].notifications == 2 && cdn_invariant(&n));
    CHECK(cdn_cancel(h) == CMX_OK); /* Completion already won. */
    CHECK(cdn_result(h, &out) == CMX_OK && out == 1);
    CHECK(cdn_wait(h, 1, cmx_forever()) == CMX_OK &&
          cdn_result(h, &out) == CMX_OK && out == 1);
    CHECK(cdn_wait(h, 1, cmx_forever()) == CMX_OK &&
          cdn_result(h, &out) == CMX_OK && out == 1);
    CHECK(cdn_wait(h, 3, cmx_forever()) == CMX_BLOCKED);
    CHECK(cdn_post(h, 5, &post) == CMX_OK && n.claimed == 1 &&
          k.task[0].notifications == 4);
    CHECK(cdn_result(h, &out) == CMX_OK && out == 3 &&
          k.task[0].notifications == 2);
    CHECK(cdn_wait(h, UINT32_MAX, cmx_forever()) == CMX_OK);
    CHECK(cdn_post(h, 2, &post) ==
          CMX_OK); /* Included in the result snapshot. */
    CHECK(cdn_result(h, &out) == CMX_OK && out == 4 && cdn_invariant(&n));
    CHECK(cdn_result(h, &out) == CMX_INVALID);
    puts(
        "CDN_HANDOFF:post3-reserves1-retains2;wait-up-to3;consume-all-result-snapshot;cancel-after-success");
    return 0;
}

static int lifetime(void)
{
    cmx_kernel k;
    struct cd_notify n;
    struct cdn_handle old, fresh;
    struct cdn_post post = {99, 98, 97};
    uint32_t out = 999;
    CHECK(setup(&k, &n, &old, 8));
    CHECK(cdn_post(old, 0, &post) == CMX_INVALID && post.woke == 99);
    CHECK(cdn_wait(old, 1, cmx_at(5)) == CMX_BLOCKED);
    CHECK(cdn_wait(old, 1, cmx_forever()) == CMX_INVALID);
    cmx_tick(&k, 5);
    CHECK(cdn_post(old, 3, &post) == CMX_OK && !post.woke && n.claimed == 0);
    CHECK(cdn_result(old, &out) == CMX_TIMEOUT && out == 999 &&
          k.task[0].notifications == 3);
    CHECK(cdn_wait(old, UINT32_MAX, cmx_forever()) == CMX_OK &&
          cdn_result(old, &out) == CMX_OK && out == 3);
    CHECK(cdn_wait(old, 1, cmx_forever()) == CMX_BLOCKED);
    CHECK(cdn_cancel(old) == CMX_CLOSED);
    CHECK(cdn_post(old, 2, &post) == CMX_OK && !post.woke);
    out = 999;
    CHECK(cdn_result(old, &out) == CMX_CLOSED && out == 999);
    CHECK(cdn_close(old) == CMX_OK && n.cancelled == 2 && cdn_invariant(&n));
    CHECK(cdn_detach(old) == CMX_OK);
    CHECK(cdn_attach(&n, 0, 8, &fresh) ==
          CMX_INVALID); /* Same incarnation cannot recycle control. */
    CHECK(cmx_exit(&k, 0, 0) == CMX_OK);
    CHECK(cdn_post(old, 1, &post) == CDN_STALE);
    CHECK(cmx_reap(&k, 0) == CMX_OK &&
          cmx_task_add(&k, 5, 0, 0x1000, 256) == 0);
    CHECK(cdn_attach(&n, 0, 8, &fresh) == CMX_OK &&
          fresh.generation != old.generation);
    CHECK(cdn_post(old, 1, &post) == CDN_STALE && k.task[0].notifications == 0);
    CHECK(cdn_post(fresh, 1, &post) == CMX_OK && cdn_close(fresh) == CMX_OK &&
          cdn_detach(fresh) == CMX_OK);
    k.task[0].generation =
        UINT32_MAX; /* Valid exhausted-incarnation fixture. */
    CHECK(cmx_exit(&k, 0, 0) == CMX_OK && cmx_reap(&k, 0) == CMX_FULL);
    puts(
        "CDN_LIFETIME:timeout-cancel-before-post;completed-credit-retained;exit-reap-stale-generation;no-wrap");
    return 0;
}

static int histories(void)
{
    unsigned histories = 0, transitions = 0;
    for (unsigned code = 0; code < 1024; code++)
    {
        cmx_kernel k;
        struct cd_notify n;
        struct cdn_handle h;
        struct cdn_post post;
        struct cdn_fault fault;
        unsigned remaining = 0, accepted = 0, consumed = 0, rejected = 0,
                 digits = code;
        CHECK(setup(&k, &n, &h, 8));
        for (unsigned step = 0; step < 5; step++)
        {
            unsigned action = digits % 4;
            digits /= 4;
            if (action < 2)
            {
                unsigned count = action == 0 ? 1 : 3;
                if (remaining + count <= 8)
                {
                    CHECK(cdn_post(h, count, &post) == CMX_OK);
                    remaining += count;
                    accepted += count;
                }
                else
                {
                    CHECK(cdn_post(h, count, &post) == CMX_FULL);
                    rejected += count;
                    CHECK(cdn_fault(h, &fault) == CMX_OK &&
                          fault.count == count);
                    CHECK(cdn_ack_fault(h, fault.sequence) == CMX_OK);
                }
            }
            else
            {
                unsigned take = remaining ? (action == 2 ? 1 : remaining) : 0;
                uint32_t out = 999;
                CHECK(cdn_wait(h, action == 2 ? 1 : UINT32_MAX, cmx_at(0)) ==
                      (take ? CMX_OK : CMX_TIMEOUT));
                CHECK(cdn_result(h, &out) == (take ? CMX_OK : CMX_TIMEOUT));
                CHECK(out == (take ? take : 999));
                remaining -= take;
                consumed += take;
            }
            CHECK(k.task[0].notifications == remaining &&
                  n.accepted == accepted && n.consumed == consumed &&
                  n.rejected == rejected && cdn_invariant(&n));
            transitions++;
        }
        CHECK(cdn_close(h) == CMX_OK && n.cancelled == remaining &&
              cdn_invariant(&n));
        histories++;
    }
    printf(
        "CDN_HISTORIES:histories=%u;transitions=%u;independent-integer-ledger;explicit-overflow-faults\n",
        histories, transitions);
    return 0;
}

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        return 2;
    }
    if (!strcmp(argv[1], "ledger"))
    {
        return ledger();
    }
    if (!strcmp(argv[1], "handoff"))
    {
        return handoff();
    }
    if (!strcmp(argv[1], "lifetime"))
    {
        return lifetime();
    }
    if (!strcmp(argv[1], "histories"))
    {
        return histories();
    }
    return 2;
}
