#define _POSIX_C_SOURCE 200809L
#include "modern.h"
#include <stdlib.h>
#include <unistd.h>
#include <sys/epoll.h>
#ifndef CS_MODERN_MUTANT
#define CS_MODERN_MUTANT 0
#endif
static er_progress result_sink(void *context, const unsigned char *bytes,
                               size_t n)
{
    struct cs_result_sink *a = context;
    struct cs_io r = a->sink(a->context, bytes, n);
    if ((unsigned)r.kind > CS_IO_ERROR ||
        ((r.kind == CS_IO_AGAIN || r.kind == CS_IO_INTERRUPTED) && r.accepted))
    {
        return (er_progress){n + 1, ER_CONTRACT};
    }
    er_code code = r.kind == CS_IO_ERROR                    ? ER_IO
                   : r.kind == CS_IO_PROGRESS && r.accepted ? ER_OK
                                                            : ER_AGAIN;
    return (er_progress){r.accepted, code};
}

er_progress cs_result_step(const unsigned char *b, size_t n, size_t next,
                           struct cs_result_sink *a)
{
    if (!a || !a->sink || n > CS_WIRE)
    {
        return (er_progress){next, ER_INVALID};
    }
    return er_transfer(b, n, next, result_sink, a);
}

static void slab_destroy(void *context)
{
    struct cs_slab *s = context;
    ++s->destructions;
}

void cs_slabs_init(struct cs_slabs *s)
{
    *s = (struct cs_slabs){0};
    for (unsigned i = 0; i < 4; ++i)
    {
        s->slot[i].generation = 1;
    }
}

enum cs_status cs_slabs_share(struct cs_slabs *p, const struct cp_message *m,
                              struct cs_slab_token out[2])
{
    if (!p || !m || !out || m->length > CP_PAYLOAD)
    {
        return CS_INVALID;
    }
    unsigned i;
    for (i = 0; i < 4; ++i)
    {
        if (!p->slot[i].live && p->slot[i].generation <= 3)
        {
            break;
        }
    }
    if (i == 4)
    {
        return CS_FULL;
    }
    struct cs_slab *s = &p->slot[i];
    if (cp_encode(m, s->wire.bytes, 42, &s->wire.length) != CP_OK)
    {
        return CS_INVALID;
    }
    s->wire.sequence = m->sequence;
    s->wire.replaceable = 0;
    s->rights = (struct ow_shared){1, 1, slab_destroy, NULL, s};
    if (ow_shared_retain(&s->rights) != OW_OK ||
        ow_shared_retain(&s->rights) != OW_OK ||
        ow_shared_drop(&s->rights) != OW_OK)
    {
        return CS_FAILED;
    }
    s->live = 3;
    out[0] = (struct cs_slab_token){i, s->generation, 0};
    out[1] = (struct cs_slab_token){i, s->generation, 1};
    return CS_OK;
}

enum cs_status cs_slabs_view(struct cs_slabs *p, struct cs_slab_token t,
                             const unsigned char **out, size_t *n)
{
    if (!p || !out || !n || t.slot >= 4 || t.consumer > 1)
    {
        return CS_INVALID;
    }
    struct cs_slab *s = &p->slot[t.slot];
    if (s->generation != t.generation || !(s->live & (1u << t.consumer)))
    {
        return CS_STALE;
    }
    *out = s->wire.bytes;
    *n = s->wire.length;
    return CS_OK;
}

enum cs_status cs_slabs_drop(struct cs_slabs *p, struct cs_slab_token t)
{
    if (!p || t.slot >= 4 || t.consumer > 1)
    {
        return CS_INVALID;
    }
    struct cs_slab *s = &p->slot[t.slot];
    if (s->generation != t.generation || !(s->live & (1u << t.consumer)))
    {
        return CS_STALE;
    }
    s->live &= ~(1u << t.consumer);
    if (CS_MODERN_MUTANT == 1 && s->live)
    {
        s->rights.strong = 1;
    }
    if (ow_shared_drop(&s->rights) != OW_OK)
    {
        return CS_FAILED;
    }
    if (!s->live)
    {
        ++s->generation;
    }
    return CS_OK;
}

