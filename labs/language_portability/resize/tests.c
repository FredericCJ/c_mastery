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

/* Test double standing behind the allocator interface. Exactly one block is
   live at a time (active, size in bytes). A non-zero resize or a release that
   names a different block aborts, which catches a stale or double use; a
   zero-size request is handled by the zero policy without that check. */
struct lp_track
{
    unsigned char *active;
    size_t size;
    unsigned calls;
    unsigned zero;
    unsigned releases;
    unsigned fail; /* 1: refuse the next non-zero request */
    unsigned policy; /* 0: zero request keeps the block; 1: frees it */
    struct lr_buffer *owner; /* buffer observed during release */
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

/* The buffer refuses zero and oversize requests without calling the
   allocator, whatever the adapter's zero policy. */
static int zero_through_buffer(unsigned policy)
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
    return 0;
}

/* Establish the different original adapter policies independently. Never
   call libc realloc with zero or inspect a freed pointer. */
static int zero_through_adapter(unsigned policy)
{
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
    return 0;
}

static int zero(void)
{
    for (unsigned policy = 0; policy < 2; policy++)
    {
        if (zero_through_buffer(policy) || zero_through_adapter(policy))
        {
            return 1;
        }
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

/* Apply one history letter to b and return the status it produced, or -1 for
   an unknown letter. a/b/c grow or shrink to 1/2/4 bytes, f resizes to 3
   after scheduling an allocator refusal, z asks for zero bytes, d releases,
   w writes a marker into the first byte. */
static int apply_history_op(struct lr_buffer *b, char op)
{
    switch (op)
    {
        case 'a':
            return lr_resize(b, 1);
        case 'b':
            return lr_resize(b, 2);
        case 'c':
            return lr_resize(b, 4);
        case 'f':
            state.fail = 1;
            return lr_resize(b, 3);
        case 'z':
            return lr_resize(b, 0);
        case 'd':
            lr_release(b);
            return LR_OK;
        case 'w':
            if (b->size)
            {
                b->data[0] = 23;
            }
            return LR_OK;
        default:
            return -1;
    }
}

/* Read "policy history" lines and print one result line per history. The
   history is at most 7 letters; '-' is a no-op separator. */
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
            if (history[i] == '-')
            {
                continue;
            }
            int step = apply_history_op(&b, history[i]);
            CHECK(step >= 0);
            status = (enum lr_status)step;
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

static int metrics(void)
{
    printf("{\"control_bytes\":%zu,\"size_limit\":64,\"zero_policies\":2}\n",
           sizeof(struct lr_buffer));
    return 0;
}

struct lp_case
{
    const char *name;
    int (*run)(void);
};

static const struct lp_case cases[] = {
    {"zero", zero},     {"failure", failure}, {"drop", drop},
    {"growth", growth}, {"oracle", oracle},   {"metrics", metrics}};

/* Exit status: 0 pass, 1 failed check, 2 unknown case name. */
int main(int argc, char **argv)
{
    CHECK(argc == 2);
    int result = 2;
    for (size_t i = 0; i < sizeof cases / sizeof cases[0]; i++)
    {
        if (!strcmp(argv[1], cases[i].name))
        {
            result = cases[i].run();
        }
    }
    cleanup();
    return result;
}
