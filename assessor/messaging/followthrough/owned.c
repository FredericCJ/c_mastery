#include "owned.h"
#include <limits.h>
#ifndef MFC_MUTANT
#define MFC_MUTANT 0
#endif
static bool valid_value(const struct mfo_value *v)
{
    return v && (!v->data || v->drop);
}

void mfo_drop(struct mfo_value *value)
{
    struct mfo_value old = *value;
    *value = (struct mfo_value){0};
    if (old.drop)
        old.drop(old.data);
}

enum msg_status mfc_init(struct mfc_hub *h, size_t a, size_t b)
{
    if (!h || a > 4 || b > 4)
        return MSG_INVALID;
    *h = (struct mfc_hub){0};
    if (pthread_mutex_init(&h->records_mutex, NULL))
        return MSG_FAILED;
    enum msg_status status = msg_hub_init(&h->hub, a, b);
    if (status != MSG_OK)
        pthread_mutex_destroy(&h->records_mutex);
    return status;
}

enum msg_status mfc_send(struct mfc_hub *h, unsigned channel,
                         struct mfo_value *value, bool wait,
                         mfc_after_submit_fn hook, void *ctx)
{
    if (!h || channel > 1 || !valid_value(value))
        return MSG_INVALID;
    pthread_mutex_lock(&h->records_mutex);
    if (h->retired || h->active == UINT_MAX)
    {
        enum msg_status s = h->retired ? MSG_CLOSED : MSG_FULL;
        pthread_mutex_unlock(&h->records_mutex);
        return s;
    }
    unsigned slot = 0;
    while (slot < 8 && h->records[slot].used)
        slot++;
    if (slot == 8)
    {
        pthread_mutex_unlock(&h->records_mutex);
        return MSG_FULL;
    }
    h->records[slot] = (struct mfc_record){.value = *value, .used = true};
    h->active++;
    pthread_mutex_unlock(&h->records_mutex);
    enum msg_status status = msg_send(&h->hub, channel, slot, wait);
    if (hook)
        hook(ctx);
    pthread_mutex_lock(&h->records_mutex);
    struct mfc_record *record = &h->records[slot];
    if (status == MSG_OK)
    {
        *value = (struct mfo_value){0};
        record->send_done = true;
        if (record->delivered)
            record->used = false;
    }
    else
    {
        /* Imported failed submission cannot have been accepted by a receiver. */
        record->used = false;
        record->value = (struct mfo_value){0};
        if (MFC_MUTANT == 1)
            *value = (struct mfo_value){0};
    }
    h->active--;
    pthread_mutex_unlock(&h->records_mutex);
    return status;
}

enum msg_status mfc_select(struct mfc_hub *h, unsigned mask, bool wait,
                           unsigned *channel, struct mfo_value *out)
{
    if (!h || !channel || !out)
        return MSG_INVALID;
    pthread_mutex_lock(&h->records_mutex);
    if (h->retired || h->active == UINT_MAX)
    {
        enum msg_status status = h->retired ? MSG_CLOSED : MSG_FULL;
        pthread_mutex_unlock(&h->records_mutex);
        return status;
    }
    h->active++;
    pthread_mutex_unlock(&h->records_mutex);
    unsigned selected, slot;
    enum msg_status status = msg_select(&h->hub, mask, wait, &selected, &slot);
    pthread_mutex_lock(&h->records_mutex);
    if (status == MSG_OK)
    {
        struct mfc_record *record = &h->records[slot];
        *out = record->value;
        record->value = (struct mfo_value){0};
        record->delivered = true;
        if (record->send_done || MFC_MUTANT == 2)
            record->used = false;
        *channel = selected;
    }
    h->active--;
    pthread_mutex_unlock(&h->records_mutex);
    return status;
}

enum msg_status mfc_close(struct mfc_hub *h, unsigned channel)
{
    return h ? msg_close(&h->hub, channel) : MSG_INVALID;
}

void mfc_cancel(struct mfc_hub *h)
{
    msg_hub_cancel(&h->hub);
}

enum msg_status mfc_destroy(struct mfc_hub *h)
{
    if (!h)
        return MSG_INVALID;
    struct mfo_value values[8];
    unsigned count = 0;
    pthread_mutex_lock(&h->records_mutex);
    if (h->active)
    {
        pthread_mutex_unlock(&h->records_mutex);
        return MSG_BUSY;
    }
    h->retired = true;
    for (unsigned i = 0; i < 8; i++)
        if (h->records[i].used)
        {
            values[count++] = h->records[i].value;
            h->records[i].value = (struct mfo_value){0};
            h->records[i].used = false;
        }
    pthread_mutex_unlock(&h->records_mutex);
    msg_hub_cancel(&h->hub);
    for (unsigned i = 0; i < count; i++)
        mfo_drop(&values[i]);
    msg_hub_destroy(&h->hub);
    pthread_mutex_destroy(&h->records_mutex);
    return MSG_OK;
}

static bool actor_deliver(void *arg, unsigned slot)
{
    struct mfa_actor *a = arg;
    struct mfo_value value = a->records[slot].value;
    a->records[slot] = (struct mfa_record){0};
    bool keep = a->handler(a->ctx, &value);
    mfo_drop(&value);
    if (!keep)
        for (unsigned i = 0; i < 5; i++)
            if (a->records[i].used)
            {
                struct mfo_value pending = a->records[i].value;
                a->records[i] = (struct mfa_record){0};
                if (MFC_MUTANT != 3)
                    mfo_drop(&pending);
            }
    return keep;
}

enum msg_status mfa_init(struct mfa_actor *a, mfa_handler_fn handler, void *ctx,
                         mfa_dead_letter_fn dead, void *dc)
{
    if (!a || !handler || !dead)
        return MSG_INVALID;
    *a = (struct mfa_actor){
        .handler = handler, .dead = dead, .ctx = ctx, .dead_ctx = dc};
    msg_actor_init(&a->actor, actor_deliver, a);
    return MSG_OK;
}

enum msg_status mfa_send(struct mfa_actor *a, uint64_t generation,
                         struct mfo_value *value)
{
    if (!a || !valid_value(value))
        return MSG_INVALID;
    if (generation != a->actor.generation && MFC_MUTANT != 4)
    {
        struct mfo_value old = *value;
        *value = (struct mfo_value){0};
        a->dead(a->dead_ctx, generation, old);
        return MSG_STALE;
    }
    unsigned slot = 0;
    while (slot < 5 && a->records[slot].used)
        slot++;
    if (slot == 5)
        return MSG_FULL;
    enum msg_status status = msg_actor_send(
        &a->actor, MFC_MUTANT == 4 ? a->actor.generation : generation, slot);
    if (status == MSG_OK)
    {
        a->records[slot] = (struct mfa_record){*value, true};
        *value = (struct mfo_value){0};
    }
    return status;
}

enum msg_status mfa_step(struct mfa_actor *a)
{
    return a ? msg_actor_step(&a->actor) : MSG_INVALID;
}

enum msg_status mfa_restart(struct mfa_actor *a)
{
    return a ? msg_actor_restart(&a->actor) : MSG_INVALID;
}

enum msg_status mfa_destroy(struct mfa_actor *a)
{
    if (!a)
        return MSG_INVALID;
    if (a->actor.stepping)
        return MSG_BUSY;
    a->actor.alive = false;
    a->actor.count = 0;
    for (unsigned i = 0; i < 5; i++)
        if (a->records[i].used)
        {
            struct mfo_value value = a->records[i].value;
            a->records[i] = (struct mfa_record){0};
            mfo_drop(&value);
        }
    return MSG_OK;
}
