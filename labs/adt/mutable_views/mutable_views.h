#ifndef CMASTERY_ADV_H
#define CMASTERY_ADV_H
#include "adt.h"
enum adv_status {ADV_OK,ADV_INVALID,ADV_RANGE,ADV_ALIAS,ADV_LAYOUT,ADV_OVERFLOW};
struct adv_geometry {size_t required;unsigned injective;};
struct adv_view {uint32_t *data;size_t capacity,rows,columns,row_stride,column_stride;};
enum adv_mode {ADV_SCALAR,ADV_VECTOR,ADV_AUTO};
struct adv_reduction {uint32_t sum;enum adv_mode backend;size_t vector_blocks,scalar_elements;};
/* Arithmetic only: no backing object or pointer is invented by this result. */
enum adv_status adv_geometry(size_t capacity,size_t rows,size_t columns,size_t row_stride,size_t column_stride,struct adv_geometry *);
/* Borrow live writable storage; runtime injectivity within one view is not an
   exclusive borrow proof across other aliases. Operations externally serialized. */
enum adv_status adv_view(uint32_t *,size_t,size_t,size_t,size_t,size_t,struct adv_view *);
struct ad_view adv_readonly(struct adv_view);
enum adv_status adv_set(struct adv_view,size_t row,size_t column,uint32_t);
enum adv_status adv_transpose(struct adv_view,struct adv_view *);
enum adv_status adv_subview(struct adv_view,size_t row,size_t column,size_t rows,size_t columns,struct adv_view *);
/* Exact mathematical sum must fit uint32. Output is disjoint and unchanged on
   refusal. VECTOR needs column_stride1; AUTO falls back for other layouts.
   Platform-specific vector4 implementation is separate from the ISO C core. */
enum adv_status adv_reduce(struct ad_view,enum adv_mode,struct adv_reduction *);
/* Backend primitive: exactly four live uint32 values, no16-byte alignment
   requirement. Original GNU vector extension, not ISO C or a library reduction. */
uint64_t adv_vector4(const uint32_t *);
#endif
