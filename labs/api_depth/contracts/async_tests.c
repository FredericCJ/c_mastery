#include "async.h"
#include "api_depth.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define CHECK(x)                                                  \
    do                                                            \
    {                                                             \
        if (!(x))                                                 \
        {                                                         \
            fprintf(stderr, "ASYNC-CHECK:%d:%s\n", __LINE__, #x); \
            return 1;                                             \
        }                                                         \
    } while (0)

struct ac_memory
{
    unsigned attempts, allocations, frees, fail;
};

static void *allocate(void *context, size_t n, size_t alignment)
{
    struct ac_memory *m = context;
    ++m->attempts;
    if (m->fail || alignment > _Alignof(max_align_t))
    {
        return 0;
    }
    void *p = malloc(n);
    if (p)
    {
        ++m->allocations;
    }
    return p;
}

static void release(void *context, void *p)
{
    struct ac_memory *m = context;
    ++m->frees;
    free(p);
}

struct ac_receiver
{
    struct ac_operation *operation;
    struct ac_memory *memory;
    unsigned calls, errors;
    unsigned entering, inline_seen;
    enum ac_async_status result;
    struct ac_box *saved;
    unsigned keep;
};

static void receive(void *context, enum ac_async_status result,
                    struct ac_box *box)
{
    struct ac_receiver *r = context;
    ++r->calls;
    r->inline_seen = r->entering;
    r->result = result;
    if (r->operation->core.state != F_DONE || r->operation->box ||
        r->operation->terminal || !r->operation->reporting)
    {
        ++r->errors;
    }
    if (ac_cancel_v2(r->operation, r->operation->core.generation) !=
        ACX_INVALID)
    {
        ++r->errors;
    }
    if (result == ACX_OK)
    {
        size_t n = 99;
        const unsigned char *p = ac_box_data(box, &n);
        if (!p || n != 3 || p[0] != 'a' || p[1] != 'b' || p[2] != 'c')
        {
            ++r->errors;
        }
        if (r->keep)
        {
            r->saved = box;
        }
        else
        {
            ac_box_drop(box);
        }
    }
    else if (box)
    {
        ++r->errors;
    }
}

struct ac_sync
{
    unsigned calls;
    unsigned char data[8];
    struct ac_memory *memory;
};

static void sync_copy(void *context, const unsigned char *p, size_t n)
{
    struct ac_sync *s = context;
    ++s->calls;
    for (size_t i = 0; i < n; ++i)
    {
        s->data[i] = p[i];
    }
}

static void allocating_log(void *context, const unsigned char *p, size_t n)
{
    struct ac_sync *s = context;
    void *copy = allocate(s->memory, n ? n : 1, 1);
    if (copy)
    {
        memcpy(copy, p, n);
        release(s->memory, copy);
    }
    sync_copy(context, p, n);
}

struct ac_scope
{
    unsigned references, parents, children, drops, destroy_inside, errors;
    unsigned char *payload;
};

static void scope_drop(void *context)
{
    struct ac_scope *s = context;
    ++s->drops;
    if (!s->references)
    {
        ++s->errors;
        return;
    }
    if (!--s->references)
    {
        free(s->payload);
        s->payload = 0;
    }
}

static void child(void *context, struct api_registry *registry,
                  struct api_handle handle)
{
    struct ac_scope *s = context;
    ++s->children;
    if (!s->payload || *s->payload != 73 ||
        api_remove(registry, handle) != API_PENDING)
    {
        ++s->errors;
    }
}

static void parent(void *context, struct api_registry *registry,
                   struct api_handle handle)
{
    struct ac_scope *s = context;
    struct api_handle descendant;
    ++s->parents;
    ++s->references;
    if (api_add(registry, child, s, scope_drop, &descendant) != API_OK)
    {
        scope_drop(s);
        ++s->errors;
    }
    if (api_remove(registry, handle) != API_PENDING || !s->payload || s->drops)
    {
        ++s->errors;
    }
    if (s->destroy_inside && api_destroy(registry) != API_PENDING)
    {
        ++s->errors;
    }
    if (s->drops || !s->payload)
    {
        ++s->errors; /* Current callback still owns. */
    }
}

static int scope_cases(void)
{
    for (unsigned close = 0; close < 3; ++close)
    {
        struct api_registry registry;
        api_registry_init(&registry);
        struct ac_scope s = {0};
        s.references = 1;
        s.destroy_inside = close == 2;
        s.payload = malloc(1);
        CHECK(s.payload);
        *s.payload = 73;
        struct api_handle h;
        CHECK(api_add(&registry, parent, &s, scope_drop, &h) == API_OK);
        CHECK(api_dispatch(&registry) == (close == 2 ? API_CLOSED : API_OK));
        CHECK(s.parents == 1 && !s.children && !s.errors);
        if (close == 2)
        {
            CHECK(s.drops == 2 && !s.payload);
        }
        else
        {
            CHECK(s.drops == 1 && s.references == 1 && s.payload);
            if (close == 1)
            {
                CHECK(api_destroy(&registry) == API_OK);
            }
            else
            {
                CHECK(api_dispatch(&registry) == API_OK && s.children == 1);
            }
            CHECK(s.drops == 2 && !s.payload && !s.errors);
        }
        CHECK(api_destroy(&registry) == API_OK && s.drops == 2);
    }
    return 0;
}

int main(void)
{
    struct ac_memory m = {0};
    struct ac_allocate allocator = {&m, allocate, release};
    unsigned char input[3] = {'A', 'B', 'C'};
    struct ac_sync sync = {0};
    sync.memory = &m;
    size_t written = 17;
    CHECK(ac_transform_v1(input, 3, sync_copy, &sync, &written) == 0);
    CHECK(sync.calls == 1 && written == 3 && sync.data[0] == 'a' &&
          !m.attempts);
    written = 17;
    CHECK(ac_transform_v1(input, 9, sync_copy, &sync, &written) == 1 &&
          written == 17);
    CHECK(ac_transform_v1(input, 3, allocating_log, &sync, &written) == 0);
    CHECK(m.attempts == 1 && m.allocations == 1 && m.frees == 1);
    CHECK(scope_cases() == 0);
    for (unsigned in = 0; in < 2; ++in)
    {
        struct ac_operation op;
        ac_operation_init(&op);
        struct ac_receiver r = {0};
        r.operation = &op;
        r.memory = &m;
        r.entering = 1;
        r.keep = 1;
        uint32_t ticket = 99;
        m.fail = 1;
        unsigned before = m.frees;
        CHECK(ac_submit_v2(&op, input, 3, allocator, receive, &r, in,
                           &ticket) == ACX_ALLOC);
        CHECK(ticket == 99 && op.core.state == F_IDLE && !r.calls &&
              m.frees == before);
        m.fail = 0;
        CHECK(ac_submit_v2(&op, input, 3, allocator, receive, &r, in,
                           &ticket) == ACX_OK);
        CHECK(ticket == 1 && r.calls == in && r.inline_seen == in);
        r.entering = 0;
        if (!in)
        {
            CHECK(ac_submit_v2(&op, input, 3, allocator, receive, &r, 0,
                               &ticket) == ACX_BUSY);
            input[0] = 'Z'; /* The operation copied before returning. */
            CHECK(ac_finish_v2(&op, ticket, 0) == ACX_OK);
            input[0] = 'A';
        }
        CHECK(r.calls == 1 && !r.errors && r.saved && op.core.releases == 1);
        size_t n;
        CHECK(ac_box_data(r.saved, &n)[0] == 'a' && n == 3);
        CHECK(ac_finish_v2(&op, ticket, 0) == ACX_INVALID);
        ac_box_drop(r.saved);
        r.saved = 0;
        r.keep = 0;
        CHECK(ac_submit_v2(&op, input, 3, allocator, receive, &r, 0, &ticket) ==
                  ACX_OK &&
              ticket == 2);
        CHECK(ac_finish_v2(&op, 1, 0) == ACX_INVALID);
        CHECK(ac_cancel_v2(&op, ticket) == ACX_PENDING && r.calls == 1);
        CHECK(ac_finish_v2(&op, ticket, 1) == ACX_OK && r.calls == 2 &&
              r.result == ACX_CANCELLED);
        CHECK(op.core.releases == 2 && !r.errors);
        op.core.generation = UINT32_MAX;
        CHECK(ac_submit_v2(&op, input, 3, allocator, receive, &r, 0, &ticket) ==
              ACX_EXHAUSTED);
        op.core.generation = 2;
        op.core.releases = UINT32_MAX;
        CHECK(ac_submit_v2(&op, input, 3, allocator, receive, &r, 0, &ticket) ==
              ACX_EXHAUSTED);
    }
    CHECK(m.allocations == m.frees);
    puts(
        "ASYNC-PASS:scope_cases=3;completion_modes=2;effects=transitive;ownership=balanced");
    return 0;
}