enum cs_status cs_receipt(struct cs_receipts *r, unsigned epoch,
                          const struct cp_message *m)
{
    if (!r || !m || m->length > CP_PAYLOAD || !epoch || epoch != r->epoch)
    {
        return CS_INVALID;
    }
    for (unsigned i = 0; i < r->count; ++i)
    {
        if (r->messages[i].sequence == m->sequence)
        {
            const struct cp_message *a = &r->messages[i];
            if (a->length != m->length)
            {
                return CS_REJECT;
            }
            for (size_t j = 0; j < m->length; ++j)
            {
                if (a->payload[j] != m->payload[j])
                {
                    return CS_REJECT;
                }
            }
            if (CS_MODERN_MUTANT == 2)
            {
                ++r->effects;
            }
            return CS_DONE;
        }
    }
    if (r->count == 4)
    {
        return CS_FULL;
    }
    r->messages[r->count++] = *m;
    ++r->effects;
    return CS_FRAME;
}

static void resource_release(void *context)
{
    struct cs_resource_entry *e = context;
    struct cs_resources *r = e->owner;
    unsigned id = e->id;
    if (!(r->acquired & (1u << id)))
    {
        abort();
    }
    if (id == 4)
    {
        if (epoll_ctl(r->epoll, EPOLL_CTL_DEL, r->input, NULL))
        {
            r->cleanup_error = 1;
        }
    }
    if (id == 3)
    {
        free(r->queue);
        r->queue = NULL;
    }
    if (id == 2)
    {
        if (close(r->output))
        {
            r->cleanup_error = 1;
        }
        r->output = -1;
    }
    if (id == 1)
    {
        free(r->candidate);
        r->candidate = NULL;
    }
    if (id == 0)
    {
        if (close(r->input))
        {
            r->cleanup_error = 1;
        }
        r->input = -1;
    }
    r->acquired &= ~(1u << id);
    r->release_ids[r->released++] = id + 1;
}

int cs_resources_close(struct cs_resources *r)
{
    if (!r)
    {
        return -1;
    }
    r->published = 0;
    r->closing = 1;
    if (r->borrows && CS_MODERN_MUTANT != 3)
    {
        return 1;
    }
    if (r->modern)
    {
        if (ow_finish(&r->scope, 0) != OW_OK)
        {
            return -1;
        }
    }
    else
    {
        for (unsigned i = 5; i; i--)
        {
            if (r->acquired & (1u << (i - 1)))
            {
                resource_release(&r->entries[i - 1]);
            }
        }
    }
    return r->cleanup_error ? -1 : 0;
}

int cs_resources_open(struct cs_resources *r, int epoll, int input, int output,
                      unsigned fail, int modern)
{
    if (!r || fail > 6 || (modern != 0 && modern != 1))
    {
        return -1;
    }
    *r = (struct cs_resources){0};
    r->input = r->output = -1;
    r->epoll = epoll;
    r->modern = modern;
    for (unsigned i = 0; i < 5; ++i)
    {
        r->entries[i] = (struct cs_resource_entry){r, i};
        if (fail == i + 1)
        {
            (void)cs_resources_close(r);
            return -1;
        }
        int success = 1;
        if (i == 0)
        {
            r->input = dup(input);
            success = r->input >= 0;
        }
        if (i == 1)
        {
            r->candidate = malloc(84);
            success = r->candidate != NULL;
        }
        if (i == 2)
        {
            r->output = dup(output);
            success = r->output >= 0;
        }
        if (i == 3)
        {
            r->queue = malloc(3 * sizeof(struct cs_wire));
            success = r->queue != NULL;
        }
        if (i == 4)
        {
            struct epoll_event e = {.events = EPOLLIN, .data.u64 = 1};
            success = epoll_ctl(epoll, EPOLL_CTL_ADD, r->input, &e) == 0;
        }
        if (!success)
        {
            (void)cs_resources_close(r);
            return -1;
        }
        r->acquired |= 1u << i;
        if (modern && ow_defer(&r->scope, resource_release, &r->entries[i],
                               OW_ALWAYS) != OW_OK)
        {
            resource_release(&r->entries[i]);
            (void)cs_resources_close(r);
            return -1;
        }
    }
    if (fail == 6)
    {
        (void)cs_resources_close(r);
        return -1;
    }
    r->published = 1;
    return 0;
}

int cs_resources_borrow(struct cs_resources *r)
{
    if (!r || !r->published || r->borrows)
    {
        return -1;
    }
    r->borrows = 1;
    return 0;
}

int cs_resources_ack(struct cs_resources *r)
{
    if (!r || !r->borrows)
    {
        return -1;
    }
    r->borrows = 0;
    return r->closing ? cs_resources_close(r) : 0;
}
