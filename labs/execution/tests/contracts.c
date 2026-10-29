#include "cmx.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define CHECK(x)                                                         \
    do                                                                   \
    {                                                                    \
        if (!(x))                                                        \
        {                                                                \
            fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, #x); \
            return 1;                                                    \
        }                                                                \
    } while (0)

static void setup(cmx_kernel *k)
{
    cmx_init(k, CMX_FIXED);
    for (unsigned i = 0; i < 4; i++)
        (void)cmx_task_add(k, 9 - i * 2, 40 - i * 8, 0x1000 + i * 0x1000, 512);
}

static int admission(void)
{
    cmx_kernel k;
    cmx_init(&k, CMX_RR);
    CHECK(cmx_task_add(&k, 4, 10, 0x1000, 512) == 0);
    CHECK(cmx_task_add(&k, 4, 10, 0x1100, 512) == CMX_NONE);
    CHECK(cmx_task_add(&k, 4, 10, 0x2001, 512) == CMX_NONE);
    CHECK(cmx_task_add(&k, 4, 10, UINTPTR_MAX - 7, 512) == CMX_NONE);
    for (unsigned i = 1; i < CMX_TASKS; i++)
        CHECK(cmx_task_add(&k, 4, 10, 0x1000 + i * 0x1000, 512) == (int)i);
    CHECK(cmx_task_add(&k, 4, 10, 0x100000, 512) == CMX_NONE);
    return 0;
}

static int enrollment(void)
{
    cmx_kernel k;
    cmx_latch l = {0};
    setup(&k);
    cmx_latch_signal(&k, &l);
    CHECK(cmx_latch_wait(&k, &l, 0, cmx_forever()) == CMX_OK);
    CHECK(cmx_latch_wait(&k, &l, 0, cmx_forever()) == CMX_BLOCKED);
    cmx_latch_signal(&k, &l);
    CHECK(k.task[0].state == CMX_READY && k.task[0].result == CMX_OK);
    cmx_latch_signal(&k, &l);
    cmx_latch_signal(&k, &l);
    CHECK(l.signaled == 1);
    return 0;
}

static int faults(void)
{
    cmx_kernel k;
    cmx_mutex m;
    cmx_cond c = {0};
    cmx_sem s;
    cmx_once o;
    cmx_rwlock r;
    setup(&k);
    cmx_sem_init(&s, 1, 1);
    CHECK(cmx_sleep(&k, 0, cmx_at(2)) == CMX_BLOCKED);
    CHECK(cmx_sem_take(&k, &s, 0, cmx_forever()) == CMX_INVALID &&
          s.count == 1);
    CHECK(cmx_exit(&k, 0, 0) == CMX_INVALID);
    cmx_tick(&k, 2);
    cmx_mutex_init(&k, &m, 0, 0);
    CHECK(cmx_mutex_lock(&k, &m, 0, cmx_forever()) == CMX_OK);
    CHECK(cmx_cond_wait(&k, &c, &m, 0, cmx_forever()) == CMX_BLOCKED);
    CHECK(cmx_mutex_close(&k, &m) == CMX_BLOCKED);
    CHECK(cmx_mutex_lock(&k, &m, 1, cmx_forever()) == CMX_OK);
    CHECK(cmx_cond_close(&k, &c) == CMX_OK);
    CHECK(cmx_cancel(&k, 0) == CMX_OK && k.task[0].state == CMX_WAITING);
    CHECK(cmx_mutex_unlock(&k, &m, 1) == CMX_OK && m.owner == 0 &&
          k.task[0].result == CMX_CLOSED);
    CHECK(cmx_mutex_unlock(&k, &m, 0) == CMX_OK);
    CHECK(cmx_mutex_close(&k, &m) == CMX_OK && k.mutex_count == 0);
    cmx_once_init(&o);
    CHECK(cmx_once_enter(&k, &o, 0, cmx_forever()) == CMX_RETRY);
    CHECK(cmx_exit(&k, 0, 0) == CMX_NOT_OWNER);
    CHECK(cmx_once_finish(&k, &o, 0, 0) == CMX_OK);
    cmx_rw_init(&r);
    CHECK(cmx_rw_read(&k, &r, 0, cmx_forever()) == CMX_OK);
    CHECK(cmx_exit(&k, 0, 0) == CMX_NOT_OWNER);
    CHECK(cmx_rw_close(&k, &r) == CMX_NOT_OWNER);
    CHECK(cmx_rw_unlock(&k, &r, 0) == CMX_OK);
    CHECK(cmx_rw_close(&k, &r) == CMX_OK);
    k.sequence = UINT32_MAX;
    CHECK(cmx_sleep(&k, 0, cmx_at(4)) == CMX_FULL);
    CHECK(cmx_invariant(&k));
    return 0;
}

