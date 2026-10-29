#ifndef STORAGE_FOLLOW_TREE_H
#define STORAGE_FOLLOW_TREE_H
#include "store.h"

enum
{
    TREE_ALLOC,
    TREE_LEAF_SPLIT,
    TREE_INTERNAL_SPLIT,
    TREE_ROOT_GROW,
    TREE_BORROW_LEFT,
    TREE_BORROW_RIGHT,
    TREE_MERGE_LEFT,
    TREE_MERGE_RIGHT,
    TREE_ROOT_SHRINK,
    TREE_FULL_VISIT,
    TREE_MIN_VISIT,
    TREE_REPAIR_NODE,
    TREE_EVENTS
};

/* Explicit teaching instrumentation; synchronous, no reentry or external owners. */
int tree_page_admit(uint32_t logical, int leaf);
void tree_event(unsigned event, uint32_t logical);
#endif
