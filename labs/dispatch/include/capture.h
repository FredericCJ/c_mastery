#ifndef CMASTERY_DISPATCH_CAPTURE_H
#define CMASTERY_DISPATCH_CAPTURE_H
#include "dispatch.h"
#include "ownership.h"
struct d_allocator ds_counted_bytes(struct ow_allocator *);
int ds_capture_i32(struct ds_erased *,const int32_t *,size_t,void (*)(void *),void *);
int ds_borrowed_i32(const int32_t *,size_t,int,int *);
struct ds_typed_capture {union {int32_t local[4];int32_t *heap;} data;size_t count;unsigned spilled;struct v4_allocator allocator;};
void ds_typed_init(struct ds_typed_capture *,struct v4_allocator);
int ds_typed_assign(struct ds_typed_capture *,const int32_t *,size_t);
int ds_typed_call(const struct ds_typed_capture *,int,int *);
int ds_typed_move(struct ds_typed_capture *,struct ds_typed_capture *);
void ds_typed_drop(struct ds_typed_capture *);
#endif
