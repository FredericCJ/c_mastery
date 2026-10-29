#include "mailbox.h"
#ifndef CDMB_MUTANT
#define CDMB_MUTANT 0
#endif
static void copy4(unsigned char *out, const unsigned char *in)
{
    for (unsigned i = 0; i < 4; i++)
        out[i] = in[i];
}

static int live(const struct cd_mailbox *m, unsigned task)
{
    return m && task < CMX_TASKS &&
           (m->kernel->task[task].state == CMX_READY ||
            m->kernel->task[task].state == CMX_RUNNING);
}

static int identity(const struct cd_mailbox *m, struct cdmb_identity id)
{
    return m && id.slot < 2 && id.session == m->session && id.generation &&
           id.generation <= CDMB_GENERATION_MAX;
}

int cdmb_init(struct cd_mailbox *m, cmx_kernel *k, unsigned capacity,
              uint32_t session)
{
    if (!m || !k || capacity > 1 || !session)
        return CMX_INVALID;
    *m = (struct cd_mailbox){0};
    m->kernel = k;
    m->capacity = capacity;
    m->session = session;
    cmx_queue_init(&m->queue, capacity);
    for (unsigned i = 0; i < 2; i++)
    {
        cmx_sem_init(&m->client[i].progress, 0, 2);
        cl_ack_init(&m->receipts[i], session);
    }
    return CMX_OK;
}

static void terminal(struct cd_mailbox *m, unsigned slot, int result)
{
    struct cdmb_client *c = &m->client[slot];
    c->phase = CDMB_TERMINAL;
    c->result = result;
    c->borrowed = 0;
    c->send_pending = c->progress_pending = 0;
}

/* Observe completion of the original transport wait before interpreting cancel.
   In rendezvous mode the wrapper's copied-byte boundary remains distinct. */
static void settle_send(struct cd_mailbox *m, unsigned slot)
{
    struct cdmb_client *c = &m->client[slot];
    cmx_task *t = &m->kernel->task[slot];
    if (!c->send_pending || t->state == CMX_WAITING)
        return;
    c->send_pending = 0;
    if (t->result != CMX_OK)
        terminal(m, slot, t->result);
    else if (m->capacity)
        c->delivered = 1;
}

/* A selected kernel terminal result wins even before the client polls it. */
static void settle(struct cd_mailbox *m, unsigned slot)
{
    settle_send(m, slot);
    struct cdmb_client *c = &m->client[slot];
    cmx_task *t = &m->kernel->task[slot];
    if (c->phase == CDMB_ACTIVE && c->progress_pending &&
        t->state != CMX_WAITING)
    {
        c->progress_pending = 0;
        if (t->result != CMX_OK)
            terminal(m, slot,
                     c->delivered && CDMB_MUTANT != 3 ? CDMB_UNKNOWN
                                                      : t->result);
    }
}

int cdmb_begin(struct cd_mailbox *m, unsigned slot,
               const unsigned char request[4], cmx_deadline deadline,
               struct cdmb_identity *out)
{
    if (!m || slot >= 2 || !request || !out || !live(m, slot) ||
        m->client[slot].phase != CDMB_IDLE)
        return CMX_INVALID;
    struct cdmb_client *c = &m->client[slot];
    if (CDMB_MUTANT != 4 && c->generation == CDMB_GENERATION_MAX)
        return CMX_FULL;
    uint32_t generation = c->generation + 1;
    *c = (struct cdmb_client){0};
    c->generation = generation;
    c->phase = CDMB_ACTIVE;
    c->deadline = deadline;
    cmx_sem_init(&c->progress, 0, 2);
    if (m->capacity)
        copy4(c->request, request);
    else
        c->borrowed = request;
    int result = cmx_queue_send(m->kernel, &m->queue, slot,
                                (generation << 1) | slot, deadline);
    if (result == CMX_BLOCKED)
        c->send_pending = 1;
    else if (result != CMX_OK)
        terminal(m, slot, result);
    else if (m->capacity)
        c->delivered = 1;
    *out = (struct cdmb_identity){m->session, generation, slot};
    return result; /* Transport admission/delivery result, not an application
                      reply. */
}

int cdmb_delivery(struct cd_mailbox *m, unsigned slot)
{
    if (!m || slot >= 2 || m->client[slot].phase == CDMB_IDLE)
        return CMX_INVALID;
    settle(m, slot);
    struct cdmb_client *c = &m->client[slot];
    if (c->phase == CDMB_TERMINAL)
        return c->result;
    return c->delivered || CDMB_MUTANT == 7 ? CMX_OK : CMX_BLOCKED;
}

int cdmb_receive(struct cd_mailbox *m, struct cdmb_request *out,
                 unsigned capacity, cmx_deadline deadline)
{
    if (!m || !out)
        return CMX_INVALID;
    if (capacity < 4)
        return CMX_FULL;
    int result;
    if (m->receive_pending)
    {
        if (m->kernel->task[2].state == CMX_WAITING)
            return CMX_BLOCKED;
        result = m->kernel->task[2].result;
        m->receive_pending = 0;
    }
    else
    {
        if (!live(m, 2))
            return CMX_INVALID;
        result = cmx_queue_recv(m->kernel, &m->queue, 2, deadline);
        if (result == CMX_BLOCKED)
        {
            m->receive_pending = 1;
            return result;
        }
    }
    if (result != CMX_OK)
        return result;
    uint32_t word = m->kernel->task[2].value;
    unsigned slot = word & 1u;
    uint32_t generation = word >> 1;
    settle(m, slot);
    struct cdmb_client *c = &m->client[slot];
    if (c->phase != CDMB_ACTIVE || c->generation != generation)
        return CDMB_STALE;
    struct cdmb_request value = {{m->session, generation, slot}, {0}};
    if (!m->capacity)
    {
        if (!c->borrowed)
            return CMX_INVALID;
        copy4(value.bytes, c->borrowed);
        copy4(c->request, value.bytes);
        c->borrowed = 0;
    }
    else
        copy4(value.bytes, c->request);
    c->send_pending = 0;
    c->delivered = 1;
    c->received = 1;
    /* At most one delivery signal and one reply signal exist per operation. */
    (void)cmx_sem_give(m->kernel, &c->progress);
    *out = value;
    return CMX_OK;
}

