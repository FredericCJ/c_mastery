#include "registry.h"
#include <stdio.h>
#include <stdlib.h>
#define CHECK(x)                                                     \
    do                                                               \
    {                                                                \
        if (!(x))                                                    \
        {                                                            \
            fprintf(stderr, "REGISTRY-CHECK:%d:%s\n", __LINE__, #x); \
            return 1;                                                \
        }                                                            \
    } while (0)

struct ac_log
{
    unsigned calls[32], length, drops, errors, terminals, free_terminal;
};

struct ac_context
{
    struct ac_log *log;
    unsigned tag, mode, owned;
    struct api_handle other;
    struct ac_context *added;
};

static void drop(void *ctx)
{
    struct ac_context *c = ctx;
    ++c->log->drops;
    if (c->owned)
    {
        free(c);
    }
}

static void callback(void *ctx, struct as_registry *r, struct api_handle own)
{
    struct ac_context *c = ctx;
    struct ac_log *l = c->log;
    if (c->mode == 6)
    {
        ++c->tag;
    }
    l->calls[l->length++] = c->tag;
    if (c->mode == 1 && as_remove(r, own) != API_PENDING)
    {
        ++l->errors;
    }
    if (c->mode == 2)
    {
        if (as_remove(r, c->other) != API_PENDING)
        {
            ++l->errors;
        }
        c->mode = 0;
    }
    if (c->mode == 3)
    {
        struct api_handle h;
        if (as_register(r, callback, c->added, drop, &h) != API_OK)
        {
            ++l->errors;
        }
        c->mode = 0;
    }
    if (c->mode == 4 && as_dispatch(r) != API_BUSY)
    {
        ++l->errors;
    }
    if (c->mode == 5)
    {
        if (as_destroy(r) != API_PENDING || as_destroy(r) != API_PENDING ||
            l->drops)
        {
            ++l->errors;
        }
    }
}

static void terminal(void *ctx, struct as_registry *r)
{
    struct ac_log *l = ctx;
    ++l->terminals;
    if (!r->core.closed || l->drops != 2)
    {
        ++l->errors;
    }
    if (l->free_terminal)
    {
        free(r);
    }
}

int main(void)
{
    unsigned histories = 0;
    for (unsigned policy = 0; policy < 2; ++policy)
    {
        for (unsigned mode = 1; mode < 5; ++mode)
        {
            struct as_registry r;
            as_registry_init(&r, policy, 0, 0);
            struct ac_log l = {0};
            struct ac_context a = {&l, 1, mode, 0, {0, 0}, 0},
                              b = {&l, 2, 0, 0, {0, 0}, 0},
                              c = {&l, 3, 0, 0, {0, 0}, 0};
            struct api_handle ha, hb;
            a.added = &c;
            CHECK(as_register(&r, callback, &a, drop, &ha) == API_OK);
            CHECK(as_register(&r, callback, &b, drop, &hb) == API_OK);
            a.other = hb;
            CHECK(as_dispatch(&r) == API_OK && !l.errors);
            CHECK(l.calls[0] == 1);
            CHECK(l.length == (policy && mode == 2 ? 1u : 2u));
            if (l.length == 2)
            {
                CHECK(l.calls[1] == 2);
            }
            if (mode == 1 || mode == 2)
            {
                CHECK(l.drops == 1);
            }
            else
            {
                CHECK(l.drops == 0);
            }
            unsigned start = l.length;
            CHECK(as_dispatch(&r) == API_OK);
            if (mode == 1)
            {
                CHECK(l.length == start + 1 && l.calls[start] == 2);
            }
            if (mode == 3)
            {
                CHECK(l.length == start + 3 && l.calls[start + 2] == 3);
            }
            CHECK(as_destroy(&r) == API_OK && l.drops == (mode == 3 ? 3u : 2u));
            CHECK(as_destroy(&r) == API_OK && !l.errors);
            ++histories;
        }
    }
    for (unsigned heap = 0; heap < 2; ++heap)
    {
        struct as_registry *r = malloc(sizeof *r);
        CHECK(r);
        struct ac_log l = {0};
        l.free_terminal = heap;
        as_registry_init(r, 0, terminal, &l);
        struct ac_context *a = malloc(sizeof *a), *b = malloc(sizeof *b);
        CHECK(a && b);
        *a = (struct ac_context){&l, 1, 5, 1, {0, 0}, 0};
        *b = (struct ac_context){&l, 2, 0, 1, {0, 0}, 0};
        struct api_handle ha, hb;
        CHECK(as_register(r, callback, a, drop, &ha) == API_OK &&
              as_register(r, callback, b, drop, &hb) == API_OK);
        CHECK(as_dispatch(r) == API_CLOSED);
        /* If heap==1, r is already freed: inspect only the independent receiver log. */
        CHECK(l.length == 1 && l.calls[0] == 1 && l.drops == 2 &&
              l.terminals == 1 && !l.errors);
        if (!heap)
        {
            CHECK(as_destroy(r) == API_OK && l.terminals == 1);
            free(r);
        }
        ++histories;
    }
    struct as_registry r;
    as_registry_init(&r, 0, 0, 0);
    struct ac_log l = {0};
    struct ac_context c[5];
    struct api_handle h;
    for (unsigned i = 0; i < 5; ++i)
    {
        c[i] = (struct ac_context){&l, i, 0, 0, {0, 0}, 0};
        CHECK(as_register(&r, callback, c + i, drop, &h) ==
              (i < 4 ? API_OK : API_FULL));
    }
    CHECK(as_dispatch(&r) == API_OK && l.length == 4 &&
          as_destroy(&r) == API_OK && l.drops == 4);
    /* Same code, separate contexts; callback/context is the behavioral pair. */
    as_registry_init(&r, 0, 0, 0);
    l = (struct ac_log){0};
    c[0] = (struct ac_context){&l, 0, 6, 0, {0, 0}, 0};
    c[1] = (struct ac_context){&l, 10, 6, 0, {0, 0}, 0};
    CHECK(as_register(&r, callback, c, 0, &h) == API_OK &&
          as_register(&r, callback, c + 1, 0, &h) == API_OK);
    CHECK(as_dispatch(&r) == API_OK && l.calls[0] == 1 && l.calls[1] == 11 &&
          as_destroy(&r) == API_OK);
    printf(
        "REGISTRY-PASS:histories=%u;terminal_free=1;capacity=4;policy_count=2\n",
        histories);
    return 0;
}