static int collector_aba(void)
{
    cmx_hp_queue q;
    uint32_t value;
    cmx_hp_init(&q);
    CHECK(cmx_hp_push(&q, 0, 1) == CMX_OK);
    CHECK(cmx_hp_pop(&q, 1, &value) == CMX_OK);
    unsigned stale = atomic_load(&q.node[0].state);
    CHECK((stale & 3u) == 2u);
    CHECK(cmx_hp_collect(&q) == 1);
    CHECK(cmx_hp_push(&q, 0, 2) == CMX_OK);
    CHECK(cmx_hp_pop(&q, 1, &value) == CMX_OK);
    CHECK(cmx_hp_collect(&q) == 1);
    CHECK(cmx_hp_push(&q, 0, 3) == CMX_OK);
    atomic_store(&q.hazard[2][0], 0);
    CHECK(cmx_hp_pop(&q, 1, &value) == CMX_OK);
    CHECK(!atomic_compare_exchange_strong(&q.node[0].state, &stale, 0));
    CHECK((stale & 3u) == 2u && stale != 2u);
    CHECK(cmx_hp_collect(&q) == 0);
    atomic_store(&q.hazard[2][0], UINT32_MAX);
    CHECK(cmx_hp_collect(&q) == 1);
    return 0;
}

static int lifecycle(void)
{
    cmx_kernel k;
    setup(&k);
    CHECK(k.task[0].state == CMX_READY);
    CHECK(cmx_task_add(&k, 1, 1, 0x1000, 512) < 0);
    CHECK(cmx_task_add(&k, 1, 1, UINTPTR_MAX - 3, 512) < 0);
    CHECK(cmx_join(&k, 0, 1, cmx_forever()) == CMX_BLOCKED);
    CHECK(cmx_join(&k, 2, 1, cmx_forever()) == CMX_INVALID);
    CHECK(cmx_exit(&k, 1, 42) == CMX_OK);
    CHECK(k.task[0].value == 42 && k.task[0].state == CMX_READY);
    CHECK(cmx_reap(&k, 1) == CMX_OK);
    CHECK(k.task[1].generation == 1);
    CHECK(cmx_invariant(&k));
    return 0;
}

static int scheduler(void)
{
    cmx_kernel k;
    setup(&k);
    CHECK(cmx_schedule(&k) == 3);
    k.policy = CMX_RR;
    CHECK(cmx_schedule(&k) == 0);
    CHECK(cmx_schedule(&k) == 1);
    k.policy = CMX_EDF;
    CHECK(cmx_schedule(&k) == 3);
    k.task[0].job_deadline = 2;
    k.task[3].job_deadline = UINT32_MAX - 2;
    CHECK(cmx_schedule(&k) == 3);
    CHECK(cmx_invariant(&k));
    return 0;
}

static int timers(void)
{
    cmx_kernel k;
    setup(&k);
    k.now = UINT32_MAX - 2;
    CHECK(cmx_sleep(&k, 0, cmx_at(2)) == CMX_BLOCKED);
    cmx_tick(&k, 1);
    CHECK(k.task[0].state == CMX_WAITING);
    cmx_tick(&k, 2);
    CHECK(k.task[0].state == CMX_READY && k.task[0].result == CMX_OK);
    CHECK(cmx_sleep(&k, 0, cmx_at(2)) == CMX_TIMEOUT);
    CHECK(cmx_invariant(&k));
    return 0;
}

static int semaphore(void)
{
    cmx_kernel k;
    cmx_sem s;
    setup(&k);
    cmx_sem_init(&s, 0, 1);
    CHECK(cmx_sem_take(&k, &s, 0, cmx_at(5)) == CMX_BLOCKED);
    CHECK(cmx_sem_give(&k, &s) == CMX_OK);
    cmx_tick(&k, 5);
    CHECK(k.task[0].result == CMX_OK && s.count == 0);
    CHECK(cmx_sem_take(&k, &s, 1, cmx_at(7)) == CMX_BLOCKED);
    cmx_tick(&k, 7);
    CHECK(k.task[1].result == CMX_TIMEOUT);
    CHECK(cmx_sem_give(&k, &s) == CMX_OK && s.count == 1);
    CHECK(cmx_sem_give(&k, &s) == CMX_FULL);
    CHECK(cmx_invariant(&k));
    return 0;
}

