#define _POSIX_C_SOURCE 200809L
#include "atomics.h"
#include <limits.h>
#include <pthread.h>
#include <sched.h>
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

static void operations(void)
{
    struct ta_counter c;
    ta_counter_init(&c, UINT_MAX);
    CHECK(ta_counter_add(&c, 2) == UINT_MAX);
    CHECK(ta_counter_read(&c) == 1);
    CHECK(ta_counter_add(&c, 0) == 1 && ta_counter_read(&c) == 1);
    _Atomic(struct ta_pair) object;
    struct ta_pair initial = {UINT64_MAX, 7};
    struct ta_pair desired = {3, 9};
    atomic_init(&object, initial);
    struct ta_pair observed = atomic_load(&object);
    CHECK(observed.first == UINT64_MAX && observed.second == 7);
    atomic_store(&object, desired);
    observed = atomic_load(&object);
    CHECK(observed.first == 3 && observed.second == 9);
    /* Initialize a distinct destination from a loaded value; never byte-copy a
       live atomic object or infer padding bytes from semantic field equality. */
    _Atomic(struct ta_pair) second;
    atomic_init(&second, observed);
    observed = atomic_load(&second);
    CHECK(observed.first == 3 && observed.second == 9);
    printf(
        "OBS operations unsigned-old-value/wrap; aggregate size=%zu align=%zu "
        "fields=3,9\n",
        sizeof object, _Alignof(_Atomic(struct ta_pair)));
}

#define QUERY(name, type, alignment)                                   \
    do                                                                 \
    {                                                                  \
        _Alignas(alignment) _Atomic(type) object;                      \
        atomic_init(&object, 0);                                       \
        printf("QUERY %s size=%zu type_align=%zu declared_align=%zu "  \
               "address_mod64=%zu lockfree=%d\n",                      \
               name, sizeof object, _Alignof(_Atomic(type)),           \
               (size_t)(alignment), (size_t)((uintptr_t)&object % 64), \
               atomic_is_lock_free(&object));                          \
    } while (0)

static void lockfree(void)
{
    QUERY("u8", uint8_t, _Alignof(_Atomic(uint8_t)));
    QUERY("u16", uint16_t, _Alignof(_Atomic(uint16_t)));
    QUERY("u32", uint32_t, _Alignof(_Atomic(uint32_t)));
    QUERY("u64", uint64_t, _Alignof(_Atomic(uint64_t)));
    QUERY("u64-overaligned", uint64_t, 64);
    _Atomic(struct ta_pair) pair;
    struct ta_pair zero = {0, 0};
    atomic_init(&pair, zero);
    printf("QUERY pair size=%zu type_align=%zu lockfree=%d\n", sizeof pair,
           _Alignof(_Atomic(struct ta_pair)), atomic_is_lock_free(&pair));
    printf("MACROS char=%d short=%d int=%d long=%d llong=%d pointer=%d\n",
           ATOMIC_CHAR_LOCK_FREE, ATOMIC_SHORT_LOCK_FREE, ATOMIC_INT_LOCK_FREE,
           ATOMIC_LONG_LOCK_FREE, ATOMIC_LLONG_LOCK_FREE,
           ATOMIC_POINTER_LOCK_FREE);
    /* Admission guard is assessed independently of runtime query availability. */
    CHECK(ta_lockfree_admit(1, 0, 1) == 1);
    CHECK(ta_lockfree_admit(1, 1, 1) == 0);
    CHECK(ta_lockfree_admit(1, 0, 0) == 0);
    CHECK(ta_lockfree_admit(0, 0, 1) == 0);
    CHECK(ta_lockfree_admit(2, 0, 1) == -1);
    puts("OBS lockfree actual objects only; over-alignment valid; no foreign "
         "runtime claim");
}

enum
{
    EVENTS = 4,
    PERMUTATION_CODES = 256,
    UNSET = 2
};

/* True when `order` lists events 0..3 exactly once each. */
static int is_permutation(const unsigned order[EVENTS])
{
    for (unsigned i = 0; i < EVENTS; i++)
    {
        for (unsigned j = i + 1; j < EVENTS; j++)
        {
            if (order[i] == order[j])
            {
                return 0;
            }
        }
    }
    return 1;
}

/* Events 0,1 are thread 0 and events 2,3 are thread 1; a total order must
 * keep each thread's own events in program order. */
static int respects_program_order(const unsigned order[EVENTS])
{
    unsigned position[EVENTS];
    for (unsigned i = 0; i < EVENTS; i++)
    {
        position[order[i]] = i;
    }
    return position[0] < position[1] && position[2] < position[3];
}

/*
 * Executes the events in `order` against plain variables x and y (initially
 * 0). Event 0 stores x; event 1 is thread 0's second operation (store y for
 * MP, load y for SB); event 2 is thread 1's first (load y for MP, store y for
 * SB); event 3 loads x. *r and *s receive the two loaded values, UNSET when
 * the corresponding load never ran.
 */
