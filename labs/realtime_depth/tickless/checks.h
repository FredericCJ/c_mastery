#ifndef CMASTERY_TICKLESS_CHECKS_H
#define CMASTERY_TICKLESS_CHECKS_H
#include "tickless.h"
#define TA(x)                \
    do                       \
    {                        \
        if (!(x))            \
            return __LINE__; \
    } while (0)

static int tq_check_relation(void)
{
    enum tq_relation r;
    TA(tq_relation(UINT32_MAX - 3, 5, 32, 0, &r) == TQ_OK && r == TQ_FUTURE);
    TA((uint32_t)(5 - (UINT32_MAX - 3)) == 9);
    TA(tq_relation(0, 0, 32, 0, &r) == TQ_OK && r == TQ_EQUAL);
    TA(tq_relation(0, UINT32_C(0x80000000), 32, 0, &r) == TQ_OK &&
       r == TQ_AMBIGUOUS);
    TA(tq_relation(0, UINT32_C(0x80000001), 32, 0, &r) == TQ_OK &&
       r == TQ_PAST);
    TA(tq_relation(0, 0, 32, 1, &r) == TQ_OK && r == TQ_INFINITE);
    TA(tq_relation(250, 5, 8, 0, &r) == TQ_OK && r == TQ_FUTURE);
    return 0;
}

static int tq_check_conversion(void)
{
    uint64_t n = 99;
    TA(tq_ticks(11, 200, 1, &n) == TQ_OK && n == 3);
    TA(tq_ticks(0, 200, 1, &n) == TQ_OK && n == 0);
    TA(tq_ticks(UINT64_MAX, 1000, 1, &n) == TQ_OK && n == UINT64_MAX);
    n = 99;
    TA(tq_ticks(UINT64_MAX, 1000000, 1, &n) == TQ_FULL && n == 99);
    TA(tq_ticks(UINT64_MAX, 1, 1, &n) == TQ_OK &&
       n == UINT64_C(18446744073709552));
    TA(tq_ticks(11, 32768, 1, &n) == TQ_OK && n == 361);
    TA(tq_ticks(11, 1024, 3, &n) == TQ_OK && n == 4);
    TA(tq_not_before(4999, 11, 200, 1, &n) == TQ_OK && n == 4);
    TA(tq_not_before(4999, 0, 200, 1, &n) == TQ_OK && n == 1);
    TA(tq_not_before(0, 0, 200, 1, &n) == TQ_OK && n == 0);
    return 0;
}

static int tq_check_wrap(void)
{
    struct tq_timer t;
    struct tq_plan p;
    uint32_t when;
    enum cmx_result result;
    TA(tq_init(&t, UINT32_MAX - 3) == TQ_OK);
    TA(tq_wait(&t, 0, 9, 0) == TQ_OK && tq_wait(&t, 1, 9, 0) == TQ_OK &&
       tq_wait(&t, 2, 13, 0) == TQ_OK && tq_wait(&t, 3, 0, 1) == TQ_OK);
    TA(tq_plan(&t, UINT32_MAX - 3, &p) == TQ_OK && p.armed && p.at == 5);
    TA(tq_commit(&t, p, 0, 0) == TQ_OK && t.armed && t.programmed == 5);
    TA(tq_advance(&t, 5) == TQ_OK && t.events == 3);
    TA(tq_collect(&t, 0, &when, &result) == TQ_OK && when == 5 &&
       result == CMX_OK);
    TA(tq_collect(&t, 1, &when, &result) == TQ_OK && when == 5);
    TA(tq_plan(&t, 5, &p) == TQ_OK && p.at == 9);
    TA(tq_commit(&t, p, 9, 0) == TQ_RETRY && !t.armed);
    TA(tq_collect(&t, 2, &when, &result) == TQ_OK && when == 9 &&
       result == CMX_OK);
    TA(t.kernel.task[3].state == CMX_WAITING && tq_cancel(&t, 3) == TQ_OK);
    TA(tq_collect(&t, 3, &when, &result) == TQ_OK && result == CMX_CLOSED);
    TA(tq_plan(&t, 9, &p) == TQ_OK && !p.armed &&
       tq_commit(&t, p, 9, 0) == TQ_OK);
    return 0;
}

