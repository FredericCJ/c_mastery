#include "capture.h"
DAllocator ds_counted_bytes(OwAllocator *a){(void)a;return (DAllocator){0};}
int ds_capture_i32(DSErased *a,const int32_t *b,size_t c,void (*d)(void *),void *e){(void)a;(void)b;(void)c;(void)d;(void)e;return DS_FAILED;}
int ds_borrowed_i32(const int32_t *a,size_t b,int c,int *d){(void)a;(void)b;(void)c;(void)d;return DS_FAILED;}
void ds_typed_init(DSTypedCapture *a,V4Allocator b){(void)a;(void)b;}
int ds_typed_assign(DSTypedCapture *a,const int32_t *b,size_t c){(void)a;(void)b;(void)c;return DS_FAILED;}
int ds_typed_call(const DSTypedCapture *a,int b,int *c){(void)a;(void)b;(void)c;return DS_FAILED;}
int ds_typed_move(DSTypedCapture *a,DSTypedCapture *b){(void)a;(void)b;return DS_FAILED;}
void ds_typed_drop(DSTypedCapture *a){(void)a;}
