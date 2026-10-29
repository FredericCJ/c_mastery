#include "operation.h"
#include <stdio.h>
#include <stdlib.h>
#define CHECK(x)                                                            \
    do                                                                      \
    {                                                                       \
        if (!(x))                                                           \
        {                                                                   \
            fprintf(stderr, "SC_CHECK: operation_follow:%d:%s\n", __LINE__, \
                    #x);                                                    \
            exit(1);                                                        \
        }                                                                   \
    } while (0)

struct msg_counts
{
    unsigned drops, starts, callbacks, frees;
};

struct msg_payload
{
    struct msg_counts *counts;
    int number;
};
struct msg_owned;

struct msg_owned
{
    struct mfo_operation op;
    struct mfo_value value;
    struct msg_owned *next;
    struct msg_counts *counts;
};

static void payload_drop(void *p)
{
    struct msg_payload *value = p;
    value->counts->drops++;
    free(value);
}

static void inline_backend(struct mfo_operation *op, void *arg)
{
    struct msg_owned *owner = arg;
    owner->counts->starts++;
    /* The argument descriptor is a separate automatic object, not inside op. */
    struct mfo_value value = owner->value;
    owner->value = (struct mfo_value){0};
    CHECK(mfo_finish(op, MSG_OK, &value) == MSG_OK);
    CHECK(!value.data && !value.drop);
    /* Do not touch owner here: it has been freed by the receiver. */
}

static void chained(void *arg, enum msg_status status, struct mfo_value value)
{
    struct msg_owned *owner = arg;
    CHECK(status == MSG_OK && value.data && value.drop);
    struct msg_counts *counts = owner->counts;
    struct msg_owned *next = owner->next;
    CHECK(((struct msg_payload *)value.data)->number == 42);
    counts->callbacks++;
    if (next)
        next->value = value;
    else
        mfo_drop(&value);
    counts->frees++;
    free(owner);
    /* Successor observes ownership already transferred, predecessor no longer live. */
    if (next)
        CHECK(mfo_start(&next->op) == MSG_OK);
}

struct msg_later
{
    unsigned callbacks, starts;
    enum msg_status status;
    struct mfo_value value;
};

static void deferred_backend(struct mfo_operation *op, void *arg)
{
    (void)op;
    ((struct msg_later *)arg)->starts++;
}

static void deferred_receiver(void *arg, enum msg_status status,
                              struct mfo_value value)
{
    struct msg_later *later = arg;
    later->callbacks++;
    later->status = status;
    later->value = value;
}

int main(void)
{
    struct msg_counts counts = {0};
    struct msg_owned *first = calloc(1, sizeof(*first)),
                     *second = calloc(1, sizeof(*second));
    struct msg_payload *payload = malloc(sizeof(*payload));
    CHECK(first && second && payload);
    *payload = (struct msg_payload){&counts, 42};
    first->value = (struct mfo_value){payload, payload_drop};
    first->next = second;
    first->counts = second->counts = &counts;
    CHECK(mfo_init(&first->op, inline_backend, first, chained, first) ==
          MSG_OK);
    CHECK(mfo_init(&second->op, inline_backend, second, chained, second) ==
          MSG_OK);
    CHECK(mfo_start(&first->op) == MSG_OK);
    CHECK(counts.starts == 2 && counts.callbacks == 2 && counts.frees == 2 &&
          counts.drops == 1);
    struct mfo_operation op;
    struct msg_later later = {0};
    struct mfo_value empty = {0};
    CHECK(mfo_init(&op, deferred_backend, &later, deferred_receiver, &later) ==
          MSG_OK);
    CHECK(mfo_finish(&op, MSG_OK, &empty) == MSG_BUSY);
    CHECK(mfo_start(&op) == MSG_OK && later.starts == 1);
    CHECK(mfo_start(&op) == MSG_INVALID && later.starts == 1);
    CHECK(mfo_stop(&op) == MSG_OK && later.callbacks == 0);
    payload = malloc(sizeof(*payload));
    CHECK(payload);
    *payload = (struct msg_payload){&counts, 42};
    struct mfo_value owned = {payload, payload_drop};
    CHECK(mfo_finish(&op, MSG_EMPTY, &owned) == MSG_INVALID &&
          owned.data == payload && !later.callbacks);
    CHECK(mfo_finish(&op, MSG_CANCELLED, &owned) == MSG_OK && !owned.data);
    CHECK(later.callbacks == 1 && later.status == MSG_CANCELLED &&
          later.value.data == payload);
    mfo_drop(&later.value);
    CHECK(counts.drops == 2);
    CHECK(mfo_finish(&op, MSG_OK, &empty) == MSG_BUSY && later.callbacks == 1);
    CHECK(mfo_stop(&op) == MSG_INVALID);
    puts("inline free-on-completion and owning successor passed");
    return 0;
}
