#include "barrier.h"
#ifndef CDB_MUTANT
#define CDB_MUTANT 0
#endif
#define BIT(t) (UINT32_C(1) << (t))

static unsigned count(uint32_t bits)
{
    unsigned n = 0;
    for (unsigned i = 0; i < CMX_TASKS; i++)
        n += (bits >> i) & 1u;
    return n;
}

static int members_valid(cmx_kernel *k, uint32_t members)
{
    if (!k || !members || (members >> CMX_TASKS))
        return 0;
    for (unsigned i = 0; i < CMX_TASKS; i++)
        if (members & BIT(i))
            if (k->task[i].state != CMX_READY &&
                k->task[i].state != CMX_RUNNING)
                return 0;
    return 1;
}

static int valid(struct cdb_member h)
{
    struct cd_barrier *b = h.owner;
    return b && b->live && h.task < CMX_TASKS && h.epoch == b->epoch &&
           (b->registered & BIT(h.task)) &&
           b->slot[h.task].task_generation ==
               b->kernel->task[h.task].generation &&
           b->kernel->task[h.task].state != CMX_UNUSED &&
           b->kernel->task[h.task].state != CMX_EXITED;
}

static void settle(struct cd_barrier *b)
{
    for (unsigned i = 0; i < CMX_TASKS; i++)
        if (b->slot[i].state == CDB_PENDING &&
            b->kernel->task[i].state != CMX_WAITING)
        {
            b->slot[i].state = CDB_DONE;
            b->slot[i].result = b->kernel->task[i].result;
        }
}

static int drained(struct cd_barrier *b)
{
    settle(b);
    if (b->raw.arrived)
        return 0;
    for (unsigned i = 0; i < CMX_TASKS; i++)
        if (b->slot[i].state != CDB_IDLE)
            return 0;
    return 1;
}

int cdb_init(struct cd_barrier *b, cmx_kernel *k, uint32_t members,
             unsigned limit)
{
    if (!b || !members_valid(k, members) || !limit || limit > 255)
        return CMX_INVALID;
    *b = (struct cd_barrier){.kernel = k,
                             .members = members,
                             .epoch = 1,
                             .generation_limit = limit,
                             .live = 1};
    cmx_barrier_init(&b->raw, count(members));
    return CMX_OK;
}

int cdb_register(struct cd_barrier *b, unsigned t, struct cdb_member *out)
{
    if (!b || !b->live || !out || t >= CMX_TASKS || b->changing ||
        !(b->members & BIT(t)) || (b->registered & BIT(t)) ||
        !members_valid(b->kernel, BIT(t)))
        return CMX_INVALID;
    b->slot[t].task_generation = b->kernel->task[t].generation;
    b->registered |= BIT(t);
    *out = (struct cdb_member){b, t, b->epoch};
    return CMX_OK;
}

int cdb_arrive(struct cdb_member h, cmx_deadline deadline,
               struct cdb_ticket *out)
{
    if (!valid(h))
        return CDB_STALE;
    struct cd_barrier *b = h.owner;
    settle(b);
    struct cdb_slot *s = &b->slot[h.task];
    if (!out || b->changing || b->registered != b->members ||
        s->state != CDB_IDLE)
        return CMX_INVALID;
    if (b->raw.broken)
        return CMX_BROKEN;
    if (b->raw.generation >= b->generation_limit)
    {
        if (CDB_MUTANT == 5)
            b->raw.generation = 0;
        else
            return CDB_RETIRED;
    }
    unsigned round = b->raw.generation, parties = b->raw.parties;
    if (CDB_MUTANT == 2)
        b->raw.parties = b->raw.arrived + 1;
    int result = cmx_barrier_wait(b->kernel, &b->raw, h.task, deadline);
    b->raw.parties = parties;
    if (result != CMX_OK && result != CMX_BLOCKED)
        return result; /* No admitted arrival. */
    s->round = round;
    s->state = result == CMX_BLOCKED ? CDB_PENDING : CDB_DONE;
    s->result = result;
    *out = (struct cdb_ticket){h, round};
    settle(b);
    return result;
}

int cdb_result(struct cdb_ticket ticket, unsigned *out)
{
    if (!valid(ticket.member))
        return CDB_STALE;
    struct cd_barrier *b = ticket.member.owner;
    settle(b);
    struct cdb_slot *s = &b->slot[ticket.member.task];
    if (!out)
        return CMX_INVALID;
    if (s->state == CDB_IDLE || (CDB_MUTANT != 1 && ticket.round != s->round))
        return CDB_STALE;
    if (s->state == CDB_PENDING)
        return CMX_BLOCKED;
    int result = s->result;
    if (result == CMX_OK)
        *out = s->round;
    s->state = CDB_IDLE;
    return result;
}

int cdb_cancel(struct cdb_ticket ticket)
{
    if (!valid(ticket.member))
        return CDB_STALE;
    struct cd_barrier *b = ticket.member.owner;
    settle(b);
    struct cdb_slot *s = &b->slot[ticket.member.task];
    if (s->state == CDB_IDLE || ticket.round != s->round)
        return CDB_STALE;
    if (s->state == CDB_PENDING)
    {
        if (CDB_MUTANT == 3)
            return CMX_OK;
        int result = cmx_cancel(b->kernel, ticket.member.task);
        if (result != CMX_OK)
            return result;
        settle(b);
    }
    return s->result;
}

int cdb_stage(struct cd_barrier *b, uint32_t next)
{
    if (!b || !b->live || b->changing || !next || (next >> CMX_TASKS))
        return CMX_INVALID;
    if (CDB_MUTANT != 4 && !drained(b))
        return CMX_BLOCKED;
    if (!members_valid(b->kernel, next))
        return CMX_INVALID;
    b->next_members = next;
    b->changing = 1;
    return CMX_OK;
}

int cdb_release(struct cdb_member *h)
{
    if (!h || !valid(*h))
        return CDB_STALE;
    struct cd_barrier *b = h->owner;
    if (!b->changing || !drained(b))
        return CMX_BLOCKED;
    b->registered &= ~BIT(h->task);
    *h = (struct cdb_member){0};
    return CMX_OK;
}

int cdb_apply(struct cd_barrier *b)
{
    if (!b || !b->live || !b->changing || !b->next_members)
        return CMX_INVALID;
    if (b->registered || !drained(b))
        return CMX_BLOCKED;
    if (b->epoch == UINT32_MAX || b->raw.generation >= b->generation_limit)
        return CDB_RETIRED;
    if (!members_valid(b->kernel, b->next_members))
        return CMX_INVALID;
    b->members = b->next_members;
    b->next_members = 0;
    b->epoch++;
    b->changing = 0;
    b->raw.parties = count(b->members);
    b->raw.broken = 0;
    return CMX_OK;
}

int cdb_close(struct cd_barrier *b)
{
    if (!b || !b->live)
        return CMX_INVALID;
    if (!drained(b))
        return CMX_BLOCKED;
    b->changing = 1;
    b->next_members = 0;
    return CMX_OK;
}

int cdb_destroy(struct cd_barrier *b)
{
    if (!b || !b->live)
        return CMX_INVALID;
    if (!b->changing || b->next_members || b->registered || !drained(b))
        return CMX_BLOCKED;
    b->live = 0;
    return CMX_OK;
}
