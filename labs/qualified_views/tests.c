/*
 * Public contract tests for the qualified-views lab (views.h).
 *
 * A failing check prints "FV_FAIL <file>:<line> <expression>" to stderr and
 * the section returns 1. The campaign matches fragments of those expressions;
 * keep them spelled as written.
 */
#include "views.h"
#include <stdint.h>
#include <stdio.h>

#define CHECK(condition)                                              \
    do                                                                \
    {                                                                 \
        if (!(condition))                                             \
        {                                                             \
            fprintf(stderr, "FV_FAIL %s:%d %s\n", __FILE__, __LINE__, \
                    #condition);                                      \
            return 1;                                                 \
        }                                                             \
    } while (0)

enum
{
    RECORD_VALUES = 4, /* ints in struct fv_record.values */
    SNAPSHOT_SIZE = 5, /* one more than a record, to see the tail untouched */
    OVERLAP_EXTENT = 6
};

static struct fv_record implicit;

/* File-scope compound literal is a modifiable static-duration object. */
static int *file_values = (int[]){[1] = 7, [3] = 9};

/* True for the documented initial contents {0, 7, 0, 9}. */
static int has_initial_values(const int *values)
{
    return values[0] == 0 && values[1] == 7 && values[2] == 0 && values[3] == 9;
}

/* Initialization: static, designated, assigned, compound literals, reset. */
static int test_initialization(void)
{
    CHECK(implicit.values[0] == 0 && implicit.values[3] == 0 &&
          implicit.link == NULL);

    struct fv_record initialized = {.values = {[1] = 7, [3] = 9}};
    CHECK(has_initial_values(initialized.values) && initialized.link == NULL);

    struct fv_record assigned;
    assigned = initialized; /* No read precedes the assignment. */
    CHECK(has_initial_values(assigned.values) && assigned.link == NULL);

    /* A block compound literal lives to the end of its enclosing block. */
    CHECK(has_initial_values((int[]){[1] = 7, [3] = 9}));
    CHECK(has_initial_values(file_values));
    int *alias = file_values;
    alias[2] = 11;
    CHECK(file_values[2] == 11);
    file_values[2] = 0;

    int target = 42;
    initialized.link = &target;
    CHECK(fv_reset(&initialized) == FV_OK);
    CHECK(initialized.values[0] == 0 && initialized.values[3] == 0 &&
          initialized.link == NULL);
    CHECK(fv_reset(NULL) == FV_INVALID);
    puts("FV_INIT_PASS zero_values null_pointer designated assignment "
         "compound_durations");
    return 0;
}

/* Views: caller-owned storage, live aliasing, detached snapshots. */
static int test_views(void)
{
    struct fv_record caller;
    struct fv_view returned = fv_initialize(&caller);
    CHECK(returned.data == caller.values && returned.length == RECORD_VALUES &&
          has_initial_values(returned.data) && caller.link == NULL);
    CHECK(fv_initialize(NULL).length == 0);

    struct fv_record owner = {.values = {[1] = 7, [3] = 9}};
    struct fv_view live = fv_view(&owner);
    CHECK(live.data == owner.values && live.length == RECORD_VALUES);

    /* A destination one element too small must be rejected untouched. */
    int snapshot[SNAPSHOT_SIZE] = {31, 32, 33, 34, 35};
    CHECK(fv_copy((struct fv_destination){snapshot, 3}, live) == FV_FULL);
    CHECK(snapshot[0] == 31 && snapshot[1] == 32 && snapshot[2] == 33 &&
          snapshot[3] == 34 && snapshot[4] == 35);
    CHECK(fv_copy((struct fv_destination){snapshot, 5}, live) == FV_OK &&
          has_initial_values(snapshot) && snapshot[4] == 35);

    /* The live view follows the owner; the snapshot does not. */
    CHECK(fv_update(&owner, 1, 8) == FV_OK && live.data[1] == 8 &&
          snapshot[1] == 7);
    CHECK(fv_update(&owner, 4, 9) == FV_RANGE && live.data[1] == 8);
    CHECK(fv_update(&owner, SIZE_MAX, 9) == FV_RANGE);
    CHECK(fv_update(NULL, 0, 1) == FV_INVALID);
    CHECK(fv_view(NULL).length == 0 && fv_view(NULL).data == NULL);

    CHECK(fv_copy((struct fv_destination){NULL, 0},
                  (struct fv_view){NULL, 0}) == FV_OK);
    CHECK(fv_copy((struct fv_destination){NULL, 1},
                  (struct fv_view){NULL, 0}) == FV_INVALID);
    CHECK(fv_copy((struct fv_destination){snapshot, 4},
                  (struct fv_view){NULL, 1}) == FV_INVALID);
    CHECK(fv_reset(&owner) == FV_OK && live.data[0] == 0 && live.data[1] == 0 &&
          live.data[3] == 0 && snapshot[1] == 7);

    struct fv_owner *handle = fv_owner();
    CHECK(handle != NULL);
    struct fv_view opaque = fv_owner_view(handle);
    int *mutable_alias = fv_owner_mutable(handle);
    CHECK(opaque.length == RECORD_VALUES && opaque.data == mutable_alias &&
          mutable_alias != NULL);
    mutable_alias[0] = 71;
    CHECK(opaque.data[0] == 71);
    CHECK(fv_owner_view(NULL).length == 0 && fv_owner_mutable(NULL) == NULL);
    puts("FV_VIEW_PASS caller_owned live_alias detached_snapshot empty "
         "capacity_atomic");
    return 0;
}

/* Qualifiers: overlapping moves in both directions, bounds, volatile reads. */
static int test_qualifiers(void)
{
    int overlap[OVERLAP_EXTENT] = {1, 2, 3, 4, 5, 6};

    /* Destination above source: copying upward would smear element 0. */
    CHECK(fv_move(overlap, OVERLAP_EXTENT, 1, 0, 5) == FV_OK);
    CHECK(overlap[0] == 1 && overlap[1] == 1 && overlap[2] == 2 &&
          overlap[3] == 3 && overlap[4] == 4 && overlap[5] == 5);
    CHECK(fv_move(overlap, OVERLAP_EXTENT, 0, 1, 5) == FV_OK);
    CHECK(overlap[0] == 1 && overlap[1] == 2 && overlap[2] == 3 &&
          overlap[3] == 4 && overlap[4] == 5);

    /* One-past positions at zero count are valid and never dereferenced. */
    CHECK(fv_move(overlap, OVERLAP_EXTENT, 6, 6, 0) == FV_OK);
    CHECK(fv_move(NULL, 0, 0, 0, 0) == FV_OK);
    CHECK(fv_move(NULL, 1, 0, 0, 0) == FV_INVALID);
    CHECK(fv_move(overlap, OVERLAP_EXTENT, SIZE_MAX, 0, 1) == FV_RANGE);
    CHECK(fv_move(overlap, OVERLAP_EXTENT, 0, SIZE_MAX, 1) == FV_RANGE);
    CHECK(fv_move(overlap, OVERLAP_EXTENT, 0, 0, SIZE_MAX) == FV_RANGE);

    /* Stable RAM: both volatile reads agree, so the XOR is zero. */
    volatile unsigned device = 3;
    CHECK(fv_device_twice(&device) == 0 && device == 3);
    puts("FV_QUALIFIER_PASS restrict_disjoint overlap_direction "
         "volatile_RAM_only");
    return 0;
}

int main(void)
{
    if (test_initialization() != 0)
    {
        return 1;
    }
    if (test_views() != 0)
    {
        return 1;
    }
    if (test_qualifiers() != 0)
    {
        return 1;
    }
    puts("FV_PASS qualified_views");
    return 0;
}
