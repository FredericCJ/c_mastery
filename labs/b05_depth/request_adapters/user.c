#include "abi.h"
#include <stdatomic.h>
static unsigned char first[4096], second[4096];
static atomic_uint finished;

static void say(const char *s)
{
    unsigned n = 0;
    while (s[n])
        n++;
    cmk_call(CMK_WRITE, (uintptr_t)s, n, 0);
}

static void fail(const char *s)
{
    say("RQA_CHECK:");
    say(s);
    say("\n");
    cmk_call(CMK_EXIT, 1, 0, 0);
    for (;;)
    {
    }
}

#define CHECK(x, s)  \
    do               \
    {                \
        if (!(x))    \
            fail(s); \
    } while (0)

static int64_t submit(uint64_t h, unsigned type, unsigned page, void *bytes)
{
    struct cmk2_block_request r = {type, page, (uintptr_t)bytes};
    return cmk_call(CMK2_BLOCK_SUBMIT, h, (uintptr_t)&r, 0);
}

static void used(unsigned n)
{
    for (unsigned i = 0; i < 10000; i++)
    {
        if (cmk_call(RQA_USED, 0, 0, 0) == n)
            return;
        cmk_call(CMK_YIELD, 0, 0, 0);
    }
    fail("actual device used entries missing");
}

static void waiter(uintptr_t ticket)
{
    CHECK(!cmk_call(CMK2_BLOCK_WAIT, ticket, 0, 0), "waiter own result");
    atomic_fetch_add_explicit(&finished, 1, memory_order_relaxed);
    cmk2_thread_exit(0);
}

