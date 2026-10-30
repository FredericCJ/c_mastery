#include "test.h"
#include <stddef.h>

/* Insert, remove, look up and reorder; identifiers survive index changes. */
static void check_identity_after_remove_and_reorder(struct nd_store *store)
{
    char alpha[ND_LABEL_BYTES] = {"alpha"};
    char beta[ND_LABEL_BYTES] = {"beta"};
    char gamma[ND_LABEL_BYTES] = {"gamma"};
    unsigned first_id;
    unsigned second_id;
    unsigned third_id;
    CHECK(nd_insert(store, 1, 2, alpha, &first_id) == 0);
    CHECK(nd_insert(store, 3, 4, beta, &second_id) == 0);
    CHECK(nd_insert(store, 5, 6, gamma, &third_id) == 0);
    CHECK(nd_remove(store, second_id) == 0);
    struct nd_record record;
    CHECK(nd_record(store, third_id, &record) == 0 && record.x == 5 &&
          !strcmp(record.label, "gamma"));
    const unsigned swap[] = {1, 0};
    CHECK(nd_reorder(store, swap, 2) == 0);
    CHECK(nd_record(store, first_id, &record) == 0 && record.x == 1 &&
          !strcmp(record.label, "alpha"));

    /* The removed identifier no longer names a record. */
    CHECK(nd_record(store, second_id, &record) == ND_BAD);
}

/* A rejected reorder must leave the store unchanged (compared with memcmp). */
static void check_invalid_permutation(struct nd_store *store)
{
    struct nd_store before = *store;
    const unsigned repeated[] = {0, 0};
    CHECK(nd_reorder(store, repeated, 2) == ND_BAD &&
          memcmp(store, &before, sizeof *store) == 0);
}

/* Fills the store to ND_ITEMS records and checks that one more is refused.
 * It depends on the two records that check_identity_after_remove_and_reorder
 * leaves in the store (three inserted, one removed), hence ND_ITEMS - 2. */
static void check_capacity(struct nd_store *store)
{
    char label[ND_LABEL_BYTES] = {"alpha"};
    unsigned id;
    for (unsigned i = 0; i < ND_ITEMS - 2; i++)
    {
        CHECK(nd_insert(store, i, -(double)i, label, &id) == 0);
    }
    struct nd_store before = *store;
    CHECK(nd_insert(store, 1, 2, label, &id) == ND_BAD &&
          memcmp(store, &before, sizeof *store) == 0);
}

/* The record view and the raw arrays must agree on every live record. */
static void check_views_agree(const struct nd_store *store)
{
    double through_record = 0;
    double through_array = 0;
    struct nd_record record;
    for (size_t i = 0; i < store->count; i++)
    {
        CHECK(nd_record(store, store->id[i], &record) == 0);
        through_record += record.x;
        through_array += store->x[i];
    }
    CHECK(through_record == through_array);
}

int main(void)
{
    struct nd_store store = {{0}, {0}, {0}, {{0}}, 0, 0};
    check_identity_after_remove_and_reorder(&store);
    check_invalid_permutation(&store);
    check_capacity(&store);
    check_views_agree(&store);
    printf("PASS hot/cold identity, reorder and bounded capacity; "
           "sizeof(record)=%zu align=%zu offsets=%zu,%zu,%zu,%zu\n",
           sizeof(struct nd_record), _Alignof(struct nd_record),
           offsetof(struct nd_record, x), offsetof(struct nd_record, y),
           offsetof(struct nd_record, id), offsetof(struct nd_record, label));
}
