#ifndef CM_CONSUME_INDEX_H
#define CM_CONSUME_INDEX_H
#include <stdatomic.h>

struct ci_node
{
    unsigned index, value;
};

/* Live atomic control; all potential nodes and four array elements are immutable
 * and live for the call. Non-null node index is 0..3. This execution contract
 * does not authorize racing ordinary array writes. See CONTRACTS.md. */
unsigned ci_pointer(_Atomic(struct ci_node *) *publication);
unsigned ci_value_index(_Atomic(struct ci_node *) *publication,
                        const unsigned values[4]);
unsigned ci_branch_index(_Atomic(struct ci_node *) *publication,
                         const unsigned values[4]);
unsigned ci_killed_index(_Atomic(struct ci_node *) *publication,
                         const unsigned values[4]);
unsigned ci_acquire_branch(_Atomic(struct ci_node *) *publication,
                           const unsigned values[4]);
/* Six edges ordered 01,02,03,12,13,23. Base-three digit: absent0,
 * value1, control2. Return the value-edge reachability mask from vertex0.
 * Valid code0..728. This is a declared finite graph, not a C parser/model. */
unsigned ci_dependency_graph(unsigned code);
#endif
