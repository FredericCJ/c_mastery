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
typedef enum {
  BL_OK,
  BL_INVALID,
  BL_FULL,
  BL_NEGATIVE,
  BL_ZERO_DIVISOR,
  BL_ALLOCATION
} BlStatus;
typedef struct {
  const uint16_t *digits;
  size_t used;
} BlView;
typedef struct {
  uint16_t *digits;
  size_t capacity, used;
} BlOutput;
typedef struct {
  int sign;
  BlView magnitude;
} BlSigned;
typedef struct {
  int sign;
  BlOutput magnitude;
} BlSignedOutput;
/* All output/scratch/control objects are disjoint; input views may alias each
   other. Capacity measures actual uint16_t objects, even with 8-bit digits. */
BlStatus bl_add(BlView, BlView, BlOutput *, uint16_t *scratch,
                size_t scratch_n);
BlStatus bl_sub(BlView, BlView, BlOutput *, uint16_t *scratch,
                size_t scratch_n);
BlStatus bl_mul(BlView, BlView, BlOutput *, uint16_t *scratch,
                size_t scratch_n);
BlStatus bl_div(BlView, BlView, BlOutput *quotient, BlOutput *remainder,
                uint16_t *scratch, size_t scratch_n);
BlStatus bl_signed_add(BlSigned, BlSigned, BlSignedOutput *, uint16_t *,
                       size_t);
BlStatus bl_signed_div(BlSigned, BlSigned, BlSignedOutput *, BlSignedOutput *,
                       uint16_t *, size_t);
/* Allocators return storage for n actual uint16_t objects, aligned accordingly.
   Callbacks are nonreentrant; free cannot fail. No callback may retain storage.
 */
typedef uint16_t *(*BlAllocate)(void *, size_t n);
typedef void (*BlFree)(void *, uint16_t *, size_t n);
typedef struct {
  uint16_t *digits;
  size_t used, capacity;
  void *context;
  BlAllocate allocate;
  BlFree release;
} BlOwner;
BlStatus bl_owner_init(BlOwner *, void *, BlAllocate, BlFree);
BlStatus bl_owner_set(BlOwner *, BlView, size_t reserve);
void bl_owner_drop(BlOwner *);
#endif
