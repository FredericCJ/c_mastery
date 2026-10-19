#ifndef CMASTERY_DISPATCH_CAPTURE_H
#define CMASTERY_DISPATCH_CAPTURE_H
#include "dispatch.h"
#include "ownership.h"
DAllocator ds_counted_bytes(OwAllocator *);
int ds_capture_i32(DSErased *,const int32_t *,size_t,void (*)(void *),void *);
int ds_borrowed_i32(const int32_t *,size_t,int,int *);
typedef struct {union {int32_t local[4];int32_t *heap;} data;size_t count;unsigned spilled;V4Allocator allocator;} DSTypedCapture;
void ds_typed_init(DSTypedCapture *,V4Allocator);
int ds_typed_assign(DSTypedCapture *,const int32_t *,size_t);
int ds_typed_call(const DSTypedCapture *,int,int *);
int ds_typed_move(DSTypedCapture *,DSTypedCapture *);
void ds_typed_drop(DSTypedCapture *);
#endif