static int tq_check_reprogram(void)
{
    struct tq_timer t;
    struct tq_plan old, p;
    uint32_t when;
    enum cmx_result result;
    TA(tq_init(&t, 0) == TQ_OK && tq_wait(&t, 0, 20, 0) == TQ_OK &&
       tq_plan(&t, 0, &p) == TQ_OK);
    old = p;
    TA(tq_wait(&t, 1, 5, 0) == TQ_OK && tq_commit(&t, p, 0, 0) == TQ_RETRY &&
       !t.armed);
    TA(tq_plan(&t, 0, &p) == TQ_OK && p.at == 5 &&
       tq_commit(&t, p, 0, 0) == TQ_OK);
    TA(tq_commit(&t, old, 0, 0) == TQ_INVALID && t.programmed == 5);
    TA(tq_advance(&t, 5) == TQ_OK &&
       tq_collect(&t, 1, &when, &result) == TQ_OK);
    TA(tq_plan(&t, 5, &p) == TQ_OK && p.at == 20 &&
       tq_commit(&t, p, 6, 1) == TQ_RETRY && !t.armed);
    TA(tq_plan(&t, 6, &p) == TQ_OK && tq_commit(&t, p, 20, 0) == TQ_RETRY);
    TA(tq_collect(&t, 0, &when, &result) == TQ_OK && when == 20);
    return 0;
}

static int tq_check_resume(void)
{
    struct tq_timer t;
    struct tq_resume r = {100, 1, 3, 0};
    uint32_t when;
    enum cmx_result result;
    TA(tq_init(&t, 0) == TQ_OK && tq_wait(&t, 0, 10, 0) == TQ_OK);
    TA(tq_resume(&t, &r, 104) == TQ_OK && t.kernel.now == 1 &&
       r.remainder == 1);
    TA(tq_resume(&t, &r, 105) == TQ_OK && t.kernel.now == 1 &&
       r.remainder == 2);
    TA(tq_resume(&t, &r, 105) == TQ_OK && t.kernel.now == 1 &&
       r.remainder == 2);
    TA(tq_resume(&t, &r, 130) == TQ_OK && t.kernel.now == 10 &&
       r.remainder == 0);
    TA(tq_collect(&t, 0, &when, &result) == TQ_OK && when == 10);
    r = (struct tq_resume){UINT32_MAX - 4, 1000, 32768, 0};
    TA(tq_resume(&t, &r, 6) == TQ_OK && r.remainder == 11000 &&
       t.kernel.now == 10);
    return 0;
}

static int tq_check_guards(void)
{
    struct tq_timer t;
    struct tq_plan p = {0};
    uint64_t n = 71;
    enum tq_relation rel = TQ_PAST;
    TA(tq_relation(256, 0, 8, 0, &rel) == TQ_INVALID && rel == TQ_PAST);
    TA(tq_ticks(1, 0, 1, &n) == TQ_INVALID && n == 71);
    TA(tq_not_before(UINT64_MAX, 1, 1, 1, &n) == TQ_FULL && n == 71);
    TA(tq_init(&t, 0) == TQ_OK &&
       tq_wait(&t, 0, UINT32_C(0x80000000), 0) == TQ_INVALID);
    TA(tq_advance(&t, UINT32_C(0x80000000)) == TQ_INVALID && t.kernel.now == 0);
    TA(tq_wait(&t, 0, 0, 1) == TQ_OK);
    t.serial = UINT64_MAX;
    TA(tq_plan(&t, 0, &p) == TQ_FULL && !t.prepared && p.token == 0);
    t.kernel.sequence = UINT32_MAX;
    TA(tq_wait(&t, 1, 1, 0) == TQ_FULL && tq_cancel(&t, 0) == TQ_OK &&
       tq_abort(&t) == TQ_OK);
    return 0;
}

#undef TA
#endif
