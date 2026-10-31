#include "atomics.h"
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define CHECK(x)                                                             \
    do                                                                       \
    {                                                                        \
        if (!(x))                                                            \
        {                                                                    \
            fprintf(stderr, "TA_ASSERT:%s:%d:%s\n", __FILE__, __LINE__, #x); \
            exit(41);                                                        \
        }                                                                    \
    } while (0)

enum
{
    SCRIPT_CALLS = 16,
    ORDER_COUNT = 6
};

/*
 * Scripted stand-in for a compare-and-swap word.
 *   actual    the value the "memory" currently holds;
 *   spurious  number of weak attempts that fail without changing anything;
 *   mismatch  if set, the next attempt first sets `actual` to 20 (a competing
 *             writer); it fails, reporting 20, only if *expected != 20;
 *   strong    count of strong attempts; desired[] records each desired value.
 */
struct tw_script
{
    unsigned actual;
    unsigned call;
    unsigned spurious;
    unsigned mismatch;
    unsigned strong;
    unsigned desired[SCRIPT_CALLS];
};

static int scripted(void *arg, unsigned *expected, unsigned desired, int weak)
{
    struct tw_script *s = arg;
    CHECK(s->call < SCRIPT_CALLS);
    s->desired[s->call++] = desired;
    if (!weak)
    {
        s->strong++;
    }
    if (weak && s->spurious)
    {
        s->spurious--;
        return 0;
    }
    if (s->mismatch)
    {
        s->mismatch = 0;
        s->actual = 20;
    }
    if (*expected != s->actual)
    {
        *expected = s->actual;
        return 0;
    }
    s->actual = desired;
    return 1;
}

static int real_cas(void *arg, unsigned *expected, unsigned desired, int weak)
{
    atomic_uint *p = arg;
    return weak ? atomic_compare_exchange_weak_explicit(p, expected, desired,
                                                        memory_order_acq_rel,
                                                        memory_order_acquire)
                : atomic_compare_exchange_strong_explicit(p, expected, desired,
                                                          memory_order_acq_rel,
                                                          memory_order_acquire);
}

static void orders(void)
{
    /* Independent explicit pairs, including the release-only changed case. */
    const unsigned pairs[][2] = {{0, 0}, {1, 0}, {1, 1}, {2, 0}, {2, 1},
                                 {2, 2}, {3, 0}, {4, 0}, {4, 1}, {4, 2},
                                 {5, 0}, {5, 1}, {5, 2}, {5, 5}};
    for (unsigned s = 0; s < ORDER_COUNT; s++)
    {
        for (unsigned f = 0; f < ORDER_COUNT; f++)
        {
            int expected = 0;
            for (size_t i = 0; i < sizeof pairs / sizeof pairs[0]; i++)
            {
                if (pairs[i][0] == s && pairs[i][1] == f)
                {
                    expected = 1;
                }
            }
            CHECK(ta_order_pair(s, f) == expected);
        }
    }
    CHECK(!ta_order_pair(ORDER_COUNT, 0) && !ta_order_pair(0, ORDER_COUNT));
    atomic_uint p;
    atomic_init(&p, 0);
    unsigned expected = 0;
    CHECK(atomic_compare_exchange_strong_explicit(
        &p, &expected, 1, memory_order_release, memory_order_relaxed));
    expected = 0;
    CHECK(!atomic_compare_exchange_strong_explicit(
              &p, &expected, 2, memory_order_release, memory_order_relaxed) &&
          expected == 1);
    puts("OBS orders36 valid14 release-only failure relaxed");
}

static void retry(void)
{
    struct tw_script s = {.actual = 10, .spurious = 1, .mismatch = 1};
    struct ta_report r;
    CHECK(ta_increment(scripted, &s, 10, 3, 0, &r) == TA_SUCCESS);
    CHECK(r.attempts == 3 && r.expected == 20 && r.desired == 21 &&
          r.published == 1);
    CHECK(s.desired[0] == 11 && s.desired[1] == 11 && s.desired[2] == 21 &&
          s.actual == 21);
    s = (struct tw_script){.actual = 10, .spurious = 1};
    CHECK(ta_increment(scripted, &s, 10, 1, 0, &r) == TA_LIMIT &&
          r.published == 0 && s.actual == 10);
    s = (struct tw_script){.actual = 10, .spurious = 1};
    CHECK(ta_increment(scripted, &s, 10, 1, 1, &r) == TA_SUCCESS &&
          s.strong == 1 && s.actual == 11);
    atomic_uint p;
    atomic_init(&p, UINT_MAX);
    CHECK(ta_increment(real_cas, &p, UINT_MAX, 1, 1, &r) == TA_SUCCESS &&
          r.desired == 0 && atomic_load(&p) == 0);
    struct ta_report sentinel = {99, 98, 97, 96};
    r = sentinel;
    CHECK(ta_increment(real_cas, &p, 0, 0, 0, &r) == TA_INVALID &&
          r.attempts == sentinel.attempts && r.expected == sentinel.expected &&
          r.desired == sentinel.desired && r.published == sentinel.published);
    puts("OBS retry spurious unchanged; mismatch20; recomputed21; strong "
         "one-attempt; unsigned wrap");
}

/* Pointer-valued counterpart of tw_script. With free_on_success the CAS
 * acts as the terminal receiver: it frees the published node itself. */
struct node_script
{
    struct ta_node *actual;
    struct ta_node *replacement;
    unsigned call;
    unsigned spurious;
    unsigned mismatch;
    unsigned freed;
    int free_on_success;
};

static int node_cas(void *arg, struct ta_node **expected,
                    struct ta_node *desired, int weak)
{
    struct node_script *s = arg;
    s->call++;
    CHECK(weak);
    if (s->spurious)
    {
        s->spurious--;
        return 0;
    }
    if (s->mismatch)
    {
        s->mismatch = 0;
        s->actual = s->replacement;
    }
    if (*expected != s->actual)
    {
        *expected = s->actual;
        return 0;
    }
    s->actual = desired;
    if (s->free_on_success)
    {
        s->actual = NULL;
        free(desired);
        s->freed++;
    }
    return 1;
}

/* A real atomic head whose "receiver" takes the node back out and frees it
 * as soon as a publication succeeds. */
struct native_node
{
    _Atomic(struct ta_node *) head;
    unsigned calls;
    unsigned freed;
};

static int native_node_cas(void *arg, struct ta_node **expected,
                           struct ta_node *desired, int weak)
{
    struct native_node *n = arg;
    CHECK(weak);
    n->calls++;
    if (!atomic_compare_exchange_strong_explicit(&n->head, expected, desired,
                                                 memory_order_acq_rel,
                                                 memory_order_acquire))
    {
        return 0;
    }
    struct ta_node *receiver =
        atomic_exchange_explicit(&n->head, NULL, memory_order_acq_rel);
    CHECK(receiver == desired);
    free(receiver);
    n->freed++;
    return 1;
}

/* Heap node holding `value`; the caller owns it. */
static struct ta_node *node_new(unsigned value)
{
    struct ta_node *node = malloc(sizeof *node);
    CHECK(node);
    *node = (struct ta_node){value};
    return node;
}

static void ownership(void)
{
    struct ta_node *old = node_new(10);
    struct ta_node *other = node_new(20);
    struct ta_node *owned = node_new(77);
    struct ta_node *leases[] = {old, other};
    struct ta_node_report r;
    struct node_script s = {
        .actual = old, .replacement = other, .spurious = 1, .mismatch = 1};
    int first = ta_publish(node_cas, &s, owned, old, leases, 2, 0, 1, &r);
    int ownership_preserved = first == TA_LIMIT && r.published == 0 &&
                              s.actual == old && owned->value == 77;
    if (!ownership_preserved)
    {
        free(owned);
        free(old);
        free(other);
        CHECK(ownership_preserved);
    }
    CHECK(ta_publish(node_cas, &s, owned, old, leases, 2, 0, 1, &r) ==
              TA_LIMIT &&
          r.published == 0 && s.actual == other);
    CHECK(old->value == 10 && other->value == 20);
    free(owned);
    owned = node_new(0);
    s = (struct node_script){
        .actual = old, .replacement = other, .spurious = 1, .mismatch = 1};
    CHECK(ta_publish(node_cas, &s, owned, old, leases, 2, 1, 3, &r) ==
              TA_SUCCESS &&
          r.published == 1 && r.derived_value == 21 && s.actual == owned);
    free(owned);
    /* A live address not protected by the declared lease is not dereferenced. */
    owned = node_new(88);
    s = (struct node_script){.actual = other};
    CHECK(ta_publish(node_cas, &s, owned, other, leases, 1, 1, 3, &r) ==
              TA_UNPROTECTED &&
          r.attempts == 0 && r.published == 0 && owned->value == 88);
    free(owned);
    owned = node_new(99);
    s = (struct node_script){.actual = old, .free_on_success = 1};
    CHECK(ta_publish(node_cas, &s, owned, old, leases, 2, 0, 1, &r) ==
              TA_SUCCESS &&
          r.published == 1 &&
          s.freed == 1); /* owned already freed by receiver callback */
    owned = node_new(7);
    struct native_node native;
    atomic_init(&native.head, other);
    native.calls = 0;
    native.freed = 0;
    CHECK(ta_publish(native_node_cas, &native, owned, old, leases, 2, 0, 2,
                     &r) == TA_SUCCESS &&
          r.published == 1 && native.calls == 2 && native.freed == 1 &&
          atomic_load(&native.head) == NULL);
    free(old);
    free(other);
    puts("OBS ownership two failures keep caller; lease protects observed "
         "fields; terminal receiver frees once");
}

int main(int argc, char **argv)
{
    CHECK(argc == 2);
    if (!strcmp(argv[1], "orders"))
    {
        orders();
    }
    else if (!strcmp(argv[1], "retry"))
    {
        retry();
    }
    else if (!strcmp(argv[1], "ownership"))
    {
        ownership();
    }
    else
    {
        return 2;
    }
    return 0;
}
