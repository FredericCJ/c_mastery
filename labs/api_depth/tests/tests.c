#include "api_depth.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define CHECK(x)                                                      \
    do                                                                \
    {                                                                 \
        if (!(x))                                                     \
        {                                                             \
            fprintf(stderr, "API_CHECK: line%d: %s\n", __LINE__, #x); \
            exit(1);                                                  \
        }                                                             \
    } while (0)

static void append(void)
{
    const unsigned char source[] = {3, 4};
    struct api_buffer b = {{1, 2}, 2, 3};
    CHECK(api_append(&b, source, 2) == API_FULL && b.length == 2 &&
          b.data[0] == 1 && b.data[1] == 2);
    b.capacity = 4;
    CHECK(api_append(&b, source, 2) == API_OK && b.length == 4 &&
          b.data[2] == 3 && b.data[3] == 4);
    b.capacity = 32;
    CHECK(api_append(&b, b.data, 2) == API_OVERLAP && b.length == 4);
    CHECK(api_append(&b, (const unsigned char *)&b.length, 1) == API_OVERLAP &&
          b.length == 4);
    CHECK(api_append(&b, NULL, 0) == API_OK && b.length == 4);
    CHECK(api_append(&b, NULL, 1) == API_INVALID && b.length == 4);
    CHECK(api_append(&b, source, SIZE_MAX) == API_FULL && b.length == 4);
    b.length = 33;
    CHECK(api_append(&b, source, 1) == API_INVALID && b.length == 33);
    /* Exhaustive scalar admission over valid32-byte extents, with full state checks. */
    unsigned char input[32];
    for (unsigned i = 0; i < 32; i++)
        input[i] = (unsigned char)(i + 1);
    for (size_t cap = 0; cap <= 32; cap++)
        for (size_t len = 0; len <= cap; len++)
            for (size_t n = 0; n <= 32; n++)
            {
                struct api_buffer value;
                value.capacity = cap;
                value.length = len;
                for (unsigned i = 0; i < 32; i++)
                    value.data[i] = (unsigned char)(200 + i);
                enum api_status status = api_append(&value, input, n);
                CHECK(status == (n <= cap - len ? API_OK : API_FULL));
                CHECK(value.capacity == cap &&
                      value.length == (n <= cap - len ? len + n : len));
                for (size_t i = 0; i < 32; i++)
                {
                    unsigned char expected =
                        (n <= cap - len && i >= len && i < len + n)
                            ? input[i - len]
                            : (unsigned char)(200 + i);
                    CHECK(value.data[i] == expected);
                }
            }
}
struct ac_context;

struct ac_context
{
    unsigned calls, drops, action;
    struct api_registry *registry;
    struct api_handle other;
    struct ac_context *added;
};

static void callback(void *, struct api_registry *, struct api_handle);

static void drop(void *p)
{
    struct ac_context *c = p;
    c->drops++;
    CHECK(api_dispatch(c->registry) == API_BUSY);
    CHECK(api_destroy(c->registry) == API_BUSY);
}

static void callback(void *p, struct api_registry *r, struct api_handle self)
{
    struct ac_context *c = p;
    c->calls++;
    CHECK(c->drops == 0);
    CHECK(api_dispatch(r) == API_BUSY);
    if (c->action == 1)
    {
        CHECK(api_remove(r, self) == API_PENDING);
        CHECK(c->drops == 0);
    }
    else if (c->action == 2)
    {
        CHECK(api_remove(r, c->other) == API_PENDING);
    }
    else if (c->action == 3)
    {
        CHECK(api_destroy(r) == API_PENDING);
        CHECK(c->drops == 0);
    }
    else if (c->action == 4)
    {
        struct api_handle h;
        CHECK(api_add(r, callback, c->added, drop, &h) == API_OK);
        c->action = 0;
    }
}

static void registry(void)
{
    for (unsigned mode = 1; mode <= 4; mode++)
    {
        struct api_registry r;
        api_registry_init(&r);
        struct ac_context a = {0, 0, mode, &r, {0, 0}, NULL},
                          b = {0, 0, 0, &r, {0, 0}, NULL},
                          c = {0, 0, 0, &r, {0, 0}, NULL};
        struct api_handle ha, hb;
        a.added = &c;
        CHECK(api_add(&r, callback, &a, drop, &ha) == API_OK);
        CHECK(api_add(&r, callback, &b, drop, &hb) == API_OK);
        a.other = hb;
        CHECK(api_dispatch(&r) == (mode == 3 ? API_CLOSED : API_OK));
        CHECK(a.calls == 1 && b.calls == (mode == 3 ? 0u : 1u));
        CHECK(a.drops == ((mode == 1 || mode == 3) ? 1u : 0u));
        CHECK(b.drops == ((mode == 2 || mode == 3) ? 1u : 0u));
        if (mode == 4)
        {
            CHECK(c.calls == 0);
            CHECK(api_dispatch(&r) == API_OK);
            CHECK(c.calls == 1);
        }
        CHECK(api_destroy(&r) == API_OK);
        CHECK(api_destroy(&r) == API_OK);
        CHECK(a.drops == 1 && b.drops == 1 && c.drops == (mode == 4 ? 1u : 0u));
        struct api_handle sentinel = {99, 99};
        CHECK(api_add(&r, callback, &a, drop, &sentinel) == API_CLOSED &&
              sentinel.index == 99);
    }
    struct api_registry r;
    api_registry_init(&r);
    struct ac_context c[5];
    struct api_handle h[5];
    for (unsigned i = 0; i < 5; i++)
    {
        c[i] = (struct ac_context){0, 0, 0, &r, {0, 0}, NULL};
        h[i] = (struct api_handle){99, 99};
    }
    for (unsigned i = 0; i < 4; i++)
        CHECK(api_add(&r, callback, c + i, drop, h + i) == API_OK);
    CHECK(api_add(&r, callback, c + 4, drop, h + 4) == API_FULL &&
          h[4].index == 99 && c[4].drops == 0);
    struct api_handle old = h[0];
    CHECK(api_remove(&r, old) == API_OK);
    CHECK(c[0].drops == 1);
    CHECK(api_add(&r, callback, c + 4, drop, h + 4) == API_OK &&
          h[4].generation != old.generation);
    CHECK(api_remove(&r, old) == API_STALE && !c[4].drops);
    CHECK(api_destroy(&r) == API_OK);
    api_registry_init(&r);
    for (unsigned i = 0; i < 4; i++)
        r.slots[i].generation = UINT32_MAX;
    h[0] = (struct api_handle){99, 99};
    CHECK(api_add(&r, callback, c, drop, h) == API_FULL && h[0].index == 99);
    CHECK(api_remove(&r, (struct api_handle){4, 1}) == API_INVALID);
}

int main(int argc, char **argv)
{
    if (argc != 2)
        return 2;
    if (!strcmp(argv[1], "append"))
        append();
    else if (!strcmp(argv[1], "registry"))
        registry();
    else
        return 2;
    return 0;
}
