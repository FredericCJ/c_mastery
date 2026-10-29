#include "comparison.h"
#include "cmx.h"

void fc_run(struct fc_report *r)
{
    cmx_kernel k;
    cmx_queue q;
    cmx_mutex m, second;
    cmx_init(&k, CMX_FIXED);
    for (unsigned n = 0; n < 3; ++n)
    {
        FC_REQUIRE(cmx_task_add(&k, 3 - n, 0, 0x1000u + n * 0x1000u, 256) ==
                   (int)n);
    }
    cmx_queue_init(&q, 8);
    unsigned ticks = 0;
    FC_REQUIRE(fc_ticks(21, FC_TICK_RATE, &ticks));
    r->ticks = ticks;
    for (unsigned n = 0; n < 8; ++n)
    {
        FC_REQUIRE(cmx_queue_send(&k, &q, 0, n, cmx_forever()) == CMX_OK);
    }
    FC_REQUIRE(cmx_queue_send(&k, &q, 0, 999, cmx_at(k.now + ticks)) ==
               CMX_BLOCKED);
    cmx_tick(&k, k.now + ticks);
    r->full = k.task[0].result == CMX_TIMEOUT;
    for (unsigned n = 0; n < 8; ++n)
    {
        FC_REQUIRE(cmx_queue_recv(&k, &q, 1, cmx_forever()) == CMX_OK);
        FC_REQUIRE(k.task[1].value == n);
    }
    FC_REQUIRE(cmx_queue_recv(&k, &q, 1, cmx_at(k.now + ticks)) == CMX_BLOCKED);
    cmx_tick(&k, k.now + ticks);
    r->empty = k.task[1].result == CMX_TIMEOUT;
    r->elapsed = ticks;
    unsigned sent = 0;
    while (r->count < 256)
    {
        while (sent < 256 && q.count < 8)
        {
            FC_REQUIRE(cmx_queue_send(&k, &q, 0, sent++, cmx_forever()) ==
                       CMX_OK);
        }
        FC_REQUIRE(cmx_queue_recv(&k, &q, 1, cmx_forever()) == CMX_OK);
        FC_REQUIRE(k.task[1].value == r->count);
        r->sum += k.task[1].value;
        ++r->count;
    }
    FC_REQUIRE(cmx_mutex_init(&k, &m, 0, 0) == CMX_OK);
    FC_REQUIRE(cmx_mutex_lock(&k, &m, 0, cmx_forever()) == CMX_OK);
    FC_REQUIRE(cmx_mutex_init(&k, &second, 0, 0) == CMX_OK);
    FC_REQUIRE(cmx_mutex_lock(&k, &second, 0, cmx_forever()) == CMX_OK);
    FC_REQUIRE(cmx_mutex_lock(&k, &m, 2, cmx_forever()) == CMX_BLOCKED);
    r->boosted = k.task[0].priority == 1;
    FC_REQUIRE(cmx_schedule(&k) == 0);
    r->trace[r->traces++] = 1;
    FC_REQUIRE(cmx_mutex_unlock(&k, &m, 0) == CMX_OK);
    r->nested_retained = k.task[0].priority == 1;
    FC_REQUIRE(cmx_mutex_unlock(&k, &second, 0) == CMX_OK);
    r->restored = k.task[0].priority == 3;
    FC_REQUIRE(k.task[2].result == CMX_OK && m.owner == 2);
    FC_REQUIRE(cmx_schedule(&k) == 2);
    r->trace[r->traces++] = 2;
    FC_REQUIRE(cmx_mutex_unlock(&k, &m, 2) == CMX_OK);
    FC_REQUIRE(cmx_exit(&k, 2, 0) == CMX_OK);
    FC_REQUIRE(cmx_schedule(&k) == 1);
    r->trace[r->traces++] = 3;
    FC_REQUIRE(cmx_mutex_close(&k, &m) == CMX_OK);
    FC_REQUIRE(cmx_mutex_close(&k, &second) == CMX_OK);
    FC_REQUIRE(cmx_queue_close(&k, &q) == CMX_OK);
    r->closed = cmx_queue_recv(&k, &q, 1, cmx_forever()) == CMX_CLOSED;
}
