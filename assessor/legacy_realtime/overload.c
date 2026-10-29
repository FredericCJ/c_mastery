#include "overload.h"
#include "cmx.h"
#ifndef OD_MUTANT
#define OD_MUTANT 0
#endif
static unsigned task_valid(struct od_task t)
{
    return t.cost && t.period && t.cost <= t.period && t.period <= 64u &&
           t.critical <= 1u;
}

static unsigned set_valid(const struct od_set *s)
{
    if (s->count > OD_TASKS)
        return 0;
    for (unsigned i = 0; i < s->count; i++)
        if (!task_valid(s->tasks[i]))
            return 0;
    return 1;
}

static unsigned gcd(unsigned a, unsigned b)
{
    while (b)
    {
        unsigned r = a % b;
        a = b;
        b = r;
    }
    return a;
}

enum od_status od_admit(struct od_set *set, struct od_task task,
                        unsigned horizon, enum od_decision *decision)
{
    if (!set_valid(set) || !task_valid(task) || !horizon || horizon > OD_TICKS)
        return OD_INVALID;
    if (set->count == OD_TASKS)
        return OD_RESOURCE;
    struct od_set staged = *set;
    staged.tasks[staged.count++] = task;
    unsigned cycle = 4, rem[OD_TASKS] = {0}, deadline[OD_TASKS] = {0};
    for (unsigned i = 0; i < staged.count; i++)
        cycle =
            cycle / gcd(cycle, staged.tasks[i].period) * staged.tasks[i].period;
    /* At most4 periods<=64: lcm product with4 is below2^27. */
    unsigned end = cycle < horizon ? cycle : horizon;
    enum od_decision result = OD_INCONCLUSIVE;
    for (unsigned t = 0; t <= end; t++)
    {
        for (unsigned i = 0; i < staged.count; i++)
            if (rem[i] && deadline[i] == t)
            {
                result = OD_REJECTED;
                goto done;
            }
        if (t == end)
            break;
        for (unsigned i = 0; i < staged.count; i++)
            if (t % staged.tasks[i].period == 0)
            {
                rem[i] = staged.tasks[i].cost;
                deadline[i] = t + staged.tasks[i].period;
            }
        if (t % 4u == 0)
            continue;
        unsigned best = OD_TASKS;
        for (unsigned i = 0; i < staged.count; i++)
            if (rem[i] && (best == OD_TASKS || deadline[i] < deadline[best]))
                best = i;
        if (best < OD_TASKS)
            --rem[best];
    }
    if (end == cycle)
        result = OD_ADMITTED;
done:
    if (OD_MUTANT == 6 || result == OD_ADMITTED)
        *set = staged;
    *decision = result;
    return OD_OK;
}

static unsigned pending(const struct od_result *o, unsigned n)
{
    unsigned p = 0;
    for (unsigned i = 0; i < n; i++)
        p += o->items[i].state == OD_PENDING;
    return p;
}

static void finish(struct od_item *item, enum od_state state, unsigned now)
{
    item->state = state;
    item->finish = now;
}

static unsigned older(const struct od_request *r, unsigned a, unsigned b)
{
    return r[a].release < r[b].release ||
           (r[a].release == r[b].release && a < b);
}

