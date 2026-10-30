#ifndef OS_ALIAS_H
#define OS_ALIAS_H
#include "depth.h"

/* An injected allocator plus the metadata it promises: every block it hands
 * out holds at least capacity bytes and is aligned to alignment bytes. */
struct os_allocator
{
    struct fd_allocator allocator;
    size_t capacity, alignment;
};

struct os_wide
{
    _Alignas(64) int value;
    unsigned char *resource;
};

typedef int (*os_drop_fn)(void *, struct os_wide *);

/* Slot life cycle: EMPTY -> LIVE <-> BORROWED; CLEANUP while a destructor
 * that failed is awaiting retry. */
enum os_state
{
    OS_EMPTY,
    OS_LIVE,
    OS_BORROWED,
    OS_CLEANUP
};

struct os_wide_slot
{
    struct os_wide *value;
    struct os_allocator allocation;
    os_drop_fn drop;
    void *context;
    enum os_state state;
};

/*
 * Contract conventions for every function below.
 *
 * Return value: 0 on success and -1 on any rejected request unless a comment
 * says otherwise. A rejected request leaves every output and every control
 * object as it was, except for the explicit CLEANUP state of
 * os_wide_reset(). Pointer arguments must be NULL or point to live, correctly
 * typed objects; an arbitrary address is never validated. Callbacks must be
 * truthful and must not re-enter these functions. Nothing here blocks.
 */

/*
 * Admits the allocator's declared capacity and alignment (bytes) for one
 * struct fd_value, then acquires the slot through fd_slot_acquire().
 * Ownership: on success the slot owns one allocation from the allocator.
 */
int os_acquire(struct fd_slot *slot, struct os_allocator allocator);

/*
 * Moves an empty slot to OS_LIVE with a typed initial value. The allocator
 * must admit sizeof(struct os_wide) at _Alignof(struct os_wide); drop must
 * not be NULL. Ownership: on success the slot owns the allocation and the
 * drop/context pair; on failure nothing was kept.
 */
int os_wide_acquire(struct os_wide_slot *slot, struct os_allocator allocator,
                    int value, os_drop_fn drop, void *context);

/*
 * Exclusive borrow of a live value: the slot must be OS_LIVE, becomes
 * OS_BORROWED, and *out receives the pointer.
 */
int os_wide_borrow(struct os_wide_slot *slot, const struct os_wide **out);

/*
 * Consumes the one outstanding borrow. *borrowed must equal the pointer
 * lent by os_wide_borrow(); on success it is set to NULL and the slot is
 * OS_LIVE again.
 */
int os_wide_return(struct os_wide_slot *slot, const struct os_wide **borrowed);

/*
 * Destroys the value and releases the allocation exactly once.
 * An empty slot succeeds without effect. A borrowed slot is rejected with
 * no callback. Otherwise the slot enters OS_CLEANUP and drop runs; if drop
 * reports failure the result is -1, the allocation and everything drop
 * needs stay owned by the slot, and a later call retries. Only a successful
 * drop frees the allocation and returns the slot to OS_EMPTY.
 */
int os_wide_reset(struct os_wide_slot *slot);

/*
 * Copies the complete object representation of *source, by character type,
 * into allocated storage. allocated must be sized and aligned for
 * struct fd_value and disjoint from *source.
 */
int os_clone(void *allocated, const struct fd_value *source);

/*
 * Decodes exactly eight octets: a big-endian unsigned 32-bit value no larger
 * than INT32_MAX, then a big-endian unsigned 32-bit serial. A different
 * count, or a value above INT32_MAX, is rejected and *out is untouched.
 */
int os_wire(const unsigned char *bytes, size_t count, struct fd_value *out);

/* Returns value after copying its representation through a character view. */
uint32_t os_host_representation(uint32_t value);

/* Big-endian decode of four readable octets; the caller guarantees four. */
uint32_t os_wire_integer(const unsigned char *bytes);

/*
 * The next four functions compute dst[i] = src[i] ^ mask for i < count
 * (os_shared: dst[i] = left[i] ^ right[i]). They differ only in how the
 * pointers are qualified: restrict on the parameters, none, restrict on
 * block-scope copies, and two read-only restricted inputs. For count > 0 the
 * destination range must not overlap an input range; the two inputs of
 * os_shared may be the same object because neither is modified.
 */
void os_transform(unsigned char *restrict dst,
                  const unsigned char *restrict src, size_t count,
                  unsigned char mask);
void os_ordinary(unsigned char *dst, const unsigned char *src, size_t count,
                 unsigned char mask);
void os_nested(unsigned char *restrict dst, const unsigned char *restrict src,
               size_t count, unsigned char mask);
void os_shared(unsigned char *restrict dst, const unsigned char *restrict left,
               const unsigned char *restrict right, size_t count);

/*
 * XOR-transforms base[src, src + count) into base[dst, dst + count) inside
 * one array of length_bytes. Offsets and count are checked as integers before
 * any pointer is formed. Empty ranges whose offsets lie within length_bytes
 * succeed; base may be NULL only when length_bytes is 0.
 * Identical ranges are accepted only when inplace is nonzero; partially
 * overlapping ranges and out-of-bounds ranges are rejected unchanged.
 */
int os_partition(unsigned char *base, size_t length_bytes, size_t dst,
                 size_t src, size_t count, unsigned char mask, int inplace);

/* fd_move() with snapshot semantics for overlapping ranges. */
enum fd_status os_move(unsigned char *base, size_t length_bytes, size_t dst,
                       size_t src, size_t count);

struct os_container
{
    uint32_t prefix;
    int member;
};

/*
 * Reads owner->member through the owner, but only when member is exactly the
 * address of that member. Both pointers must refer to live objects.
 */
int os_member(struct os_container *owner, int *member, int *out);
#ifdef UINTPTR_MAX
/*
 * Optional experiment: stores a two-bit tag (0..3) in the low bits of the
 * owner's address, recovers the address and checks it. *representation gets
 * the tagged integer, *out the member value. Rejected if the address is not
 * four-byte aligned.
 */
int os_tag_roundtrip(struct os_container *owner, unsigned tag,
                     uintptr_t *representation, int *out);
#endif
#endif