static int mutex(void)
{
    cmx_kernel k;
    cmx_mutex m;
    setup(&k);
    CHECK(cmx_mutex_init(&k, &m, 0, 0) == CMX_OK);
    CHECK(cmx_mutex_lock(&k, &m, 0, cmx_forever()) == CMX_OK);
    CHECK(cmx_mutex_lock(&k, &m, 0, cmx_forever()) == CMX_DEADLOCK);
    CHECK(cmx_mutex_unlock(&k, &m, 1) == CMX_NOT_OWNER);
    CHECK(cmx_exit(&k, 0, 0) == CMX_NOT_OWNER);
    CHECK(cmx_mutex_lock(&k, &m, 1, cmx_forever()) == CMX_BLOCKED);
    CHECK(cmx_mutex_unlock(&k, &m, 0) == CMX_OK && m.owner == 1);
    CHECK(cmx_invariant(&k));
    return 0;
}

static int inheritance(void)
{
    cmx_kernel k;
    cmx_mutex a, b;
    setup(&k);
    cmx_mutex_init(&k, &a, 0, 0);
    cmx_mutex_init(&k, &b, 0, 0);
    CHECK(cmx_mutex_lock(&k, &a, 0, cmx_forever()) == CMX_OK);
    CHECK(cmx_mutex_lock(&k, &b, 1, cmx_forever()) == CMX_OK);
    CHECK(cmx_mutex_lock(&k, &a, 1, cmx_forever()) == CMX_BLOCKED);
    CHECK(cmx_mutex_lock(&k, &b, 3, cmx_at(5)) == CMX_BLOCKED);
    CHECK(k.task[0].priority == 3 && k.task[1].priority == 3);
    CHECK(cmx_mutex_lock(&k, &b, 0, cmx_forever()) == CMX_DEADLOCK);
    cmx_tick(&k, 5);
    CHECK(k.task[0].priority == 7 && k.task[1].priority == 7);
    CHECK(cmx_cancel(&k, 1) == CMX_OK);
    CHECK(k.task[0].priority == 9);
    CHECK(cmx_invariant(&k));
    return 0;
}

static int ceiling(void)
{
    cmx_kernel k;
    cmx_mutex m;
    setup(&k);
    cmx_mutex_init(&k, &m, 4, 1);
    CHECK(cmx_mutex_lock(&k, &m, 3, cmx_forever()) == CMX_INVALID);
    CHECK(cmx_mutex_lock(&k, &m, 0, cmx_forever()) == CMX_OK);
    CHECK(k.task[0].priority == 4);
    CHECK(cmx_mutex_unlock(&k, &m, 0) == CMX_OK);
    CHECK(k.task[0].priority == 9);
    return 0;
}

static int condition(void)
{
    cmx_kernel k;
    cmx_mutex m;
    cmx_cond c = {0};
    setup(&k);
    cmx_mutex_init(&k, &m, 0, 0);
    CHECK(cmx_mutex_lock(&k, &m, 0, cmx_forever()) == CMX_OK);
    CHECK(cmx_cond_wait(&k, &c, &m, 0, cmx_at(4)) == CMX_BLOCKED &&
          m.owner < 0);
    CHECK(cmx_mutex_lock(&k, &m, 1, cmx_forever()) == CMX_OK);
    cmx_tick(&k, 4);
    CHECK(k.task[0].state == CMX_WAITING && k.task[0].wait == CMX_W_MUTEX);
    CHECK(cmx_mutex_unlock(&k, &m, 1) == CMX_OK);
    CHECK(m.owner == 0 && k.task[0].result == CMX_TIMEOUT);
    CHECK(cmx_cond_wait(&k, &c, &m, 0, cmx_forever()) == CMX_BLOCKED);
    CHECK(cmx_cond_signal(&k, &c, 0) == CMX_OK && m.owner == 0);
    CHECK(cmx_invariant(&k));
    return 0;
}

