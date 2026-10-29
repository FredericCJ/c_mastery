#include "outbox.h"
#ifndef RPO_MUTATION
#define RPO_MUTATION 0
#endif
static int live(const struct rpl_log *l)
{
    return l && l->pages && !l->poisoned && !l->pages->poisoned &&
           !l->pages->active;
}

int rpo_projection(const struct rpl_log *events, unsigned position,
                   int64_t *out)
{
    if (!events || !out || position > events->memory.count)
        return RPL_INVALID;
    if (!live(events))
        return RPL_STATE;
    struct rp_projection p = {0};
    for (unsigned i = 0; i < position; ++i)
        if (rp_project(&p, events->memory.events[i]) != RP_OK)
            return RPL_CORRUPT;
    *out = p.total;
    return RPL_OK;
}

int rpo_sender(struct rpl_log *events, struct rpl_log *ack,
               struct rpo_sender *out)
{
    if (!events || !ack || !out || events == ack || events->pages == ack->pages)
        return RPL_INVALID;
    if (!live(events) || !live(ack))
        return RPL_STATE;
    if ((RPO_MUTATION != 14 && events->uuid != ack->uuid) ||
        ack->schema != RPO_ACK_SCHEMA || events->schema == RPO_ACK_SCHEMA)
        return RPL_IDENTITY;
    if (ack->memory.count > events->memory.count)
        return RPL_CORRUPT;
    for (unsigned i = 0; i < ack->memory.count; ++i)
    {
        struct rp_event e = ack->memory.events[i];
        if (e.key != 0 || e.value != (int)(i + 1) || ack->kinds[i] != RPL_SET)
            return RPL_CORRUPT;
    }
    *out = (struct rpo_sender){events, ack};
    return RPL_OK;
}

int rpo_receiver(struct rpl_log *inbox, struct rpo_receiver *out)
{
    if (!inbox || !out || inbox->schema == RPO_ACK_SCHEMA)
        return RPL_INVALID;
    if (!live(inbox))
        return RPL_STATE;
    int64_t ignored;
    int r = rpo_projection(inbox, inbox->memory.count, &ignored);
    if (r)
        return r;
    *out = (struct rpo_receiver){inbox};
    return RPL_OK;
}

int rpo_next(struct rpo_sender *sender, struct rpl_event *out)
{
    if (!sender || !out)
        return RPL_INVALID;
    struct rpo_sender checked;
    int r = rpo_sender(sender->events, sender->acknowledgments, &checked);
    if (r)
        return r;
    unsigned at = checked.acknowledgments->memory.count;
    if (at == checked.events->memory.count)
        return RPO_EMPTY;
    struct rp_event e = checked.events->memory.events[at];
    *out = (struct rpl_event){e.position, e.key, checked.events->kinds[at],
                              e.value};
    return RPL_OK;
}

int rpo_receive(struct rpo_receiver *receiver, uint64_t uuid, uint32_t schema,
                struct rpl_event e, struct rpo_reply *out)
{
    if (!receiver || !out || !e.position || e.position > 16 || e.key > 31 ||
        (e.kind != 1 && e.kind != 2))
        return RPL_INVALID;
    struct rpl_log *inbox = receiver->inbox;
    if (!live(inbox))
        return RPL_STATE;
    if (uuid != inbox->uuid || schema != inbox->schema)
        return RPL_IDENTITY;
    unsigned count = inbox->memory.count;
    int replay = 0;
    if (e.position <= count && RPO_MUTATION != 11 && RPO_MUTATION != 19)
    {
        struct rp_event old = inbox->memory.events[e.position - 1];
        if (old.key != e.key || old.value != e.value ||
            inbox->kinds[e.position - 1] != e.kind)
            return RPO_CONFLICT;
        replay = 1;
    }
    else
    {
        if (e.position != count + 1 && RPO_MUTATION != 13 &&
            RPO_MUTATION != 11 && RPO_MUTATION != 19)
            return RPO_GAP;
        if (RPO_MUTATION == 17)
            *out = (struct rpo_reply){uuid, e.position, 0};
        int r = rpl_append(inbox, e.kind, e.key, e.value);
        if (r)
            return RPO_MUTATION == 18 ? RPL_OK : r;
    }
    int64_t total;
    int r = rpo_projection(inbox, e.position, &total);
    if (r)
        return r;
    *out = (struct rpo_reply){uuid, e.position, total};
    return replay ? RPO_REPLAY : RPL_OK;
}

int rpo_ack(struct rpo_sender *sender, struct rpo_reply reply)
{
    if (!sender)
        return RPL_INVALID;
    struct rpl_event next;
    int r = rpo_next(sender, &next);
    if (r)
        return r;
    int64_t expected;
    r = rpo_projection(sender->events, next.position, &expected);
    if (r)
        return r;
    if (RPO_MUTATION != 12 &&
        (reply.uuid != sender->events->uuid ||
         reply.position != next.position || reply.total != expected))
        return RPO_CONFLICT;
    if (RPO_MUTATION == 20)
        return RPL_OK;
    return rpl_append(sender->acknowledgments, RPL_SET, 0,
                      (int32_t)next.position);
}
