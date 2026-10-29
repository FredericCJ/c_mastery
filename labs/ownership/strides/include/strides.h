#ifndef CMASTERY_OWNERSHIP_STRIDES_H
#define CMASTERY_OWNERSHIP_STRIDES_H
#include "ownership.h"
struct osv_shape {size_t offset,rows,columns,row_stride,column_stride;};
struct osv_loan {struct osv_shape shape;uint32_t serial;unsigned active,writer,retired;};
struct osv_book {struct osv_loan loan[4];size_t extent;};
struct osv_token {const struct osv_book *owner;unsigned slot;uint32_t serial;};
/* Fresh zero book, then extent assignment. Shapes touch at most64 distinct cells. */
enum ow_status osv_indices(size_t,struct osv_shape,size_t out[64],size_t *count);
enum ow_status osv_acquire(struct osv_book *,struct osv_shape,int writer,struct osv_token *);
enum ow_status osv_release(struct osv_book *,struct osv_token);
/* axis0 splits rows; axis1 splits columns. Failure preserves book and outputs. */
enum ow_status osv_split(struct osv_book *,struct osv_token,unsigned axis,size_t cut,struct osv_token out[2]);
enum ow_status osv_read(const struct osv_book *,struct osv_token,const int32_t *,size_t,size_t,size_t,int32_t *);
enum ow_status osv_write(struct osv_book *,struct osv_token,int32_t *,size_t,size_t,size_t,int32_t);
#endif
