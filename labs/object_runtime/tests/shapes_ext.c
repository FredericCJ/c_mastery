#include "object_runtime_shapes.h"
#include <limits.h>
#include <stdio.h>
#include <string.h>
#define CHECK(c)                                               \
    do                                                         \
    {                                                          \
        if (!(c))                                              \
        {                                                      \
            fprintf(stderr, "OR_CHECK:%d:%s\n", __LINE__, #c); \
            return 1;                                          \
        }                                                      \
    } while (0)

static int sid(struct orx_shape_id a, struct orx_shape_id b)
{
    return a.slot == b.slot && a.generation == b.generation;
}

static int shapes(void)
{
    struct orx_world w, before;
    struct orx_handle a, b, c;
    struct orx_shape_id stale;
    struct orx_property_cache cache = {0};
    int32_t value = 777;
    unsigned dead, i, retired = 0;
    enum or_status s;
    orx_world_init(&w);
    CHECK(orx_world_new(&w, &a) == OR_OK);
    CHECK(orx_world_new(&w, &b) == OR_OK);
    CHECK(orx_world_new(&w, &c) == OR_OK);
    CHECK(orx_define(&w, a, 1, 0, 10, NULL, NULL) == OR_OK);
    CHECK(orx_define(&w, b, 1, 0, 20, NULL, NULL) == OR_OK);
    CHECK(sid(w.object[a.slot].shape, w.object[b.slot].shape));
    CHECK(orx_define(&w, a, 2, ORX_READONLY, 30, NULL, NULL) == OR_OK);
    memcpy(&before, &w, sizeof w);
    CHECK(orx_define(&w, a, 2, 0, 40, NULL, NULL) == OR_STATE &&
          !memcmp(&before, &w, sizeof w));
    CHECK(orx_delete(&w, a, 2) == OR_STATE);
    CHECK(orx_prototype(&w, c, a) == OR_OK);
    memcpy(&before, &w, sizeof w);
    CHECK(orx_prototype(&w, a, c) == OR_STATE &&
          !memcmp(&before, &w, sizeof w));
    CHECK(orx_get(&w, c, 2, &cache, &value) == OR_OK && value == 30);
    CHECK(orx_define(&w, c, 2, 0, 99, NULL, NULL) == OR_OK);
    CHECK(orx_get(&w, c, 2, &cache, &value) == OR_OK && value == 99);
    CHECK(orx_delete(&w, c, 2) == OR_OK);
    CHECK(orx_get(&w, c, 2, &cache, &value) == OR_OK && value == 30);
    stale = w.object[b.slot].shape;
    CHECK(orx_delete(&w, b, 1) == OR_OK);
    CHECK(!w.shape[stale.slot].live &&
          w.shape[stale.slot].generation != stale.generation);
    CHECK(orx_define(&w, b, 9, 0, 9, NULL, NULL) == OR_OK);
    CHECK(!sid(w.object[b.slot].shape, stale));
    CHECK(orx_release(&w.heap, a) == OR_OK);
    CHECK(orx_world_collect(&w, &dead) == OR_OK && dead == 0);
    CHECK(orx_prototype(&w, c, (struct orx_handle){0, 0}) == OR_OK);
    CHECK(orx_world_collect(&w, &dead) == OR_OK && dead == 1);
    CHECK(orx_release(&w.heap, b) == OR_OK);
    CHECK(orx_release(&w.heap, c) == OR_OK);
    CHECK(orx_world_collect(&w, &dead) == OR_OK && dead == 2);
    /* Repeated layout changes consume a finite descriptor generation budget. */
    orx_world_init(&w);
    CHECK(orx_world_new(&w, &a) == OR_OK);
    for (i = 0; i < 60; i++)
    {
        s = orx_define(&w, a, 1, 0, 1, NULL, NULL);
        if (s == OR_FULL)
        {
            break;
        }
        CHECK(s == OR_OK);
        CHECK(orx_delete(&w, a, 1) == OR_OK);
    }
    CHECK(i == 45);
    for (i = 1; i < 16; i++)
    {
        retired += w.shape[i].generation == 4;
    }
    CHECK(retired == 15);
    memcpy(&before, &w, sizeof w);
    CHECK(orx_define(&w, a, 2, 0, 2, NULL, NULL) == OR_FULL &&
          !memcmp(&before, &w, sizeof w));
    CHECK(orx_release(&w.heap, a) == OR_OK);
    CHECK(orx_world_collect(&w, &dead) == OR_OK && dead == 1);
    orx_world_init(&w);
    CHECK(orx_world_new(&w, &a) == OR_OK);
    CHECK(orx_world_new(&w, &b) == OR_OK);
    for (i = 0; i < 254; i++)
    {
        CHECK(orx_prototype(&w, a, i % 2 ? (struct orx_handle){0, 0} : b) ==
              OR_OK);
    }
    CHECK(w.epoch == UINT8_MAX);
    memcpy(&before, &w, sizeof w);
    CHECK(orx_prototype(&w, a, b) == OR_FULL && !memcmp(&before, &w, sizeof w));
    dead = 99;
    CHECK(orx_world_collect(&w, &dead) == OR_FULL && dead == 99 &&
          !memcmp(&before, &w, sizeof w));
    CHECK(orx_release(&w.heap, a) == OR_OK && orx_release(&w.heap, b) == OR_OK);
    return 0;
}

struct or_access
{
    unsigned calls;
    enum or_status nested;
    int fail;
    int consume_owned_roots;
    struct orx_handle holder;
    unsigned dead, receiver_live, holder_live;
};

static enum or_status mutate(void *context, struct orx_world *w,
                             struct orx_handle receiver, int32_t *out)
{
    struct or_access *a = context;
    struct orx_property_cache nested = {0};
    int32_t n = 777;
    enum or_status s;
    struct orx_handle temporary;
    a->calls++;
    a->nested = orx_get(w, receiver, 1, &nested, &n);
    s = orx_define(w, receiver, 1, 0, 99, NULL, NULL);
    if (s != OR_OK)
    {
        return s;
    }
    if (a->consume_owned_roots)
    {
        s = orx_prototype(w, receiver, (struct orx_handle){0, 0});
        if (s != OR_OK)
        {
            return s;
        }
        /* The context explicitly owns these original roots, not the temporary
           holds acquired by orx_get. */
        if (orx_release(&w->heap, receiver) != OR_OK ||
            orx_release(&w->heap, a->holder) != OR_OK)
        {
            return OR_STATE;
        }
        a->consume_owned_roots = 0;
    }
    s = orx_world_new(w, &temporary);
    if (s != OR_OK)
    {
        return s;
    }
    s = orx_release(&w->heap, temporary);
    if (s != OR_OK)
    {
        return s;
    }
    s = orx_world_collect(w, &a->dead);
    if (s != OR_OK)
    {
        return s;
    }
    a->receiver_live = w->heap.entry[receiver.slot].live;
    if (a->holder.generation)
    {
        a->holder_live = w->heap.entry[a->holder.slot].live;
    }
    *out = 42;
    return a->fail ? OR_FULL : OR_OK;
}

static int cache(void)
{
    struct orx_world w, other;
    struct orx_handle receiver, a, b, other_receiver;
    struct orx_property_cache c = {0};
    int32_t value = 777;
    struct or_access context = {0};
    unsigned dead;
    orx_world_init(&w);
    CHECK(orx_world_new(&w, &receiver) == OR_OK);
    CHECK(orx_world_new(&w, &a) == OR_OK);
    CHECK(orx_world_new(&w, &b) == OR_OK);
    CHECK(orx_define(&w, a, 1, 0, 10, NULL, NULL) == OR_OK);
    CHECK(orx_define(&w, b, 1, 0, 20, NULL, NULL) == OR_OK);
    CHECK(orx_prototype(&w, receiver, a) == OR_OK);
    CHECK(orx_get(&w, receiver, 1, &c, &value) == OR_OK && value == 10 &&
          c.misses == 1);
    CHECK(orx_get(&w, receiver, 1, &c, &value) == OR_OK && value == 10 &&
          c.hits == 1);
    CHECK(orx_prototype(&w, receiver, b) == OR_OK);
    CHECK(orx_get(&w, receiver, 1, &c, &value) == OR_OK && value == 20 &&
          c.misses == 2);
    CHECK(orx_define(&w, b, 1, 0, 21, NULL, NULL) == OR_OK);
    CHECK(orx_get(&w, receiver, 1, &c, &value) == OR_OK && value == 21 &&
          c.hits == 2);
    CHECK(orx_define(&w, b, 1, ORX_ACCESSOR, 0, mutate, &context) == OR_OK);
    CHECK(orx_get(&w, receiver, 1, &c, &value) == OR_OK && value == 42 &&
          context.calls == 1 && context.nested == OR_STATE && !c.valid &&
          context.dead == 1 && context.receiver_live &&
          w.heap.entry[receiver.slot].roots == 1 &&
          w.heap.entry[b.slot].roots == 1);
    CHECK(orx_get(&w, receiver, 1, &c, &value) == OR_OK && value == 99);
    CHECK(orx_delete(&w, receiver, 1) == OR_OK);
    context.fail = 1;
    value = 777;
    CHECK(orx_get(&w, receiver, 1, &c, &value) == OR_FULL && value == 777 &&
          context.calls == 2 && !c.valid && context.dead == 1 &&
          w.heap.entry[receiver.slot].roots == 1 &&
          w.heap.entry[b.slot].roots == 1);
    CHECK(orx_get(&w, receiver, 1, &c, &value) == OR_OK && value == 99);
    CHECK(orx_release(&w.heap, receiver) == OR_OK);
    CHECK(orx_release(&w.heap, a) == OR_OK);
    CHECK(orx_release(&w.heap, b) == OR_OK);
    CHECK(orx_world_collect(&w, &dead) == OR_OK && dead == 3);
    value = 777;
    CHECK(orx_get(&w, receiver, 1, &c, &value) == OR_INVALID && value == 777);
    orx_world_init(&other);
    CHECK(orx_world_new(&other, &other_receiver) == OR_OK);
    CHECK(orx_define(&other, other_receiver, 1, 0, 88, NULL, NULL) == OR_OK);
    CHECK(orx_get(&other, other_receiver, 1, &c, &value) == OR_OK &&
          value == 88 && c.world == &other && c.misses == 1 && c.hits == 0);
    c.hits = UINT32_MAX;
    value = 777;
    CHECK(orx_get(&other, other_receiver, 1, &c, &value) == OR_FULL &&
          value == 777);
    CHECK(orx_release(&other.heap, other_receiver) == OR_OK);
    CHECK(orx_world_collect(&other, &dead) == OR_OK && dead == 1 &&
          other.shape[0].refs == 0);
    orx_world_init(&w);
    c = (struct orx_property_cache){0};
    CHECK(orx_world_new(&w, &receiver) == OR_OK);
    CHECK(orx_world_new(&w, &b) == OR_OK);
    context =
        (struct or_access){.fail = 1, .consume_owned_roots = 1, .holder = b};
    CHECK(orx_define(&w, b, 1, ORX_ACCESSOR, 0, mutate, &context) == OR_OK);
    CHECK(orx_prototype(&w, receiver, b) == OR_OK);
    value = 777;
    CHECK(orx_get(&w, receiver, 1, &c, &value) == OR_FULL && value == 777 &&
          context.dead == 1 && context.receiver_live && context.holder_live &&
          w.heap.entry[receiver.slot].roots == 0 &&
          w.heap.entry[b.slot].roots == 0);
    CHECK(orx_get(&w, receiver, 1, &c, &value) == OR_OK && value == 99);
    CHECK(orx_world_collect(&w, &dead) == OR_OK && dead == 2);
    return 0;
}

static int oracle(void)
{
    unsigned a, b, c, d, remove, shadow, readonly, k, index = 0;
    for (a = 1; a <= 4; a++)
    {
        for (b = 1; b <= 4; b++)
        {
            for (c = 1; c <= 4; c++)
            {
                for (d = 1; d <= 4; d++)
                {
                    unsigned order[4] = {a, b, c, d};
                    if (a == b || a == c || a == d || b == c || b == d ||
                        c == d)
                    {
                        continue;
                    }
                    for (remove = 0; remove <= 4; remove++)
                    {
                        for (shadow = 0; shadow < 3; shadow++)
                        {
                            for (readonly = 0; readonly < 2; readonly++)
                            {
                                struct orx_world w;
                                struct orx_handle base, child;
                                struct orx_property_cache cache = {0};
                                enum or_status status;
                                unsigned dead;
                                orx_world_init(&w);
                                CHECK(orx_world_new(&w, &base) == OR_OK);
                                CHECK(orx_world_new(&w, &child) == OR_OK);
                                for (k = 0; k < 4; k++)
                                {
                                    CHECK(orx_define(&w, base, order[k],
                                                     readonly && order[k] == 2
                                                         ? ORX_READONLY
                                                         : 0,
                                                     (int32_t)(order[k] * 10),
                                                     NULL, NULL) == OR_OK);
                                }
                                CHECK(orx_prototype(&w, child, base) == OR_OK);
                                if (shadow)
                                {
                                    CHECK(orx_define(&w, child, 1, 0, 91, NULL,
                                                     NULL) == OR_OK);
                                    if (shadow == 2)
                                    {
                                        CHECK(orx_delete(&w, child, 1) ==
                                              OR_OK);
                                    }
                                }
                                status = remove ? orx_delete(&w, base, remove)
                                                : OR_OK;
                                for (k = 1; k <= 5; k++)
                                {
                                    int32_t out = 777;
                                    enum or_status found =
                                        orx_get(&w, child, k, &cache, &out);
                                    printf("S %u %u %u %u %u %d %d %d\n", index,
                                           remove, shadow, readonly, k, status,
                                           found, out);
                                }
                                CHECK(orx_release(&w.heap, base) == OR_OK);
                                CHECK(orx_release(&w.heap, child) == OR_OK);
                                CHECK(orx_world_collect(&w, &dead) == OR_OK &&
                                      dead == 2);
                            }
                        }
                    }
                    index++;
                }
            }
        }
    }
    return 0;
}

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        return 2;
    }
    if (!strcmp(argv[1], "shapes"))
    {
        return shapes();
    }
    if (!strcmp(argv[1], "cache"))
    {
        return cache();
    }
    if (!strcmp(argv[1], "oracle"))
    {
        return oracle();
    }
    return 2;
}
