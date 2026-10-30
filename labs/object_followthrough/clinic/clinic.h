#ifndef MC_CLINIC_H
#define MC_CLINIC_H
#include "depth.h"

/*
 * Contract conventions. Unless a comment says otherwise an int function
 * returns 0 on success and -1 when the request is rejected, and a rejected
 * request leaves every output and control object as it was. Pointer arguments
 * are NULL (rejected) or point to live objects of the stated type; invalid
 * non-NULL pointers cannot be detected. Callbacks supply real allocated
 * storage or NULL, do not re-enter these functions and do not modify the
 * control objects. Single-threaded; shared.h covers the concurrent case.
 * CONTRACTS.md is the normative text.
 */

/* Injected allocation callbacks; context is passed back unchanged. */
struct mc_allocator
{
    void *context;
    void *(*allocate)(void *context, size_t bytes);
    void (*release)(void *context, void *block);
};

/*
 * Unique owner of one heap int. A new owner is {0}. releases counts
 * successful mc_release() calls (a diagnostic counter, fewer than UINT_MAX).
 */
struct mc_owner
{
    int *value;
    struct mc_allocator allocation;
    unsigned releases;
};

/*
 * Allocates one int through the allocator, stores value in it and publishes
 * it in an empty owner. The allocator needs both callbacks. A live owner is
 * rejected; on allocation failure the owner stays empty.
 */
int mc_acquire(struct mc_owner *owner, struct mc_allocator allocator,
               int value);

/* Copies the current value to *out. Rejected for an empty owner. */
int mc_read(const struct mc_owner *owner, int *out);

/* An independent copy of the current value: a value, never an address into
 * the owner's storage. Same rejection rule as mc_read(). */
int mc_snapshot(const struct mc_owner *owner, int *out);

/*
 * Gives the allocation back exactly once: the owner is emptied before the
 * release callback runs, and releases is incremented. Rejects an empty
 * owner, so a second release is refused.
 */
int mc_release(struct mc_owner *owner);

/* fd_move() semantics (both overlap directions) on bytes of length_bytes. */
enum fd_status mc_move(unsigned char *base, size_t length_bytes, size_t dst,
                       size_t src, size_t count);

struct mc_container
{
    unsigned prefix;
    int member;
};

struct mc_first
{
    int first;
    unsigned tag;
};

/*
 * Reads owner->member, but only if member is exactly the address of that
 * member inside owner (computed from the whole object's address and
 * offsetof). An unrelated int is rejected.
 */
int mc_member(const struct mc_container *owner, const int *member, int *out);

/*
 * Reads owner->first, but only if member is the address of that first
 * member of owner. Qualifiers are preserved.
 */
int mc_initial(const struct mc_first *owner, const int *member, int *out);
#endif