static void execute_order(unsigned litmus, const unsigned order[EVENTS],
                          unsigned *r, unsigned *s)
{
    unsigned x = 0;
    unsigned y = 0;
    *r = UNSET;
    *s = UNSET;
    for (unsigned i = 0; i < EVENTS; i++)
    {
        switch (order[i])
        {
            case 0:
                x = 1;
                break;
            case 1:
                if (litmus == TA_MP)
                {
                    y = 1;
                }
                else
                {
                    *r = y;
                }
                break;
            case 2:
                if (litmus == TA_MP)
                {
                    *r = y;
                }
                else
                {
                    y = 1;
                }
                break;
            case 3:
                *s = x;
                break;
            default:
                break;
        }
    }
}

static int permutation(unsigned litmus, unsigned first, unsigned second)
{
    /* Independent SC oracle: enumerate all 24 total orders and execute stores
       and loads. This oracle is deliberately not used for relaxed or RA
       outcomes. Each order is encoded as four base-4 digits. */
    for (unsigned code = 0; code < PERMUTATION_CODES; code++)
    {
        unsigned order[EVENTS];
        for (unsigned i = 0; i < EVENTS; i++)
        {
            order[i] = (code >> (2 * i)) & 3u;
        }
        if (!is_permutation(order) || !respects_program_order(order))
        {
            continue;
        }
        unsigned r;
        unsigned s;
        execute_order(litmus, order, &r, &s);
        if (r == first && s == second)
        {
            return 1;
        }
    }
    return 0;
}

static void model(void)
{
    const unsigned masks[2][4] = {{15, 11, 11, 15}, {15, 15, 14, 15}};
    for (unsigned l = 0; l < 2; l++)
    {
        for (unsigned m = 0; m < 4; m++)
        {
            for (unsigned a = 0; a < 2; a++)
            {
                for (unsigned b = 0; b < 2; b++)
                {
                    int expected = (int)((masks[l][m] >> (2 * a + b)) & 1u);
                    CHECK(ta_litmus(l, m, a, b) == expected);
                    if (m == TA_MODEL_SC)
                    {
                        CHECK(expected == permutation(l, a, b));
                    }
                }
            }
        }
    }
    CHECK(ta_litmus(2, 0, 0, 0) == -1);
    puts("OBS model32 exact MP/SB order-outcomes; SC independently enumerated; "
         "RA SB00 permitted; mixed loads outside SC order");
}

/* Message passing between the main thread (writer) and one reader thread.
 * The reader sees the payload `node` either through `pointer` or after
 * observing `ready`; each path has its own ordering, stated where it is
 * used. */
struct tw_message
{
    atomic_int ready;
    _Atomic(struct ta_node *) pointer;
    struct ta_node node;
    int result;
};

static void *consume_reader(void *p)
{
    struct tw_message *m = p;
    struct ta_node *node;
    do
    {
        node = atomic_load_explicit(&m->pointer, memory_order_consume);
        if (!node)
        {
            sched_yield();
        }
    } while (!node);
    m->result = (int)node->value;
    return NULL;
}
#ifndef TA_TSAN
static void *fence_reader(void *p)
{
    struct tw_message *m = p;
    while (!atomic_load_explicit(&m->ready, memory_order_relaxed))
    {
        sched_yield();
    }
    atomic_thread_fence(memory_order_acquire);
    m->result = (int)m->node.value;
    return NULL;
}
#endif
static void dependency(void)
{
    CHECK(ta_dependency(1, 0, 0) == 1);
    CHECK(ta_dependency(1, 1, 0) == 0);
    CHECK(ta_dependency(1, 0, 1) == 0);
    CHECK(ta_dependency(0, 1, 0) == 0);
    CHECK(ta_dependency(2, 0, 0) == -1);
    struct tw_message m;
    atomic_init(&m.ready, 0);
    atomic_init(&m.pointer, NULL);
    m.result = 0;
    pthread_t thread;
    CHECK(pthread_create(&thread, NULL, consume_reader, &m) == 0);
    m.node.value = 42;
    /* Publishes the payload written above; the reader's consume load pairs
       with this store. */
    atomic_store_explicit(&m.pointer, &m.node, memory_order_release);
    CHECK(pthread_join(thread, NULL) == 0 && m.result == 42);
#ifndef TA_TSAN
    atomic_store(&m.ready, 0);
    m.result = 0;
    CHECK(pthread_create(&thread, NULL, fence_reader, &m) == 0);
    m.node.value = 71;
    /* The release fence orders the payload write before the relaxed flag
       store; the reader's acquire fence completes the pairing. */
    atomic_thread_fence(memory_order_release);
    atomic_store_explicit(&m.ready, 1, memory_order_relaxed);
    CHECK(pthread_join(thread, NULL) == 0 && m.result == 71);
    puts(
        "FENCE actual release-fence/relaxed-flag/acquire-fence publication=71");
#else
    puts(
        "FENCE not-run under TSan: this tool does not model atomic_thread_fence "
        "synchronization");
#endif
    puts("OBS dependency direct pointer publication=42; control and "
         "kill_dependency do not carry pointer dependency; draft/final gap "
         "retained");
}

int main(int argc, char **argv)
{
    CHECK(argc == 2);
    if (!strcmp(argv[1], "operations"))
    {
        operations();
    }
    else if (!strcmp(argv[1], "lockfree"))
    {
        lockfree();
    }
    else if (!strcmp(argv[1], "model"))
    {
        model();
    }
    else if (!strcmp(argv[1], "dependency"))
    {
        dependency();
    }
    else
    {
        return 2;
    }
    return 0;
}
