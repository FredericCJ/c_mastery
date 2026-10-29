#include "resize.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define CHECK(x)                                                            \
    do                                                                      \
    {                                                                       \
        if (!(x))                                                           \
        {                                                                   \
            fprintf(stderr, "LP_CHECK %s:%d %s\n", __FILE__, __LINE__, #x); \
            return 1;                                                       \
        }                                                                   \
    } while (0)

struct lp_track
{
    unsigned char *active;
    size_t size;
    unsigned calls, zero, releases, fail, policy;
    struct lr_buffer *owner;
    unsigned empty_at_release;
};
static struct lp_track state;

static void cleanup(void)
{
    free(state.active);
    state = (struct lp_track){0};
}

/* Both zero policies are original teaching adapters, not claims about libc.
   Nonzero implementation uses actual malloc/copy/free and scheduled refusal. */
static void *allocate(void *ctx, void *old, size_t old_size, size_t size)
{
    struct lp_track *t = ctx;
    t->calls++;
    if (!size)
    {
        t->zero++;
        if (t->policy)
        {
            free(t->active);
            t->active = NULL;
            t->size = 0;
        }
        return NULL;
    }
    if (t->fail)
    {
        t->fail = 0;
        return NULL;
    }
    if (old != t->active || old_size != t->size)
    {
        abort();
    }
    unsigned char *next = malloc(size);
    if (!next)
    {
        return NULL;
    }
    for (size_t i = 0; i < size; i++)
    {
        next[i] = 0xa5;
    }
    for (size_t i = 0; i < old_size && i < size; i++)
    {
        next[i] = t->active[i];
    }
    free(t->active);
    t->active = next;
    t->size = size;
    return next;
}

static void release(void *ctx, void *old, size_t size)
{
    struct lp_track *t = ctx;
    if (old != t->active || size != t->size)
    {
        abort();
    }
    t->empty_at_release += t->owner->data == NULL && t->owner->size == 0;
    t->releases++;
    free(old);
    t->active = NULL;
    t->size = 0;
}

static struct lr_allocator adapter(void)
{
    return (struct lr_allocator){
        .context = &state, .resize = allocate, .release = release};
}

static int zero(void)
{
    for (unsigned policy = 0; policy < 2; policy++)
    {
        struct lr_buffer b;
        state.policy = policy;
        state.owner = &b;
        CHECK(lr_init(&b, adapter()) == LR_OK);
        CHECK(lr_resize(&b, 0) == LR_INVALID && state.calls == 0 && !b.data &&
              b.size == 0);
        CHECK(lr_resize(&b, 3) == LR_OK);
        unsigned char *old = b.data;
        b.data[0] = 17;
        CHECK(lr_resize(&b, 0) == LR_INVALID && b.data == old && b.size == 3 &&
              b.data[0] == 17 && state.zero == 0 && state.calls == 1);
        CHECK(lr_resize(&b, 65) == LR_INVALID &&
              lr_resize(&b, (size_t)-1) == LR_INVALID && state.calls == 1);
        lr_release(&b);
        CHECK(state.releases == 1 && state.empty_at_release == 1);
        cleanup();
        /* Establish the different original adapter policies independently.
           Never call libc realloc with zero or inspect a freed pointer. */
        state.policy = policy;
        unsigned char *raw = allocate(&state, NULL, 0, 1);
        CHECK(raw != NULL);
        raw[0] = 19;
        CHECK(allocate(&state, raw, 1, 0) == NULL && state.zero == 1);
        if (policy)
        {
            CHECK(state.active == NULL);
        }
        else
        {
            CHECK(state.active == raw && raw[0] == 19);
        }
        cleanup();
    }
    puts("LP zero validated");
    return 0;
}

static int failure(void)
{
    struct lr_buffer b;
    state.owner = &b;
    CHECK(lr_init(&b, adapter()) == LR_OK && lr_resize(&b, 4) == LR_OK);
    unsigned char *old = b.data;
    for (size_t i = 0; i < 4; i++)
    {
        b.data[i] = (unsigned char)(i + 7);
    }
    state.fail = 1;
    CHECK(lr_resize(&b, 8) == LR_NOMEM && b.data == old && b.size == 4);
    for (size_t i = 0; i < 4; i++)
    {
        CHECK(b.data[i] == i + 7);
    }
    lr_release(&b);
    puts("LP failure validated");
    return 0;
}

static int drop(void)
{
    struct lr_buffer b;
    state.owner = &b;
    CHECK(lr_init(&b, adapter()) == LR_OK && lr_resize(&b, 2) == LR_OK);
    lr_release(&b);
    CHECK(!b.data && b.size == 0 && state.releases == 1 &&
          state.empty_at_release == 1);
    lr_release(&b);
    CHECK(state.releases == 1);
    CHECK(lr_resize(&b, 1) == LR_OK);
    lr_release(&b);
    CHECK(state.releases == 2);
    puts("LP drop validated");
    return 0;
}

static int growth(void)
{
    struct lr_buffer b;
    state.owner = &b;
    CHECK(lr_init(&b, adapter()) == LR_OK && lr_resize(&b, 2) == LR_OK);
    CHECK(b.data[0] == 0 && b.data[1] == 0);
    b.data[0] = 9;
    b.data[1] = 11;
    CHECK(lr_resize(&b, 5) == LR_OK && b.data[0] == 9 && b.data[1] == 11);
    for (size_t i = 2; i < 5; i++)
    {
        CHECK(b.data[i] == 0);
    }
    CHECK(lr_resize(&b, 1) == LR_OK && b.data[0] == 9);
    lr_release(&b);
    puts("LP growth validated");
    return 0;
}

static int oracle(void)
{
    unsigned policy;
    char history[8];
    while (scanf("%u %7s", &policy, history) == 2)
    {
        struct lr_buffer b;
        state.policy = policy;
        state.owner = &b;
        CHECK(lr_init(&b, adapter()) == LR_OK);
        enum lr_status status = LR_OK;
        for (size_t i = 0; history[i]; i++)
        {
            char op = history[i];
            if (op == '-')
            {
                continue;
            }
            if (op == 'a')
            {
                status = lr_resize(&b, 1);
            }
            else if (op == 'b')
            {
                status = lr_resize(&b, 2);
            }
            else if (op == 'c')
            {
                status = lr_resize(&b, 4);
            }
            else if (op == 'f')
            {
                state.fail = 1;
                status = lr_resize(&b, 3);
            }
            else if (op == 'z')
            {
                status = lr_resize(&b, 0);
            }
            else if (op == 'd')
            {
                lr_release(&b);
                status = LR_OK;
            }
            else if (op == 'w')
            {
                if (b.size)
                {
                    b.data[0] = 23;
                }
                status = LR_OK;
            }
            else
            {
                CHECK(0);
            }
        }
        unsigned sum = 0;
        for (size_t i = 0; i < b.size; i++)
        {
            sum += b.data[i];
        }
        printf("%d %zu %u %u %u %u\n", status, b.size, sum, state.calls,
               state.releases, state.zero);
        lr_release(&b);
        cleanup();
    }
    return 0;
}

int main(int argc, char **argv)
{
    CHECK(argc == 2);
    int result = 2;
    if (!strcmp(argv[1], "zero"))
    {
        result = zero();
    }
    if (!strcmp(argv[1], "failure"))
    {
        result = failure();
    }
    if (!strcmp(argv[1], "drop"))
    {
        result = drop();
    }
    if (!strcmp(argv[1], "growth"))
    {
        result = growth();
    }
    if (!strcmp(argv[1], "oracle"))
    {
        result = oracle();
    }
    if (!strcmp(argv[1], "metrics"))
    {
        printf(
            "{\"control_bytes\":%zu,\"size_limit\":64,\"zero_policies\":2}\n",
            sizeof(struct lr_buffer));
        result = 0;
    }
    cleanup();
    return result;
}