static int queue(void)
{
    cmx_kernel k;
    cmx_queue q;
    setup(&k);
    cmx_queue_init(&q, 1);
    CHECK(cmx_queue_send(&k, &q, 0, 11, cmx_forever()) == CMX_OK);
    CHECK(cmx_queue_send(&k, &q, 1, 12, cmx_forever()) == CMX_BLOCKED);
    CHECK(cmx_queue_recv(&k, &q, 2, cmx_forever()) == CMX_OK &&
          k.task[2].value == 11);
    CHECK(k.task[1].state == CMX_READY);
    CHECK(cmx_queue_close(&k, &q) == CMX_OK);
    CHECK(cmx_queue_recv(&k, &q, 2, cmx_forever()) == CMX_OK &&
          k.task[2].value == 12);
    CHECK(cmx_queue_recv(&k, &q, 2, cmx_forever()) == CMX_CLOSED);
    cmx_queue_init(&q, 1);
    CHECK(cmx_queue_send(&k, &q, 0, 88, cmx_forever()) == CMX_OK);
    CHECK(cmx_queue_send(&k, &q, 1, 99, cmx_forever()) == CMX_BLOCKED);
    CHECK(cmx_queue_abort(&k, &q) == CMX_OK && k.task[1].result == CMX_CLOSED);
    CHECK(q.count == 0 &&
          cmx_queue_recv(&k, &q, 2, cmx_forever()) == CMX_CLOSED);
    cmx_queue_init(&q, 0);
    CHECK(cmx_queue_recv(&k, &q, 2, cmx_forever()) == CMX_BLOCKED);
    CHECK(cmx_queue_abort(&k, &q) == CMX_OK && k.task[2].result == CMX_CLOSED);
    cmx_queue_init(&q, 0);
    CHECK(cmx_queue_send(&k, &q, 0, 77, cmx_forever()) == CMX_BLOCKED);
    CHECK(cmx_queue_recv(&k, &q, 1, cmx_forever()) == CMX_OK &&
          k.task[1].value == 77);
    CHECK(cmx_invariant(&k));
    return 0;
}

static int events(void)
{
    cmx_kernel k;
    cmx_event e = {0};
    setup(&k);
    CHECK(cmx_event_wait(&k, &e, 0, 3, 1, 1, cmx_forever()) == CMX_BLOCKED);
    CHECK(cmx_event_wait(&k, &e, 1, 1, 0, 1, cmx_forever()) == CMX_BLOCKED);
    cmx_event_set(&k, &e, 1);
    CHECK(k.task[1].state == CMX_READY && k.task[0].state == CMX_WAITING);
    cmx_event_set(&k, &e, 3);
    CHECK(k.task[0].value == 3 && e.bits == 0);
    cmx_event_set(&k, &e, 4);
    cmx_event_set(&k, &e, 4);
    CHECK(e.bits == 4);
    return 0;
}

static int notification(void)
{
    cmx_kernel k;
    setup(&k);
    CHECK(cmx_notify_take(&k, 0, cmx_forever()) == CMX_BLOCKED);
    CHECK(cmx_notify(&k, 0, 3) == CMX_OK);
    CHECK(k.task[0].state == CMX_READY && k.task[0].notifications == 2);
    CHECK(cmx_notify_take(&k, 0, cmx_forever()) == CMX_OK);
    CHECK(cmx_notify(&k, 0, UINT32_MAX) == CMX_FULL);
    return 0;
}

static int barrier(void)
{
    cmx_kernel k;
    cmx_barrier b;
    setup(&k);
    cmx_barrier_init(&b, 2);
    CHECK(cmx_barrier_wait(&k, &b, 0, cmx_at(5)) == CMX_BLOCKED);
    CHECK(cmx_barrier_wait(&k, &b, 1, cmx_forever()) == CMX_OK);
    CHECK(b.generation == 1 && k.task[0].result == CMX_OK);
    CHECK(cmx_barrier_wait(&k, &b, 0, cmx_at(5)) == CMX_BLOCKED);
    cmx_tick(&k, 5);
    CHECK(b.broken && k.task[0].result == CMX_TIMEOUT);
    CHECK(cmx_barrier_wait(&k, &b, 1, cmx_forever()) == CMX_BROKEN);
    return 0;
}

