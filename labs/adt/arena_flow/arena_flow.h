#ifndef CMASTERY_ADF_H
#define CMASTERY_ADF_H
#include "adt.h"

enum adf_status
{
    ADF_OK,
    ADF_INVALID,
    ADF_BUDGET,
    ADF_NO_FIXED_POINT
};

struct adf_arena
{
    struct ad_node nodes[32];
    uint32_t values[32];
    size_t count;
};
/* Copy then validate the entire arena, including unreachable nodes. Certificate
   owns inline storage; clients must not mutate its public representation. */
enum adf_status adf_arena(const struct ad_node *, size_t, struct adf_arena *);
enum adf_status adf_root(const struct adf_arena *, struct ad_node_index,
                         uint32_t *);

struct adf_node
{
    unsigned predecessors, generate, keep;
};

struct adf_session
{
    struct adf_node nodes[8];
    unsigned count, values[8], pending, evaluations;
};

struct adf_result
{
    unsigned values[8], evaluations;
};
/* L=(powerset of8facts)^count, bottom0; each transfer is
   generate | ((union of predecessors) & keep). Count1..8, all masks checked. */
enum adf_status adf_start(const struct adf_node *, unsigned,
                          struct adf_session *);
/* Progress commits one popped node at a time. BUDGET preserves output, retains
   resumable session progress. OK publishes a fixed point; repeated OK is stable. */
enum adf_status adf_step(struct adf_session *, unsigned budget,
                         struct adf_result *);

struct adf_boolean
{
    unsigned left, right, table;
};
/* Separate arbitrary Boolean-equation model, count1..4. Enumerate assignments
   in numeric order. Only a complete search proves NO_FIXED_POINT. This is not
   the finite monotone solver's state space or a claim of static analysis. */
enum adf_status adf_boolean(const struct adf_boolean *, unsigned count,
                            unsigned budget, unsigned *assignment);
#endif
