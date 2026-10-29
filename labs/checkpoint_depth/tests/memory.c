#include "checkpoint_memory.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define CHECK(x)                                                             \
    do                                                                       \
    {                                                                        \
        if (!(x))                                                            \
        {                                                                    \
            fprintf(stderr, "CP_CHECK: %s:%d %s\n", __FILE__, __LINE__, #x); \
            exit(1);                                                         \
        }                                                                    \
    } while (0)

static unsigned live(const struct cpm_pool *p)
{
    unsigned n = 0;
    for (unsigned i = 0; i < CPM_SLOTS; ++i)
        n += p->state[i] == 1;
    return n;
}

static int identical(const struct cpm_pool *a, const struct cpm_pool *b)
{
    if (a->attempts != b->attempts || a->fail_at != b->fail_at)
        return 0;
    for (unsigned i = 0; i < CPM_SLOTS; ++i)
        if (a->state[i] != b->state[i] ||
            a->generation[i] != b->generation[i] || a->pins[i] != b->pins[i] ||
            a->objects[i].value != b->objects[i].value)
            return 0;
    return 1;
}

static int pop_value(struct cpm_pool *p, struct cpm_ring *q)
{
    struct cpm_handle h;
    const struct cpm_record *r;
    CHECK(cpm_pop(q, &h) == CPM_OK);
    CHECK(cpm_lookup(p, h, &r) == CPM_OK);
    int result = r->value;
    CHECK(cpm_release(p, h) == CPM_OK);
    return result;
}

void cp_test_memory(void)
{
    _Alignas(8) unsigned char bytes[32];
    memset(bytes, 0xcc, sizeof bytes);
    struct cpm_arena a;
    struct cpm_span first, second, last = {99, 99, 99};
    CHECK(cpm_arena_init(&a, bytes, 32, 8, 8) == CPM_OK);
    CHECK(cpm_reserve(&a, 5, &first) == CPM_OK && first.offset == 8 &&
          a.used == 13);
    CHECK(cpm_reserve(&a, 9, &second) == CPM_OK && second.offset == 16 &&
          a.used == 25);
    unsigned char saved[32];
    memcpy(saved, bytes, sizeof saved);
    CHECK(cpm_reserve(&a, 1, &last) == CPM_FULL && a.used == 25 &&
          last.offset == 99);
    CHECK(!memcmp(saved, bytes, sizeof saved));
    CHECK(cpm_reserve(&a, SIZE_MAX, &last) == CPM_INVALID && a.used == 25);
    CHECK(cpm_arena_init(&a, bytes, 32, 8, 16) == CPM_INVALID && a.used == 25);
    /* Copy into a declared typed object, never reinterpret byte storage. */
    int value = 123, roundtrip = 0;
    CHECK(sizeof value <= first.length);
    memcpy(bytes + first.offset, &value, sizeof value);
    memcpy(&roundtrip, bytes + first.offset, sizeof roundtrip);
    CHECK(roundtrip == 123);
    struct cpm_pool p;
    struct cpm_ring q;
    cpm_pool_init(&p);
    cpm_ring_init(&q);
    int values[] = {11, 22, 33, 44, 55, 66, 77};
    size_t accepted;
    CHECK(cpm_prefix(&p, &q, values, 4, &accepted) == CPM_OK && accepted == 4 &&
          live(&p) == 4);
    CHECK(cpm_prefix(&p, &q, values + 4, 1, &accepted) == CPM_FULL &&
          accepted == 0);
    CHECK(pop_value(&p, &q) == 11 && pop_value(&p, &q) == 22);
    CHECK(cpm_prefix(&p, &q, values + 5, 2, &accepted) == CPM_OK &&
          accepted == 2);
    for (size_t i = 2; i < 4; ++i)
        CHECK(pop_value(&p, &q) == values[i]);
    CHECK(pop_value(&p, &q) == 66 && pop_value(&p, &q) == 77 && live(&p) == 0);
    cpm_pool_init(&p);
    cpm_ring_init(&q);
    CHECK(cpm_prefix(&p, &q, values, 2, &accepted) == CPM_OK);
    CHECK(cpm_prefix(&p, &q, values + 2, 3, &accepted) == CPM_FULL &&
          accepted == 2 && q.count == 4);
    for (unsigned ordinal = 1; ordinal <= 4; ++ordinal)
    {
        cpm_pool_init(&p);
        cpm_ring_init(&q);
        cpm_fail_at(&p, ordinal);
        CHECK(cpm_prefix(&p, &q, values, 4, &accepted) == CPM_FULL &&
              accepted == ordinal - 1 && live(&p) == ordinal - 1);
        for (unsigned i = 0; i < accepted; ++i)
            CHECK(pop_value(&p, &q) == values[i]);
        CHECK(live(&p) == 0);
        cpm_pool_init(&p);
        cpm_ring_init(&q);
        cpm_fail_at(&p, ordinal);
        struct cpm_pool original = p;
        CHECK(cpm_atomic(&p, &q, values, 4) == CPM_FULL && q.count == 0 &&
              identical(&p, &original));
    }
    cpm_pool_init(&p);
    cpm_ring_init(&q);
    CHECK(cpm_atomic(&p, &q, values, 4) == CPM_OK && q.count == 4);
    CHECK(cpm_atomic(&p, &q, NULL, 0) == CPM_OK);
    for (unsigned i = 0; i < 4; ++i)
        CHECK(pop_value(&p, &q) == values[i]);
    CHECK(live(&p) == 0);
}

void cp_test_ownership(void)
{
    _Alignas(8) unsigned char bytes[32];
    struct cpm_arena a;
    struct cpm_span first, second;
    CHECK(cpm_arena_init(&a, bytes, sizeof bytes, 8, 8) == CPM_OK);
    CHECK(cpm_reserve(&a, 5, &first) == CPM_OK);
    CHECK(cpm_reserve(&a, 9, &second) == CPM_OK);
    CHECK(cpm_pin(&a, first) == CPM_OK && cpm_pin(&a, second) == CPM_OK);
    CHECK(cpm_reset(&a) == CPM_BUSY && a.used == 25);
    CHECK(cpm_unpin(&a) == CPM_OK);
    CHECK(cpm_reset(&a) == CPM_BUSY);
    /* Cancellation requested is not callback completion: retain the last pin. */
    CHECK(cpm_reset(&a) == CPM_BUSY);
    CHECK(cpm_unpin(&a) == CPM_OK);
    CHECK(cpm_reset(&a) == CPM_OK);
    const unsigned char *view = bytes + 31;
    CHECK(cpm_view(&a, first, &view) == CPM_INVALID && view == bytes + 31);
    CHECK(cpm_reset(&a) == CPM_OK);
    CHECK(cpm_reset(&a) == CPM_FULL && a.generation == CPM_GENERATION_MAX);
    struct cpm_pool p;
    cpm_pool_init(&p);
    struct cpm_handle h, old;
    const struct cpm_record *record;
    CHECK(cpm_allocate(&p, 11, &h) == CPM_OK && h.slot == 0);
    old = h;
    CHECK(cpm_pool_pin(&p, h) == CPM_OK);
    CHECK(cpm_release(&p, h) == CPM_BUSY);
    CHECK(cpm_pool_unpin(&p, h) == CPM_OK);
    CHECK(cpm_release(&p, h) == CPM_OK);
    CHECK(cpm_lookup(&p, old, &record) == CPM_INVALID);
    for (unsigned gen = 2; gen <= CPM_GENERATION_MAX; ++gen)
    {
        CHECK(cpm_allocate(&p, 22, &h) == CPM_OK && h.slot == 0 &&
              h.generation == gen);
        CHECK(cpm_release(&p, h) == CPM_OK);
    }
    CHECK(cpm_allocate(&p, 33, &h) == CPM_OK && h.slot == 1);
    CHECK(cpm_lookup(&p, old, &record) == CPM_INVALID && p.state[0] == 2);
    CHECK(cpm_pool_unpin(&p, old) == CPM_INVALID);
    CHECK(cpm_lookup(&p, h, &record) == CPM_OK && record == &p.objects[1] &&
          record->value == 33);
}