int cdmb_reply(struct cd_mailbox *m, struct cdmb_identity id,
               const unsigned char reply[4])
{
    if (!reply || !identity(m, id))
        return CDMB_STALE;
    if (CDMB_MUTANT != 8)
        settle(m, id.slot);
    struct cdmb_client *c = &m->client[id.slot];
    if (c->phase != CDMB_ACTIVE || !c->received || c->has_reply ||
        (CDMB_MUTANT != 1 && c->generation != id.generation))
        return CDMB_STALE;
    if (c->progress.closed || c->progress.count == c->progress.maximum)
        return CMX_FULL;
    copy4(c->reply, reply);
    c->has_reply = 1;
    return cmx_sem_give(m->kernel, &c->progress);
}

int cdmb_poll(struct cd_mailbox *m, unsigned slot, unsigned char *reply,
              unsigned capacity)
{
    if (!m || slot >= 2 || !reply || m->client[slot].phase == CDMB_IDLE)
        return CMX_INVALID;
    struct cdmb_client *c = &m->client[slot];
    if (capacity < 4)
    {
        if (CDMB_MUTANT == 2)
            c->phase = CDMB_IDLE;
        return CMX_FULL;
    }
    settle(m, slot);
    if (c->phase == CDMB_TERMINAL)
    {
        int result = c->result;
        c->phase = CDMB_IDLE;
        return result;
    }
    if (m->kernel->task[slot].state == CMX_WAITING)
        return CMX_BLOCKED;
    while (!c->has_reply)
    {
        int result = cmx_sem_take(m->kernel, &c->progress, slot, c->deadline);
        if (result == CMX_BLOCKED)
        {
            c->progress_pending = 1;
            return result;
        }
        if (result != CMX_OK)
        {
            int terminal_result =
                c->delivered && CDMB_MUTANT != 3 ? CDMB_UNKNOWN : result;
            terminal(m, slot, terminal_result);
            c->phase = CDMB_IDLE;
            return terminal_result;
        }
    }
    copy4(reply, c->reply);
    c->phase = CDMB_IDLE;
    c->borrowed = 0;
    return CMX_OK;
}

int cdmb_cancel(struct cd_mailbox *m, unsigned slot)
{
    if (!m || slot >= 2 || m->client[slot].phase == CDMB_IDLE)
        return CMX_INVALID;
    settle(m, slot);
    struct cdmb_client *c = &m->client[slot];
    if (c->phase == CDMB_TERMINAL)
        return c->result;
    if (c->has_reply)
        return CMX_OK;
    if (m->kernel->task[slot].state == CMX_WAITING)
        (void)cmx_cancel(m->kernel, slot);
    int result = c->delivered ? (CDMB_MUTANT == 3 ? CMX_TIMEOUT : CDMB_UNKNOWN)
                              : CMX_CLOSED;
    terminal(m, slot, result);
    return result;
}

int cdmb_restart(struct cd_mailbox *m)
{
    if (!m)
        return CMX_INVALID;
    if (m->session == UINT32_MAX)
        return CMX_FULL;
    for (unsigned slot = 0; slot < 2; slot++)
        if (m->client[slot].phase == CDMB_ACTIVE)
            (void)cdmb_cancel(m, slot);
    (void)cmx_queue_abort(m->kernel, &m->queue);
    m->receive_pending = 0;
    cmx_queue_init(&m->queue, m->capacity);
    if (CDMB_MUTANT != 5)
        m->session++;
    for (unsigned slot = 0; slot < 2; slot++)
    {
        uint32_t value = m->receipts[slot].value;
        cl_ack_init(&m->receipts[slot], m->session);
        m->receipts[slot].value = value;
    }
    return CMX_OK;
}

static int map(enum cl_result r)
{
    return r == CL_OK        ? CMX_OK
           : r == CL_FULL    ? CMX_FULL
           : r == CL_STALE   ? CDMB_STALE
           : r == CL_PENDING ? CMX_BLOCKED
                             : CMX_INVALID;
}

int cdmb_effect(struct cd_mailbox *m, struct cdmb_identity id, uint32_t delta,
                uint32_t *out)
{
    if (!out || !identity(m, id))
        return CDMB_STALE;
    struct cl_ack *a = &m->receipts[id.slot];
    enum cl_result result =
        cl_request(a, id.session, id.generation, delta, out);
    if (result == CL_PENDING)
    {
        result = cl_apply(a, id.generation);
        if (result == CL_OK)
            result = cl_request(a, id.session, id.generation, delta, out);
    }
    return map(result);
}

int cdmb_acknowledge(struct cd_mailbox *m, struct cdmb_identity id)
{
    if (!identity(m, id))
        return CDMB_STALE;
    int result = map(cl_acknowledge(&m->receipts[id.slot], id.generation));
    if (result == CMX_OK && CDMB_MUTANT == 6)
        m->receipts[id.slot].highwater = 0;
    return result;
}
