#include "cl_cmx.h"
#ifdef CC_TARGET
#define CHECK(x)               \
    do                         \
    {                          \
        if (!(x))              \
            cc_target_stop(1); \
    } while (0)
#define puts(x) cc_target_puts(x)
#else
#include <stdio.h>
#include <string.h>
#define CHECK(x)                                               \
    do                                                         \
    {                                                          \
        if (!(x))                                              \
        {                                                      \
            fprintf(stderr, "CC_CHECK:%d:%s\n", __LINE__, #x); \
            return 1;                                          \
        }                                                      \
    } while (0)
#endif
static int cq_setup(struct cc_queue *q, cmx_kernel *k, struct cc_handle h[5])
{
    cmx_init(k, CMX_FIXED);
    for (unsigned i = 0; i < 5; i++)
        if (cmx_task_add(k, 5, 0, 0x1000 + 256 * i, 256) != (int)i)
            return 0;
    if (cc_init(q, k) != CL_OK || q->bytes.slots != q->storage ||
        q->life.generation != 1)
        return 0;
    for (unsigned i = 0; i < 5; i++)
        if (cc_register(q, i, &h[i]) != CL_OK)
            return 0;
    return 1;
}

static int cq_finish(struct cc_queue *q, struct cc_handle h[5])
{
    if (cc_close(q, 0) != CL_OK)
        return 0;
    for (unsigned i = 0; i < 5; i++)
        if (cc_release(&h[i]) != CL_OK)
            return 0;
    return cc_destroy(q) == CL_OK;
}

static int cq_queue(void)
{
    cmx_kernel k;
    struct cc_queue q;
    struct cc_handle h[5];
    struct cc_ticket a, b, c, r;
    struct cl_message out = {.sequence = 99};
    unsigned char bytes[4] = {3, 0, 9, 7};
    CHECK(cq_setup(&q, &k, h));
    CHECK(cc_send(h[0], 0, 1, bytes, 4, cmx_forever(), &a) == CL_OK &&
          cc_result(a, 0) == CL_OK);
    bytes[0] = 88;
    CHECK(cc_send(h[1], 1, 1, 0, 0, cmx_forever(), &b) == CL_OK &&
          cc_result(b, 0) == CL_OK);
    unsigned char later[4] = {5, 6, 7, 8};
    CHECK(cc_send(h[2], 2, 1, later, 4, cmx_forever(), &c) == CC_BLOCKED);
    CHECK(k.task[2].state == CMX_WAITING && q.bytes.accepted == 2 &&
          q.bytes.count == 2);
    CHECK(cc_receive(h[3], 3, cmx_forever(), &r) == CL_RANGE &&
          out.sequence == 99 && q.bytes.count == 2 && q.bytes.accepted == 2);
    CHECK(cc_receive(h[3], 4, cmx_forever(), &r) == CL_OK &&
          cc_result(r, &out) == CL_OK && out.bytes[0] == 3 &&
          out.bytes[3] == 7);
    CHECK(cc_result(c, 0) == CL_OK && q.bytes.accepted == 3 &&
          q.bytes.count == 2);
    later[0] = 99;
    CHECK(cc_close(&q, 0) == CL_OK &&
          cc_send(h[0], 0, 2, bytes, 4, cmx_forever(), &a) == CL_CLOSED);
    CHECK(cc_receive(h[4], 0, cmx_forever(), &r) == CL_OK &&
          cc_result(r, &out) == CL_OK && out.length == 0);
    CHECK(cc_receive(h[3], 4, cmx_forever(), &r) == CL_OK &&
          cc_result(r, &out) == CL_OK && out.bytes[0] == 5);
    CHECK(cc_receive(h[4], 4, cmx_forever(), &r) == CL_CLOSED &&
          q.bytes.accepted == q.bytes.delivered && cq_finish(&q, h));
    puts(
        "CC_QUEUE:actual-byte-core;two-slots;CMX-blocked-third;copy-at-acceptance;short-output-preserved;zero-length;close-drain");
    return 0;
}

static int cq_terminal(void)
{
    for (unsigned cause = 0; cause < 3; cause++)
    {
        cmx_kernel k;
        struct cc_queue q;
        struct cc_handle h[5];
        struct cc_ticket a, b, c, r, old;
        struct cl_message out;
        unsigned char bytes[4] = {1, 2, 3, 4};
        CHECK(cq_setup(&q, &k, h));
        CHECK(cc_send(h[0], 0, 1, bytes, 4, cmx_forever(), &a) == CL_OK &&
              cc_result(a, 0) == CL_OK);
        CHECK(cc_send(h[1], 1, 1, bytes, 4, cmx_forever(), &b) == CL_OK &&
              cc_result(b, 0) == CL_OK);
        CHECK(cc_send(h[2], 2, 1, bytes, 4, cmx_at(5), &c) == CC_BLOCKED);
        if (cause == 0)
            cmx_tick(&k, 5);
        else if (cause == 1)
            CHECK(cc_cancel(c) == CC_CANCELLED);
        else
            CHECK(cc_close(&q, 1) == CL_OK && q.bytes.discarded == 2);
        if (cause < 2)
            CHECK(cc_receive(h[3], 4, cmx_forever(), &r) == CL_OK &&
                  cc_result(r, &out) == CL_OK);
        CHECK(cc_result(c, 0) == (cause == 0   ? CC_TIMEOUT
                                  : cause == 1 ? CC_CANCELLED
                                               : CL_CLOSED));
        CHECK(q.bytes.accepted == 2 && q.nodes[2].borrowed == 0 &&
              k.task[2].state != CMX_WAITING);
        old = c;
        if (cause < 2)
        {
            CHECK(cc_send(h[2], 2, 2, bytes, 4, cmx_forever(), &c) == CL_OK);
            CHECK(cc_cancel(old) == CL_STALE && cc_result(old, 0) == CL_STALE &&
                  cc_result(c, 0) == CL_OK);
            CHECK(cc_close(&q, 1) == CL_OK);
        }
        CHECK(cq_finish(&q, h));
    }
    cmx_kernel k;
    struct cc_queue q;
    struct cc_handle h[5];
    struct cc_ticket r;
    struct cl_message out = {.sequence = 99};
    CHECK(cq_setup(&q, &k, h) &&
          cc_receive(h[4], 4, cmx_forever(), &r) == CC_BLOCKED);
    CHECK(cc_close(&q, 0) == CL_OK && cc_result(r, &out) == CL_CLOSED &&
          out.sequence == 99 && cq_finish(&q, h));
    puts(
        "CC_TERMINAL:timeout-before-capacity;cancel-before-copy;abort-count2;stale-operation;empty-close;no-output-on-failure");
    return 0;
}

static int cq_condition(void)
{
    for (unsigned signaled = 0; signaled < 2; signaled++)
    {
        cmx_kernel k;
        struct cc_queue q;
        struct cc_handle h[5];
        struct cc_ticket t;
        cmx_mutex m;
        cmx_cond c = {0};
        CHECK(cq_setup(&q, &k, h) && cmx_mutex_init(&k, &m, 0, 0) == CMX_OK &&
              cmx_mutex_lock(&k, &m, 0, cmx_forever()) == CMX_OK);
        CHECK(cc_condition(h[0], &c, &m, cmx_at(5), &t) == CC_BLOCKED &&
              m.owner == CMX_NONE);
        CHECK(cmx_mutex_lock(&k, &m, 1, cmx_forever()) == CMX_OK);
        if (signaled)
            CHECK(cmx_cond_signal(&k, &c, 0) == CMX_OK);
        else
            cmx_tick(&k, 5);
        CHECK(k.task[0].state == CMX_WAITING && k.task[0].wait == CMX_W_MUTEX &&
              cc_progress(&q) == CL_OK);
        CHECK(cc_close(&q, 0) == CL_OK && cc_cancel(t) == CC_BLOCKED &&
              cc_result(t, 0) == CC_BLOCKED);
        CHECK(cc_release(&h[0]) == CL_BUSY && cc_destroy(&q) == CL_BUSY);
        CHECK(cmx_mutex_unlock(&k, &m, 1) == CMX_OK && m.owner == 0);
        CHECK(cc_result(t, 0) == (signaled ? CL_OK : CC_TIMEOUT) &&
              cc_release(&h[0]) == CL_BUSY);
        CHECK(cc_condition_unlock(h[0]) == CL_OK &&
              cmx_mutex_close(&k, &m) == CMX_OK && cq_finish(&q, h));
    }
    puts(
        "CC_CONDITION:actual-condition-reacquire;selected-timeout-or-signal-survives-close;lease-held-through-mutex-unlock");
    return 0;
}

static int cq_lifetime(void)
{
    cmx_kernel k;
    struct cc_queue q;
    struct cc_handle h[5], old;
    struct cc_port port, stale;
    struct cc_ticket t;
    unsigned char bytes[4] = {7, 0, 0, 0};
    struct cl_message out;
    CHECK(cq_setup(&q, &k, h) && cc_port_acquire(&q, &port) == CL_OK);
    stale = port;
    old = h[0];
    CHECK(cc_receive(h[4], 4, cmx_forever(), &t) == CC_BLOCKED &&
          cc_port_send(port, 9, 1, bytes, 4) == CL_OK);
    CHECK(k.task[4].state == CMX_READY && cc_result(t, &out) == CL_OK &&
          out.producer == 9 && out.bytes[0] == 7);
    CHECK(cc_close(&q, 0) == CL_OK);
    for (unsigned i = 0; i < 5; i++)
        CHECK(cc_release(&h[i]) == CL_OK);
    CHECK(cc_destroy(&q) == CL_BUSY &&
          cc_port_send(port, 9, 2, bytes, 4) == CL_CLOSED);
    /* Hosted serial case supplies quiescence directly; target/native port tests
     * must establish actual interrupt-disable/join before this release. */
    CHECK(cc_port_release_after_quiescence(&port) == CL_OK &&
          cc_destroy(&q) == CL_OK && cc_restart(&q) == CL_OK);
    CHECK(cc_send(old, 0, 1, bytes, 4, cmx_forever(), &t) == CL_STALE &&
          cc_port_send(stale, 0, 1, bytes, 4) == CL_STALE);
    CHECK(cc_register(&q, 0, &h[0]) == CL_OK);
    q.nodes[0].serial = UINT32_MAX;
    CHECK(cc_send(h[0], 0, 1, bytes, 4, cmx_forever(), &t) == CC_RETIRED &&
          cc_close(&q, 0) == CL_OK && cc_release(&h[0]) == CL_OK &&
          cc_destroy(&q) == CL_OK);
    q.life.generation = UINT64_MAX;
    CHECK(cc_restart(&q) == CL_RANGE && q.life.phase == CL_DESTROYED);
    CHECK(cq_setup(&q, &k, h));
    k.sequence = UINT32_MAX;
    t = (struct cc_ticket){.serial = 123};
    CHECK(cc_receive(h[0], 4, cmx_forever(), &t) == CC_RETIRED &&
          t.serial == 123 && q.nodes[0].phase == CC_IDLE &&
          k.task[0].state == CMX_READY);
    cmx_mutex mutex;
    cmx_cond condition = {0};
    CHECK(cmx_mutex_init(&k, &mutex, 0, 0) == CMX_OK &&
          cmx_mutex_lock(&k, &mutex, 0, cmx_forever()) == CMX_OK);
    CHECK(cc_condition(h[0], &condition, &mutex, cmx_forever(), &t) ==
              CC_RETIRED &&
          mutex.owner == 0 && t.serial == 123 && q.nodes[0].phase == CC_IDLE);
    CHECK(cmx_mutex_unlock(&k, &mutex, 0) == CMX_OK && cq_finish(&q, h));
    puts(
        "CC_LIFETIME:implicit-port-reference;drain-before-destroy;new-object-generation;stale-handle-and-callback;retirement-cleanup");
    return 0;
}

static int cq_dispatch(void)
{
    cmx_kernel k;
    struct cc_queue q;
    struct cc_handle h[5];
    struct cc_ticket tickets[5];
    unsigned phase[5] = {0}, done[5] = {0}, count[5] = {0}, expected[3] = {0};
    unsigned char payload[3][4] = {{0}};
    struct cl_message out;
    unsigned steps = 0, finished = 0;
    CHECK(cq_setup(&q, &k, h));
    while (finished < 5 && steps++ < 10000)
    {
        int selected = cmx_schedule(&k);
        CHECK(selected >= 0);
        unsigned t = (unsigned)selected;
        CHECK(t < 5 && !done[t]);
        if (!phase[t])
        {
            int result;
            if (t < 3)
            {
                payload[t][0] = (unsigned char)t;
                payload[t][1] = (unsigned char)count[t];
                payload[t][2] = (unsigned char)(count[t] ^ 0x55);
                payload[t][3] = 0xa5;
                result = cc_send(h[t], t, count[t], payload[t], 4,
                                 cmx_forever(), &tickets[t]);
            }
            else
                result = cc_receive(h[t], 4, cmx_forever(), &tickets[t]);
            CHECK(result == CL_OK || result == CC_BLOCKED);
            phase[t] = 1;
        }
        else
        {
            int result = cc_result(tickets[t], &out);
            CHECK(result == CL_OK);
            if (t >= 3)
            {
                CHECK(out.producer < 3 &&
                      out.sequence == expected[out.producer]++ &&
                      out.length == 4 && out.bytes[0] == out.producer &&
                      out.bytes[1] == out.sequence &&
                      out.bytes[2] == (out.sequence ^ 0x55) &&
                      out.bytes[3] == 0xa5);
            }
            count[t]++;
            phase[t] = 0;
            if (count[t] == (t < 3 ? 20u : 30u))
            {
                done[t] = 1;
                finished++;
                CHECK(cmx_sleep(&k, t, cmx_forever()) == CMX_BLOCKED);
            }
        }
    }
    CHECK(finished == 5 && q.bytes.accepted == 60 && q.bytes.delivered == 60 &&
          q.bytes.discarded == 0 && q.bytes.count == 0);
    for (unsigned t = 0; t < 5; t++)
        CHECK(cmx_cancel(&k, t) == CMX_OK);
    CHECK(expected[0] == 20 && expected[1] == 20 && expected[2] == 20 &&
          cq_finish(&q, h));
    puts(
        "CC_DISPATCH:actual-CMX-schedule;three-producers-two-consumers;60-copied-messages;per-producer-order;all-blocked-results-collected");
    return 0;
}
#ifndef CC_TARGET
int main(int argc, char **argv)
{
    if (argc != 2)
        return 2;
    if (!strcmp(argv[1], "queue"))
        return cq_queue();
    if (!strcmp(argv[1], "terminal"))
        return cq_terminal();
    if (!strcmp(argv[1], "condition"))
        return cq_condition();
    if (!strcmp(argv[1], "lifetime"))
        return cq_lifetime();
    if (!strcmp(argv[1], "dispatch"))
        return cq_dispatch();
    return 2;
}
#endif
