#include "completion.h"
#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define CHECK(x)                                                           \
    do                                                                     \
    {                                                                      \
        if (!(x))                                                          \
        {                                                                  \
            fprintf(stderr, "SC_CHECK: completion:%d:%s\n", __LINE__, #x); \
            exit(1);                                                       \
        }                                                                  \
    } while (0)

struct msg_counts
{
    atomic_uint calls, drops, releases, ready;
    atomic_bool go;
    enum msg_status status;
};

struct msg_context
{
    struct msg_counts *counts;
    bool free_context;
};

static void payload_drop(void *p)
{
    struct msg_counts *c = p;
    atomic_fetch_add(&c->drops, 1);
}

static void control_drop(void *p)
{
    struct msg_counts *c = p;
    atomic_fetch_add(&c->releases, 1);
}

static void receiver(void *p, enum msg_status status, struct mfo_value value)
{
    struct msg_context *ctx = p;
    struct msg_counts *c = ctx->counts;
    c->status = status;
    atomic_fetch_add(&c->calls, 1);
    mfo_drop(&value);
    if (ctx->free_context)
    {
        free(ctx);
    }
}

static void initialize(struct msg_counts *c)
{
    atomic_init(&c->calls, 0);
    atomic_init(&c->drops, 0);
    atomic_init(&c->releases, 0);
    atomic_init(&c->ready, 0);
    atomic_init(&c->go, false);
    c->status = MSG_INVALID;
}

static void boundaries(void)
{
    struct msg_counts c;
    initialize(&c);
    struct msg_context ctx = {&c, false};
    struct mfx_completion *root = NULL, *a = NULL, *b = NULL;
    CHECK(mfx_create(&root, receiver, &ctx, control_drop, &c, true) ==
              MSG_FAILED &&
          !root);
    CHECK(mfx_create(&root, receiver, &ctx, control_drop, &c, false) == MSG_OK);
    CHECK(mfx_retain(root, &a) == MSG_OK && mfx_retain(root, &b) == MSG_OK);
    CHECK(atomic_load(&root->references) ==
          3); /* Control112 fails before unsafe use. */
    atomic_store(&root->references, SIZE_MAX);
    struct mfx_completion *unchanged = NULL;
    CHECK(mfx_retain(root, &unchanged) == MSG_OVERFLOW && !unchanged);
    atomic_store(&root->references, 3);
    CHECK(mfx_cancel(root) == MSG_OK && atomic_load(&c.calls) == 0);
    mfx_drop(&root);
    CHECK(!root && atomic_load(&c.releases) == 0);
    struct mfo_value first = {&c, payload_drop}, second = {&c, payload_drop};
    CHECK(mfx_complete(a, MSG_OK, &first) == MSG_OK && !first.data &&
          c.status == MSG_CANCELLED);
    CHECK(mfx_complete(b, MSG_FAILED, &second) == MSG_BUSY &&
          second.data == &c);
    CHECK(mfx_cancel(b) == MSG_CLOSED && atomic_load(&c.calls) == 1);
    mfo_drop(&second);
    mfx_drop(&a);
    CHECK(atomic_load(&c.releases) == 0);
    mfx_drop(&b);
    CHECK(atomic_load(&c.releases) == 1 && atomic_load(&c.drops) == 2);
    mfx_drop(&b);
}

struct msg_arg
{
    struct mfx_completion *token;
    struct msg_counts *counts;
    unsigned role;
    enum msg_status result;
};

static void *race(void *p)
{
    struct msg_arg *a = p;
    struct msg_counts *c = a->counts;
    atomic_fetch_add(&c->ready, 1);
    while (!atomic_load(&c->go))
    {
        sched_yield();
    }
    if (a->role == 2)
    {
        a->result = mfx_cancel(a->token);
    }
    else
    {
        struct mfo_value owner = {c, payload_drop};
        a->result =
            mfx_complete(a->token, a->role ? MSG_FAILED : MSG_OK, &owner);
        mfo_drop(&owner);
    }
    mfx_drop(&a->token);
    return NULL;
}

static void races(void)
{
    for (unsigned round = 0; round < 128; round++)
    {
        struct msg_counts c;
        initialize(&c);
        struct msg_context *ctx = malloc(sizeof(*ctx));
        CHECK(ctx);
        *ctx = (struct msg_context){&c, true};
        struct mfx_completion *root = NULL;
        CHECK(mfx_create(&root, receiver, ctx, control_drop, &c, false) ==
              MSG_OK);
        struct msg_arg args[3];
        pthread_t threads[3];
        for (unsigned i = 0; i < 3; i++)
        {
            args[i] = (struct msg_arg){.counts = &c, .role = i};
            CHECK(mfx_retain(root, &args[i].token) == MSG_OK);
        }
        CHECK(atomic_load(&root->references) == 4);
        mfx_drop(&root);
        for (unsigned i = 0; i < 3; i++)
        {
            CHECK(!pthread_create(&threads[i], NULL, race, &args[i]));
        }
        while (atomic_load(&c.ready) != 3)
        {
            sched_yield();
        }
        atomic_store(&c.go, true);
        for (unsigned i = 0; i < 3; i++)
        {
            CHECK(!pthread_join(threads[i], NULL));
        }
        CHECK((args[0].result == MSG_OK && args[1].result == MSG_BUSY) ||
              (args[1].result == MSG_OK && args[0].result == MSG_BUSY));
        CHECK(args[2].result == MSG_OK || args[2].result == MSG_CLOSED);
        CHECK(atomic_load(&c.calls) == 1 && atomic_load(&c.drops) == 2 &&
              atomic_load(&c.releases) == 1);
        if (args[2].result == MSG_OK)
        {
            CHECK(c.status == MSG_CANCELLED);
        }
        else
        {
            CHECK(c.status == MSG_OK || c.status == MSG_FAILED);
        }
    }
}

static void oracle(void)
{
    char word[16];
    while (scanf("%15s", word) == 1)
    {
        struct msg_counts c;
        initialize(&c);
        struct msg_context ctx = {&c, false};
        struct mfx_completion *root = NULL;
        CHECK(mfx_create(&root, receiver, &ctx, control_drop, &c, false) ==
              MSG_OK);
        for (size_t i = 0; word[i]; i++)
        {
            enum msg_status s;
            if (word[i] == 'c')
            {
                s = mfx_cancel(root);
            }
            else
            {
                struct mfo_value owner = {&c, payload_drop};
                s = mfx_complete(root, word[i] == 'a' ? MSG_OK : MSG_FAILED,
                                 &owner);
                mfo_drop(&owner);
            }
            printf("%u,", (unsigned)s);
        }
        mfx_drop(&root);
        printf("|%u,%u,%u,%u\n", atomic_load(&c.calls), atomic_load(&c.drops),
               atomic_load(&c.releases), (unsigned)c.status);
    }
}

int main(int argc, char **argv)
{
    if (argc == 2 && !strcmp(argv[1], "oracle"))
    {
        oracle();
        return 0;
    }
    if (argc != 1)
    {
        return 2;
    }
    boundaries();
    races();
    puts("Retained completion winner and cancellation request checks passed");
    return 0;
}