static int once(void)
{
    cmx_kernel k;
    cmx_once o;
    setup(&k);
    cmx_once_init(&o);
    CHECK(cmx_once_enter(&k, &o, 0, cmx_forever()) == CMX_RETRY);
    CHECK(cmx_once_enter(&k, &o, 1, cmx_forever()) == CMX_BLOCKED);
    CHECK(cmx_once_enter(&k, &o, 0, cmx_forever()) == CMX_DEADLOCK);
    CHECK(cmx_once_finish(&k, &o, 0, 0) == CMX_OK);
    CHECK(k.task[1].result == CMX_RETRY);
    CHECK(cmx_once_enter(&k, &o, 1, cmx_forever()) == CMX_RETRY);
    CHECK(cmx_once_finish(&k, &o, 1, 1) == CMX_OK);
    CHECK(cmx_once_enter(&k, &o, 2, cmx_forever()) == CMX_OK);
    return 0;
}

static int rwlock(void)
{
    cmx_kernel k;
    cmx_rwlock r;
    setup(&k);
    cmx_rw_init(&r);
    CHECK(cmx_rw_read(&k, &r, 0, cmx_forever()) == CMX_OK);
    CHECK(cmx_rw_write(&k, &r, 0, cmx_forever()) == CMX_DEADLOCK);
    CHECK(cmx_rw_write(&k, &r, 1, cmx_forever()) == CMX_BLOCKED);
    CHECK(cmx_rw_read(&k, &r, 2, cmx_forever()) == CMX_BLOCKED);
    CHECK(cmx_rw_unlock(&k, &r, 0) == CMX_OK && r.writer == 1);
    CHECK(cmx_rw_unlock(&k, &r, 1) == CMX_OK && r.readers == (1u << 2));
    CHECK(cmx_rw_unlock(&k, &r, 2) == CMX_OK);
    CHECK(cmx_rw_read(&k, &r, 0, cmx_forever()) == CMX_OK);
    CHECK(cmx_rw_write(&k, &r, 1, cmx_at(5)) == CMX_BLOCKED);
    CHECK(cmx_rw_read(&k, &r, 2, cmx_forever()) == CMX_BLOCKED);
    cmx_tick(&k, 5);
    CHECK(k.task[1].result == CMX_TIMEOUT && k.task[2].result == CMX_OK);
    CHECK(r.readers == ((1u << 0) | (1u << 2)));
    CHECK(cmx_rw_write(&k, &r, 1, cmx_forever()) == CMX_BLOCKED);
    CHECK(cmx_rw_read(&k, &r, 3, cmx_forever()) == CMX_BLOCKED);
    CHECK(cmx_cancel(&k, 1) == CMX_OK && k.task[3].result == CMX_OK);
    CHECK(cmx_invariant(&k));
    return 0;
}

static int teardown(void)
{
    cmx_kernel k;
    cmx_sem s;
    setup(&k);
    cmx_sem_init(&s, 0, 1);
    CHECK(cmx_sem_take(&k, &s, 0, cmx_forever()) == CMX_BLOCKED);
    CHECK(cmx_sem_close(&k, &s) == CMX_OK && k.task[0].result == CMX_CLOSED);
    CHECK(cmx_sem_give(&k, &s) == CMX_CLOSED);
    CHECK(cmx_join(&k, 0, 1, cmx_at(3)) == CMX_BLOCKED);
    cmx_tick(&k, 3);
    CHECK(k.task[1].joiner < 0);
    CHECK(cmx_join(&k, 2, 1, cmx_forever()) == CMX_BLOCKED);
    CHECK(cmx_invariant(&k));
    return 0;
}

static int response(void)
{
    cmx_job j[] = {{1, 4, 4, 0, 0}, {2, 5, 5, 0, 0}, {2, 10, 10, 0, 1}};
    uint32_t r = 0;
    CHECK(cmx_response(j, 3, 1, &r) == CMX_OK && r == 3);
    CHECK(cmx_response(j, 3, 2, &r) == CMX_OK && r == 10);
    j[2].deadline = 9;
    CHECK(cmx_response(j, 3, 2, &r) == CMX_TIMEOUT);
    return 0;
}

