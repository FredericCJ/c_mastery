#ifndef CMASTERY_OWNERSHIP_STRIDES_H
#define CMASTERY_OWNERSHIP_STRIDES_H
#include "ownership.h"
typedef struct {size_t offset,rows,columns,row_stride,column_stride;} OsvShape;
typedef struct {OsvShape shape;uint32_t serial;unsigned active,writer,retired;} OsvLoan;
typedef struct OsvBook {OsvLoan loan[4];size_t extent;} OsvBook;
typedef struct {const OsvBook *owner;unsigned slot;uint32_t serial;} OsvToken;
/* Fresh zero book, then extent assignment. Shapes touch at most64 distinct cells. */
OwStatus osv_indices(size_t,OsvShape,size_t out[64],size_t *count);
OwStatus osv_acquire(OsvBook *,OsvShape,int writer,OsvToken *);
OwStatus osv_release(OsvBook *,OsvToken);
/* axis0 splits rows; axis1 splits columns. Failure preserves book and outputs. */
OwStatus osv_split(OsvBook *,OsvToken,unsigned axis,size_t cut,OsvToken out[2]);
OwStatus osv_read(const OsvBook *,OsvToken,const int32_t *,size_t,size_t,size_t,int32_t *);
OwStatus osv_write(OsvBook *,OsvToken,int32_t *,size_t,size_t,size_t,int32_t);
#endif
