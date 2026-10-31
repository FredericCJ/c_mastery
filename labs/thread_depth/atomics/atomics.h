#ifndef THREAD_ATOMICS_H
#define THREAD_ATOMICS_H
#include <stdatomic.h>
#include <stddef.h>
#include <stdint.h>

/* Own enum, never rely on a compiler's memory_order integer encoding. */
enum ta_order
{
    TA_RELAXED,
    TA_CONSUME,
    TA_ACQUIRE,
    TA_RELEASE,
    TA_ACQ_REL,
    TA_SC
};

int ta_order_pair(unsigned success, unsigned failure);

enum ta_result
{
    TA_SUCCESS,
    TA_LIMIT,
    TA_INVALID,
    TA_UNPROTECTED
};

typedef int (*ta_cas_fn)(void *, unsigned *, unsigned, int);

/* Outcome of ta_increment: attempts made, the last expected and desired
 * values, and whether the exchange published (1) or not (0). */
struct ta_report
{
    unsigned attempts;
    unsigned expected;
    unsigned desired;
    unsigned published;
};

int ta_increment(ta_cas_fn, void *, unsigned, unsigned, int,
                 struct ta_report *);

struct ta_node
{
    unsigned value;
};

typedef int (*ta_node_cas_fn)(void *, struct ta_node **, struct ta_node *, int);

struct ta_node_report
{
    unsigned attempts;
    unsigned published;
    unsigned derived_value;
};

/* The explicit lease set is live until the operation completes. It is not
   acquired by pointer equality and is not a general reclamation mechanism. */
int ta_publish(ta_node_cas_fn, void *, struct ta_node *, struct ta_node *,
               struct ta_node *const *, size_t, int, unsigned,
               struct ta_node_report *);

struct ta_snapshot
{
    atomic_uint sequence;
    atomic_int a;
    atomic_int b;
};

void ta_snapshot_init(struct ta_snapshot *);
int ta_snapshot_write(struct ta_snapshot *, int);
int ta_snapshot_try(struct ta_snapshot *, unsigned, int *);

struct ta_slot
{
    atomic_uint pending;
    unsigned readers;
    int payload;
};

int ta_slot_init(struct ta_slot *, unsigned);
int ta_slot_publish(struct ta_slot *, int);
int ta_slot_read(struct ta_slot *, unsigned, int *);
/* Analysis-only access descriptor. No racy target expression is executed. */
int ta_data_race(unsigned same_location, unsigned first_writes,
                 unsigned second_writes, unsigned both_atomic,
                 unsigned happens_before);
/* Transitive closure for at most 8 events. Each bit j of edges[i] is i->j. */
int ta_closure(unsigned *, size_t);

enum ta_litmus
{
    TA_MP,
    TA_SB
};

enum ta_model
{
    TA_MODEL_RELAXED,
    TA_MODEL_RA,
    TA_MODEL_SC,
    TA_MODEL_MIXED
};

/* Restricted two-location, two-thread, constant-store model in CONTRACTS.md.
   Returns 1 permitted, 0 forbidden, -1 outside this model. */
int ta_litmus(unsigned, unsigned, unsigned, unsigned);

struct ta_pair
{
    uint64_t first;
    uint64_t second;
};

struct ta_counter
{
    atomic_uint value;
};

void ta_counter_init(struct ta_counter *, unsigned);
unsigned ta_counter_add(struct ta_counter *, unsigned);
unsigned ta_counter_read(const struct ta_counter *);
int ta_lockfree_admit(unsigned query, unsigned unresolved_helpers,
                      unsigned context_authorized);
int ta_dependency(unsigned pointer_value_used, unsigned control_only,
                  unsigned killed);
#endif
