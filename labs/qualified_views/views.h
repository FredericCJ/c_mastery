/*
 * Qualified views: const, restrict and volatile in a small interface.
 *
 * Ownership: struct fv_record and every array a caller passes stay owned by
 * the caller. A struct fv_view only borrows; it never extends a lifetime and
 * it is a live window, not a snapshot. Functions never block or allocate.
 */
#ifndef FV_VIEWS_H
#define FV_VIEWS_H
#include <stddef.h>

enum fv_status
{
    FV_OK,
    /* Required pointer is NULL where a nonzero length or capacity needs it. */
    FV_INVALID,
    /* Destination capacity is smaller than the source length. */
    FV_FULL,
    /* An index or extent is outside the admitted array. */
    FV_RANGE,
    /* Scaffold stub: not yet implemented by the learner. */
    FV_UNFINISHED
};

/* Four owned ints plus a borrowed link that the record never frees. */
struct fv_record
{
    int values[4];
    int *link;
};

/* Read-only window onto `length` ints at `data`; empty means {NULL, 0}. */
struct fv_view
{
    const int *data;
    size_t length;
};

/* Writable window; `capacity` counts ints, not bytes. */
struct fv_destination
{
    int *data;
    size_t capacity;
};

/*
 * Sets the four values to 0 and link to NULL. Returns FV_INVALID for a NULL
 * owner, otherwise FV_OK.
 */
enum fv_status fv_reset(struct fv_record *owner);

/*
 * Fills caller storage with the documented initial values {0, 7, 0, 9} and
 * link NULL, then returns a live four-element view of it. The view is valid
 * while *caller_owner is. A NULL owner returns the empty view.
 */
struct fv_view fv_initialize(struct fv_record *caller_owner);

/*
 * Stores value at values[index], leaving link and the other values alone.
 * FV_INVALID for a NULL owner; FV_RANGE for index >= 4, with no write.
 */
enum fv_status fv_update(struct fv_record *owner, size_t index, int value);

/* Live view of the owner's four values; the empty view for a NULL owner. */
struct fv_view fv_view(const struct fv_record *owner);

/*
 * Copies source.length ints into destination.data. Nonempty source and
 * destination accessed ranges must be disjoint; overlapping ranges are
 * undefined behaviour. FV_INVALID when a nonzero length or capacity has a
 * NULL pointer; FV_FULL when the capacity is too small. Both failures leave
 * the destination unchanged. On success the destination tail beyond
 * source.length is untouched.
 */
enum fv_status fv_copy(struct fv_destination destination,
                       struct fv_view source);

/*
 * Moves count ints from base[source] to base[destination]. All positions refer
 * to the same admitted array of `extent` ints and overlap is permitted: the
 * result equals copying the source slice out first. FV_INVALID for a nonzero
 * extent with NULL base; FV_RANGE when either position exceeds extent or the
 * remaining extent is smaller than count, with no write.
 */
enum fv_status fv_move(int *base, size_t extent, size_t destination,
                       size_t source, size_t count);

/* Opaque owner of one static demonstration record; never freed. */
struct fv_owner;
struct fv_owner *fv_owner(void);

/* Live read-only view of the owner's values; empty for a NULL owner. */
struct fv_view fv_owner_view(const struct fv_owner *owner);

/* Mutable alias of the same four ints, or NULL for a NULL owner. */
int *fv_owner_mutable(struct fv_owner *owner);

/*
 * Reads *status twice in separate expressions and returns first ^ second.
 * Ordinary RAM device model only; no hardware or interthread guarantee.
 */
unsigned fv_device_twice(volatile unsigned *status);
#endif