enum od_status od_run(const struct od_set *set,
                      const struct od_request *requests, unsigned count,
                      unsigned capacity, unsigned horizon,
                      enum od_policy policy, unsigned supervisor,
                      uint32_t switch_at, uint32_t recover_at,
                      struct od_result *output)
{
    if (!set_valid(set) || !set->count || count > OD_REQUESTS || !capacity ||
        capacity > 8 || !horizon || horizon > OD_TICKS ||
        (unsigned)policy > (unsigned)OD_LATEST ||
        (supervisor != 0 && supervisor != 4))
        return OD_INVALID;
    for (unsigned i = 0; i < count; i++)
    {
        struct od_request r = requests[i];
        if (r.id == OD_NONE || r.stream >= set->count || r.release > OD_TICKS ||
            r.cost > 256 || r.deadline < r.release || r.deadline > 256 ||
            r.io_until > 256)
            return OD_INVALID;
        for (unsigned j = 0; j < i; j++)
            if (requests[j].id == r.id)
                return OD_INVALID;
    }
    struct od_result out = {0};
    unsigned mode = 0;
    for (unsigned i = 0; i < OD_REQUESTS; i++)
        out.items[i].finish = OD_NONE;
    for (unsigned t = 0; t <= horizon; t++)
    {
        if (t == switch_at)
        {
            mode = 1;
            ++out.mode_switches;
            for (unsigned i = 0; i < count; i++)
                if (out.items[i].state == OD_PENDING &&
                    !set->tasks[requests[i].stream].critical && OD_MUTANT != 7)
                    finish(&out.items[i], OD_MODE_DROP, t);
        }
        for (unsigned i = 0; i < count; i++)
            if (out.items[i].state == OD_PENDING && requests[i].deadline <= t)
                finish(&out.items[i], OD_MISS, t);
        if (t == recover_at)
        {
            if (pending(&out, count))
                ++out.recovery_refused;
            else
            {
                mode = 0;
                ++out.recovered;
            }
        }
        if (t == horizon)
            break;
        for (unsigned i = 0; i < count; i++)
            if (requests[i].release == t)
            {
                struct od_item *item = &out.items[i];
                struct od_request r = requests[i];
                if (mode && !set->tasks[r.stream].critical)
                {
                    finish(item, OD_MODE_REJECT, t);
                    continue;
                }
                if (!r.cost)
                {
                    finish(item, OD_DONE, t);
                    continue;
                }
                if (r.deadline == t)
                {
                    finish(item, OD_MISS, t);
                    continue;
                }
                unsigned size = pending(&out, count), victim = OD_REQUESTS;
                if (policy == OD_LATEST)
                    for (unsigned j = 0; j < count; j++)
                        if (out.items[j].state == OD_PENDING &&
                            requests[j].stream == r.stream &&
                            (victim == OD_REQUESTS ||
                             older(requests, j, victim)))
                            victim = j;
                if (victim == OD_REQUESTS && size >= capacity &&
                    policy == OD_DROP_OLDEST)
                    for (unsigned j = 0; j < count; j++)
                        if (out.items[j].state == OD_PENDING &&
                            (victim == OD_REQUESTS ||
                             older(requests, j, victim)))
                            victim = j;
                if (victim < OD_REQUESTS)
                {
                    finish(&out.items[victim],
                           OD_MUTANT == 2 ? OD_DONE : OD_DROP, t);
                    --size;
                }
                if (size >= capacity && OD_MUTANT != 1)
                {
                    finish(item, OD_REJECT, t);
                    continue;
                }
                item->state = OD_PENDING;
                ++size;
                if (size > out.peak)
                    out.peak = size;
            }
        unsigned best = OD_REQUESTS;
        for (unsigned i = 0; i < count; i++)
            if (out.items[i].state == OD_PENDING && requests[i].io_until <= t &&
                (best == OD_REQUESTS ||
                 requests[i].deadline < requests[best].deadline ||
                 (requests[i].deadline == requests[best].deadline &&
                  older(requests, i, best))))
                best = i;
        struct od_tick *tick = &out.ticks[t];
        tick->selected = OD_NONE;
        tick->mode = mode;
        if (supervisor && t % supervisor == 0 && OD_MUTANT != 3)
        {
            ++out.heartbeats;
            tick->heartbeat = 1;
        }
        else if (best < count)
        {
            struct od_item *item = &out.items[best];
            ++item->executed;
            ++out.work;
            tick->selected = requests[best].id;
            if (item->executed == requests[best].cost)
                finish(item, OD_DONE, t + 1);
            else if (item->executed == set->tasks[requests[best].stream].cost &&
                     OD_MUTANT != 4)
                finish(item, OD_OVERRUN, t + 1);
        }
        else
            ++out.idle;
        tick->pending = pending(&out, count);
    }
    *output = out;
    return OD_OK;
}

enum od_status od_backpressure(struct od_dependency *output)
{
    cmx_kernel k;
    cmx_mutex mutex;
    cmx_sem spaces;
    cmx_init(&k, CMX_FIXED);
    if (cmx_task_add(&k, 2, 20, 0x1000, 512) != 0 ||
        cmx_task_add(&k, 1, 10, 0x2000, 512) != 1)
        return OD_RESOURCE;
    cmx_mutex_init(&k, &mutex, 0, 0);
    cmx_sem_init(&spaces, 0, 2);
    if (cmx_mutex_lock(&k, &mutex, 0, cmx_forever()) != CMX_OK ||
        cmx_sem_take(&k, &spaces, 0, cmx_forever()) != CMX_BLOCKED ||
        cmx_mutex_lock(&k, &mutex, 1, cmx_forever()) != CMX_BLOCKED)
        return OD_RESOURCE;
    struct od_dependency result = {0};
    result.producer_waiting = k.task[0].state == CMX_WAITING;
    result.consumer_waiting = k.task[1].state == CMX_WAITING;
    result.mutex_owner = (unsigned)mutex.owner;
    if (OD_MUTANT != 5)
    {
        if (cmx_cancel(&k, 0) != CMX_OK ||
            cmx_mutex_unlock(&k, &mutex, 0) != CMX_OK)
            return OD_RESOURCE;
    }
    result.repaired_owner = (unsigned)mutex.owner;
    result.producer_result = (unsigned)k.task[0].result;
    if (cmx_sem_give(&k, &spaces) != CMX_OK)
        return OD_RESOURCE;
    result.space = spaces.count;
    /* The returned trace owns no live CMX registrations. Close waits before
     * releasing the remaining owner, then retire both local task records. */
    for (unsigned t = 0; t < 2; t++)
        if (k.task[t].state == CMX_WAITING && cmx_cancel(&k, t) != CMX_OK)
            return OD_RESOURCE;
    if (cmx_sem_close(&k, &spaces) != CMX_OK ||
        cmx_mutex_close(&k, &mutex) != CMX_NOT_OWNER ||
        cmx_mutex_unlock(&k, &mutex, (unsigned)mutex.owner) != CMX_OK ||
        cmx_mutex_close(&k, &mutex) != CMX_OK || cmx_exit(&k, 0, 0) != CMX_OK ||
        cmx_exit(&k, 1, 0) != CMX_OK)
        return OD_RESOURCE;
    *output = result;
    return OD_OK;
}
