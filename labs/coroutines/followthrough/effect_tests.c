#include "effect_follow.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define C(x)                                                   \
    do                                                         \
    {                                                          \
        if (!(x))                                              \
        {                                                      \
            fprintf(stderr, "CH_CHECK:%d:%s\n", __LINE__, #x); \
            return 1;                                          \
        }                                                      \
    } while (0)

struct co_alloc
{
    unsigned live, calls, fail;
};

static void *allocate(void *v, size_t n)
{
    struct co_alloc *a = v;
    if (++a->calls == a->fail)
    {
        return NULL;
    }
    void *p = malloc(n);
    if (p)
    {
        a->live++;
    }
    return p;
}

static void release(void *v, void *p)
{
    ((struct co_alloc *)v)->live--;
    free(p);
}

struct co_answers
{
    uint32_t a, b;
    unsigned wait;
};

static int answer(void *v, unsigned question, uint32_t *out)
{
    struct co_answers *a = v;
    if (a->wait)
    {
        a->wait--;
        return CO_PENDING;
    }
    *out = question == 1 ? a->a : a->b;
    return CO_OK;
}

int main(void)
{
    struct co_answers outer = {10, 20, 0}, inner = {0, 7, 0},
                      other = {100, 9, 0};
    struct ch_handler base = {3, answer, &outer, NULL},
                      nested = {2, answer, &inner, &base},
                      dynamic = {3, answer, &other, NULL};
    unsigned char bytes[] = {1, 2, 3, 4};
    for (unsigned mode = 0; mode < 2; mode++)
    {
        struct co_alloc a = {0};
        struct ch_effect *f = NULL;
        C(ch_create((struct co_allocator){&a, allocate, release}, &nested, mode,
                    bytes, &f) == 0 &&
          a.live == 2);
        uint32_t out = 91;
        C(ch_step(f, &dynamic, &out) == CO_REQUEST && out == 91 && a.live == 2);
        C(ch_step(f, &dynamic, &out) == CO_DONE && out == (mode ? 109u : 17u) &&
          a.live == 1);
        C(ch_destroy(&f) == 0 && !a.live);
    }
    struct co_alloc a = {0};
    struct ch_effect *f = NULL, *copy = NULL;
    C(ch_create((struct co_allocator){&a, allocate, release}, &nested, 1, bytes,
                &f) == 0);
    uint32_t out = 91;
    C(ch_step(f, &base, &out) == CO_REQUEST);
    C(ch_clone(f, &copy) == 0 && copy->resource != f->resource &&
      copy->token.owner == &copy->core &&
      !memcmp(copy->resource, f->resource, 4) && a.live == 4);
    copy->resource[0] = 99;
    C(f->resource[0] == 1);
    C(ch_step(f, &nested, &out) == CO_DONE && out == 17);
    C(ch_step(copy, &dynamic, &out) == CO_DONE && out == 19);
    C(ch_destroy(&f) == 0 && ch_destroy(&copy) == 0 && !a.live);
    for (unsigned stage = 0; stage < 2; stage++)
    {
        a = (struct co_alloc){0};
        C(ch_create((struct co_allocator){&a, allocate, release}, &nested, 0,
                    bytes, &f) == 0);
        if (stage)
        {
            C(ch_step(f, NULL, &out) == CO_REQUEST);
        }
        C(ch_abort(f) == 0 && a.live == 1 && !f->resource);
        C(ch_abort(f) == 0);
        C(ch_destroy(&f) == 0 && !a.live);
    }
    a = (struct co_alloc){0};
    C(ch_create((struct co_allocator){&a, allocate, release}, &nested, 0, bytes,
                &f) == 0);
    C(ch_step(f, NULL, &out) == CO_REQUEST);
    for (unsigned offset = 1; offset <= 2; offset++)
    {
        a.fail = a.calls + offset;
        copy = NULL;
        C(ch_clone(f, &copy) == CO_RANGE && !copy && a.live == 2);
    }
    a.fail = 0;
    inner.wait = 1;
    out = 91;
    C(ch_step(f, NULL, &out) == CO_PENDING && out == 91 && a.live == 2);
    inner.b = UINT32_MAX;
    C(ch_step(f, NULL, &out) == CO_RANGE && out == 91 && a.live == 2);
    inner.b = 7;
    C(ch_step(f, NULL, &out) == CO_DONE && out == 17);
    C(ch_destroy(&f) == 0 && !a.live);
    puts(
        "CH_HANDLERS:lexical/dynamic nesting;owned suspension;deep clone branches;allocation rollback;abort;overflow retry");
    return 0;
}