void _start(void)
{
    uint64_t h = CMK_DISK_HANDLE;
#ifdef RQA_ERROR
    for (unsigned j = 0; j < 4096; j++)
        first[j] = (unsigned char)(j * 3 + 7);
    CHECK(cmk2_block((void *)(uintptr_t)h, 1, 12, first) == -1,
          "device error propagated");
    CHECK(!cmk_call(RQA_PENDING, 0, 0, 0),
          "error completion releases ownership");
    CHECK(!cmk2_block((void *)(uintptr_t)h, 0, 12, second),
          "read failed write target");
    for (unsigned j = 0; j < 4096; j++)
        CHECK(second[j] == 0, "injected write had no effect");
    CHECK(!cmk2_block((void *)(uintptr_t)h, 1, 12, first) &&
              !cmk2_block((void *)(uintptr_t)h, 4, 0, 0) &&
              !cmk2_block((void *)(uintptr_t)h, 0, 12, second),
          "error retry queue reusable");
    for (unsigned j = 0; j < 4096; j++)
        CHECK(second[j] == first[j], "successful retry exact bytes");
    say("PASS rqa-device-error-retry\n");
    cmk_call(CMK_EXIT, 0, 0, 0);
    for (;;)
    {
    }
#endif
    if (cmk_call(CMK_PID, 0, 0, 0) != 0)
    {
        for (unsigned j = 0; j < 4096; j++)
            first[j] = (unsigned char)(j * 5 + 57);
        CHECK(!cmk_call(RQA_HOLD, 1, 0, 0), "child holds observer");
        CHECK(submit(h, 1, 11, first) >= 0, "child request admitted");
        for (unsigned j = 0; j < 4096; j++)
            first[j] = 99;
        cmk_call(CMK_EXIT, 42, 0, 0);
        for (;;)
        {
        }
    }
    for (unsigned i = 0; i < 4096; i++)
    {
        first[i] = (unsigned char)(3 * i + 17);
        second[i] = (unsigned char)(5 * i + 31);
    }
    CHECK(!cmk_call(RQA_HOLD, 1, 0, 0), "hold observer");
    int64_t a = submit(h, 1, 8, first), b = submit(h, 1, 9, second);
    CHECK(a >= 0 && b >= 0 && a != b, "two outstanding tickets");
    CHECK(submit(h, 1, 10, first) == CMK2_EAGAIN, "pool exhaustion");
    used(2);
    CHECK(cmk_call(RQA_PENDING, 0, 0, 0) == 2,
          "unobserved completions remain owned");
    CHECK(!cmk_call(RQA_CANCEL, (uint64_t)a, 0, 0) &&
              cmk_call(RQA_QUERY, (uint64_t)a, 0, 0) == RQ_CANCELLED,
          "cancel intent");
    CHECK(submit(h, 1, 10, first) == CMK2_EAGAIN,
          "cancel cannot recycle descriptors");
    CHECK(!cmk_call(RQA_HOLD, 0, 0, 0), "release observation");
    CHECK(cmk_call(RQA_PENDING, 0, 0, 0) == 0 &&
              cmk_call(RQA_TERMINALS, 0, 0, 0) == 2,
          "consume all coalesced entries");
    CHECK(!cmk_call(CMK2_BLOCK_WAIT, (uint64_t)a, 0, 0) &&
              !cmk_call(CMK2_BLOCK_WAIT, (uint64_t)b, 0, 0),
          "completion wins cancellation");
    CHECK(cmk_call(RQA_CANCEL, (uint64_t)a, 0, 0) == RQ_STALE,
          "stale ticket rejected");
    CHECK(!cmk2_block((void *)(uintptr_t)h, 4, 0, 0), "flush observed writes");
    for (unsigned i = 0; i < 4096; i++)
    {
        first[i] = 0;
        second[i] = 0;
    }
    CHECK(!cmk2_block((void *)(uintptr_t)h, 0, 8, first) &&
              !cmk2_block((void *)(uintptr_t)h, 0, 9, second),
          "read persisted pages");
    for (unsigned i = 0; i < 4096; i++)
        CHECK(first[i] == (unsigned char)(3 * i + 17) &&
                  second[i] == (unsigned char)(5 * i + 31),
              "distinct descriptor payloads");
    CHECK(!cmk_call(RQA_HOLD, 1, 0, 0), "hold waiters observer");
    a = submit(h, 0, 8, first);
    b = submit(h, 0, 9, second);
    CHECK(a >= 0 && b >= 0, "waiter requests");
    used(2);
    unsigned char *stack_a = cmk_alloc(4), *stack_b = cmk_alloc(4);
    CHECK(stack_a && stack_b, "waiter stack allocation");
    int64_t wa = cmk2_thread_create(waiter, stack_a + 16384, (uintptr_t)a),
            wb = cmk2_thread_create(waiter, stack_b + 16384, (uintptr_t)b);
    CHECK(wa > 0 && wb > 0, "waiter threads");
    for (unsigned spins = 0; cmk_call(RQA_WAITERS, 0, 0, 0) != 2; spins++)
    {
        CHECK(spins < 10000, "both matching waiters blocked");
        cmk_call(CMK_YIELD, 0, 0, 0);
    }
    CHECK(!cmk_call(RQA_HOLD, 0, 0, 0) && !cmk2_thread_join((uint32_t)wa) &&
              !cmk2_thread_join((uint32_t)wb),
          "coalesced wake and join");
    CHECK(atomic_load_explicit(&finished, memory_order_relaxed) == 2 &&
              cmk_call(RQA_PENDING, 0, 0, 0) == 0,
          "each matching waiter woke once");
    CHECK(cmk_call(CMK2_BLOCK_WAIT, (uint64_t)a, 0, 0) == CMK2_EINVAL &&
              cmk_call(CMK2_BLOCK_WAIT, (uint64_t)b, 0, 0) == CMK2_EINVAL,
          "waiters consumed tickets once");
    say("PASS rqa-two-blocked-waiters\n");
    say("PASS rqa-two-outstanding-cancel-coalesced\n");
    CHECK(!cmk_call(RQA_HOLD, 1, 0, 0), "hold second observer");
    a = submit(h, 1, 10, first);
    CHECK(a >= 0, "unknown request admission");
    CHECK(!cmk_call(RQA_TIMEOUT, (uint64_t)a, 0, 0) &&
              cmk_call(CMK2_BLOCK_WAIT, (uint64_t)a, 0, 0) == RQ_UNCERTAIN,
          "timeout uncertain retains ticket");
    CHECK(!cmk_call(RQA_RESET_BEGIN, 0, 0, 0), "begin device reset");
    CHECK(cmk_call(RQA_QUERY, (uint64_t)a, 0, 0) == RQ_RESETTING &&
              cmk_call(RQA_PENDING, 0, 0, 0) == 1,
          "reset intent retains request");
    CHECK(cmk_call(RQA_RESET_ACK, 1, 0, 0) == RQ_BUSY &&
              cmk_call(RQA_QUERY, (uint64_t)a, 0, 0) == RQ_RESETTING,
          "withheld acknowledgement retains request");
    CHECK(!cmk_call(RQA_RESET_STATUS, 0, 0, 0), "actual reset status zero");
    CHECK(!cmk_call(RQA_RESET_ACK, 0, 0, 0) &&
              cmk_call(RQA_PENDING, 0, 0, 0) == 0,
          "observed reset releases DMA ownership");
    CHECK(cmk_call(CMK2_BLOCK_WAIT, (uint64_t)a, 0, 0) == RQ_UNCERTAIN,
          "reset does not determine write outcome");
    CHECK(cmk_call(RQA_QUERY, (uint64_t)a, 0, 0) == RQ_STALE,
          "post-reset generation changed");
    CHECK(!cmk2_block((void *)(uintptr_t)h, 0, 8, first),
          "renegotiated queue works");
    for (unsigned i = 0; i < 4096; i++)
        CHECK(first[i] == (unsigned char)(3 * i + 17),
              "unrelated committed data survived reset");
    say("PASS rqa-reset-ack-unknown-reuse\n");
    for (unsigned cycle = 0; cycle < 131074; cycle++)
    {
        a = submit(h, 0, 8, first);
        CHECK(a >= 0, "wrap admission");
        CHECK(!cmk_call(CMK2_BLOCK_WAIT, (uint64_t)a, 0, 0), "wrap completion");
        CHECK(first[0] == 17 && first[4095] == (unsigned char)(3 * 4095 + 17),
              "wrap payload");
    }
    CHECK(cmk_call(RQA_INDICES, 0, 0, 0) == ((3u << 16) | 3u),
          "actual 16-bit available used wrap");
    say("PASS rqa-131074-transfers-two-wraps\n");
    const char name[] = "init";
    int64_t child = cmk_call(CMK2_SPAWN, (uintptr_t)name, 4, 1);
    CHECK(child > 0, "owner exit child");
    for (unsigned spins = 0; cmk_call(RQA_DEAD_PENDING, 0, 0, 0) != 1; spins++)
    {
        CHECK(spins < 10000, "dead child enrollment");
        cmk_call(CMK_YIELD, 0, 0, 0);
    }
    CHECK(!cmk_call(RQA_HOLD, 0, 0, 0), "owner exit completion observation");
    CHECK(cmk_call(CMK2_WAIT_PROCESS, (uint64_t)child, 0, 0) == 42 &&
              cmk_call(RQA_PENDING, 0, 0, 0) == 0,
          "owner exit quiescence and reaping");
    CHECK(!cmk2_block((void *)(uintptr_t)h, 0, 11, second),
          "read departed owner's write");
    for (unsigned j = 0; j < 4096; j++)
        CHECK(second[j] == (unsigned char)(j * 5 + 57),
              "write copied before owner mutation and death");
    say("PASS rqa-owner-exit-keeps-request\n");
    cmk_call(CMK_EXIT, 0, 0, 0);
    for (;;)
    {
    }
}