static int edf(void)
{
    cmx_job j[] = {{1, 4, 4, 0, 0}, {2, 5, 5, 0, 0}};
    CHECK(cmx_edf_admit(j, 2, 20) == CMX_OK);
    j[1].cost = 5;
    CHECK(cmx_edf_admit(j, 2, 20) == CMX_TIMEOUT);
    j[1].jitter = 1;
    CHECK(cmx_edf_admit(j, 2, 20) == CMX_INVALID);
    return 0;
}

static int cbs(void)
{
    cmx_cbs s;
    CHECK(cmx_cbs_init(&s, 2, 5) == CMX_OK);
    CHECK(cmx_cbs_release(&s, 10) == CMX_OK && s.deadline == 15);
    CHECK(cmx_cbs_charge(&s, 3) == CMX_FULL && s.remaining == 2);
    CHECK(cmx_cbs_charge(&s, 2) == CMX_OK && s.deadline == 20);
    s.active = 0;
    CHECK(cmx_cbs_release(&s, 30) == CMX_OK && s.deadline == 35);
    return 0;
}

static int spsc(void)
{
    cmx_spsc q;
    uint32_t v;
    CHECK(cmx_spsc_init(&q, 4));
    atomic_store(&q.head, UINT32_MAX - 1);
    atomic_store(&q.tail, UINT32_MAX - 1);
    for (unsigned i = 0; i < 4; i++)
        CHECK(cmx_spsc_push(&q, i) == CMX_OK);
    CHECK(cmx_spsc_push(&q, 8) == CMX_FULL);
    for (unsigned i = 0; i < 4; i++)
    {
        CHECK(cmx_spsc_pop(&q, &v) == CMX_OK);
        CHECK(v == i);
    }
    CHECK(cmx_spsc_pop(&q, &v) == CMX_RETRY);
    return 0;
}

static int hazard(void)
{
    cmx_hp_queue q;
    uint32_t v;
    cmx_hp_init(&q);
    CHECK(cmx_hp_push(&q, 0, 42) == CMX_OK);
    atomic_store(&q.hazard[2][0], 0);
    CHECK(cmx_hp_pop(&q, 1, &v) == CMX_OK && v == 42);
    CHECK(cmx_hp_collect(&q) == 0);
    atomic_store(&q.hazard[2][0], UINT32_MAX);
    CHECK(cmx_hp_collect(&q) == 1);
    for (unsigned n = 0; n < 500; n++)
    {
        CHECK(cmx_hp_push(&q, 0, n) == CMX_OK);
        CHECK(cmx_hp_pop(&q, 1, &v) == CMX_OK && v == n);
        CHECK(cmx_hp_collect(&q) == 1);
    }
    CHECK(cmx_hp_pop(&q, 1, &v) == CMX_RETRY);
    return 0;
}

static int epoch(void)
{
    cmx_epoch e;
    cmx_epoch_init(&e);
    CHECK(cmx_epoch_enter(&e, 0) == CMX_OK);
    CHECK(cmx_epoch_advance(&e));
    CHECK(!cmx_epoch_advance(&e));
    cmx_epoch_leave(&e, 0);
    CHECK(cmx_epoch_advance(&e));
    e.epoch = UINT32_MAX;
    CHECK(!cmx_epoch_advance(&e));
    return 0;
}

int main(int argc, char **argv)
{
    struct entry
    {
        const char *name;
        int (*test)(void);
    } tests[] = {{"admission", admission},
                 {"enrollment", enrollment},
                 {"faults", faults},
                 {"collector_aba", collector_aba},
                 {"lifecycle", lifecycle},
                 {"scheduler", scheduler},
                 {"timers", timers},
                 {"semaphore", semaphore},
                 {"mutex", mutex},
                 {"inheritance", inheritance},
                 {"ceiling", ceiling},
                 {"condition", condition},
                 {"queue", queue},
                 {"events", events},
                 {"notification", notification},
                 {"barrier", barrier},
                 {"once", once},
                 {"rwlock", rwlock},
                 {"teardown", teardown},
                 {"response", response},
                 {"edf", edf},
                 {"cbs", cbs},
                 {"spsc", spsc},
                 {"hazard", hazard},
                 {"epoch", epoch}};

    if (argc != 2)
        return 2;
    for (size_t i = 0; i < sizeof tests / sizeof tests[0]; i++)
        if (!strcmp(argv[1], tests[i].name))
        {
            int r = tests[i].test();
            if (!r)
                printf("PASS execution.%s\n", argv[1]);
            return r;
        }
    return 2;
}
