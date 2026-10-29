#ifndef CORPUS_LIMBS_H
#define CORPUS_LIMBS_H
#include <stddef.h>
#include <stdint.h>
#ifndef BL_BITS
#define BL_BITS 16
#endif
#if BL_BITS != 8 && BL_BITS != 16
#error "BL_BITS must be 8 or 16"
#endif
#define BL_MASK ((uint32_t)((UINT32_C(1) << BL_BITS) - 1))

enum bl_status
{
    BL_OK,
    BL_INVALID,
    BL_FULL,
    BL_NEGATIVE,
    BL_ZERO_DIVISOR,
    BL_ALLOCATION
};

struct bl_view
{
    const uint16_t *digits;
    size_t used;
};

struct bl_output
{
    uint16_t *digits;
    size_t capacity, used;
};

struct bl_signed
{
    int sign;
    struct bl_view magnitude;
};

struct bl_signed_output
{
    int sign;
    struct bl_output magnitude;
};
/* All output/scratch/control objects are disjoint; input views may alias each
   other. Capacity measures actual uint16_t objects, even with 8-bit digits. */
enum bl_status bl_add(struct bl_view, struct bl_view, struct bl_output *,
                      uint16_t *scratch, size_t scratch_n);
enum bl_status bl_sub(struct bl_view, struct bl_view, struct bl_output *,
                      uint16_t *scratch, size_t scratch_n);
enum bl_status bl_mul(struct bl_view, struct bl_view, struct bl_output *,
                      uint16_t *scratch, size_t scratch_n);
enum bl_status bl_div(struct bl_view, struct bl_view,
                      struct bl_output *quotient, struct bl_output *remainder,
                      uint16_t *scratch, size_t scratch_n);
enum bl_status bl_signed_add(struct bl_signed, struct bl_signed,
                             struct bl_signed_output *, uint16_t *, size_t);
enum bl_status bl_signed_div(struct bl_signed, struct bl_signed,
                             struct bl_signed_output *,
                             struct bl_signed_output *, uint16_t *, size_t);
/* Allocators return storage for n actual uint16_t objects, aligned accordingly.
   Callbacks are nonreentrant; free cannot fail. No callback may retain storage.
 */
typedef uint16_t *(*bl_allocate_fn)(void *, size_t n);
typedef void (*bl_free_fn)(void *, uint16_t *, size_t n);

struct bl_owner
{
    uint16_t *digits;
    size_t used, capacity;
    void *context;
    bl_allocate_fn allocate;
    bl_free_fn release;
};
enum bl_status bl_owner_init(struct bl_owner *, void *, bl_allocate_fn,
                             bl_free_fn);
enum bl_status bl_owner_set(struct bl_owner *, struct bl_view, size_t reserve);
void bl_owner_drop(struct bl_owner *);
#endif
