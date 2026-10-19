#ifndef CMASTERY_ADV_H
#define CMASTERY_ADV_H
#include "adt.h"
typedef enum {ADV_OK,ADV_INVALID,ADV_RANGE,ADV_ALIAS,ADV_LAYOUT,ADV_OVERFLOW} AdvStatus;
typedef struct {size_t required;unsigned injective;} AdvGeometry;
typedef struct {uint32_t *data;size_t capacity,rows,columns,row_stride,column_stride;} AdvView;
typedef enum {ADV_SCALAR,ADV_VECTOR,ADV_AUTO} AdvMode;
typedef struct {uint32_t sum;AdvMode backend;size_t vector_blocks,scalar_elements;} AdvReduction;
/* Arithmetic only: no backing object or pointer is invented by this result. */
AdvStatus adv_geometry(size_t capacity,size_t rows,size_t columns,size_t row_stride,size_t column_stride,AdvGeometry *);
/* Borrow live writable storage; runtime injectivity within one view is not an
   exclusive borrow proof across other aliases. Operations externally serialized. */
AdvStatus adv_view(uint32_t *,size_t,size_t,size_t,size_t,size_t,AdvView *);
ADView adv_readonly(AdvView);
AdvStatus adv_set(AdvView,size_t row,size_t column,uint32_t);
AdvStatus adv_transpose(AdvView,AdvView *);
AdvStatus adv_subview(AdvView,size_t row,size_t column,size_t rows,size_t columns,AdvView *);
/* Exact mathematical sum must fit uint32. Output is disjoint and unchanged on
   refusal. VECTOR needs column_stride1; AUTO falls back for other layouts.
   Platform-specific vector4 implementation is separate from the ISO C core. */
AdvStatus adv_reduce(ADView,AdvMode,AdvReduction *);
/* Backend primitive: exactly four live uint32 values, no16-byte alignment
   requirement. Original GNU vector extension, not ISO C or a library reduction. */
uint64_t adv_vector4(const uint32_t *);
#endif
