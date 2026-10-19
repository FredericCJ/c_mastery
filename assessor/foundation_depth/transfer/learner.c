#include "transfer.h"
/* Intentionally incomplete: release only with the selected transfer brief. */
FDStep fd_step_value(int v) { (void)v; return (FDStep){FD_NOT_IMPLEMENTED,0,0}; }
FDStatus fd_layout(size_t h,intmax_t a,size_t aw,intmax_t b,size_t bw,size_t l,FDLayout *o) {
    (void)h;(void)a;(void)aw;(void)b;(void)bw;(void)l;(void)o;return FD_NOT_IMPLEMENTED;
}
FDStatus fd_copy_segments(FDSegmented *c,size_t n,unsigned char *d,size_t cap) {
    (void)c;(void)n;(void)d;(void)cap;return FD_NOT_IMPLEMENTED;
}
FDStatus fd_owner_subview(const FDOwnerView *s,size_t o,size_t n,FDOwnerView *v) {
    (void)s;(void)o;(void)n;(void)v;return FD_NOT_IMPLEMENTED;
}
FDStatus fd_read_tagged(FDCursor *s,uint16_t *o) { (void)s;(void)o;return FD_NOT_IMPLEMENTED; }
FDStatus fd_least_encode(FDLeastRecord r,unsigned char *o,size_t n) {
    (void)r;(void)o;(void)n;return FD_NOT_IMPLEMENTED;
}
FDStatus fd_least_decode(const unsigned char *w,size_t n,FDLeastRecord *o) {
    (void)w;(void)n;(void)o;return FD_NOT_IMPLEMENTED;
}
FDStatus fd_encode_v2(uintmax_t c,intmax_t v,unsigned char *o,size_t n) {
    (void)c;(void)v;(void)o;(void)n;return FD_NOT_IMPLEMENTED;
}
FDStatus fd_decode_v2(const unsigned char *w,size_t n,uintmax_t *c,intmax_t *v) {
    (void)w;(void)n;(void)c;(void)v;return FD_NOT_IMPLEMENTED;
}
void *fd_typed_allocate(void *c,size_t n,size_t a) { (void)c;(void)n;(void)a;return 0; }
void fd_typed_release(void *c,void *v) { (void)c;(void)v; }
FDStatus fd_slot_transfer(FDSlot *d,FDSlot *s) { (void)d;(void)s;return FD_NOT_IMPLEMENTED; }
FDStatus fd_stride_move(unsigned char *b,size_t p,size_t l,size_t stride,size_t d,size_t s,size_t n) {
    (void)b;(void)p;(void)l;(void)stride;(void)d;(void)s;(void)n;return FD_NOT_IMPLEMENTED;
}
FDStatus fd_compact(FDStreamView *s) { (void)s;return FD_NOT_IMPLEMENTED; }
FDStatus fd_inplace_xor(unsigned char *b,size_t n,unsigned char m) { (void)b;(void)n;(void)m;return FD_NOT_IMPLEMENTED; }
FDStatus fd_pool_borrow(const FDPool *p,FDHandle h,const FDValue **o) { (void)p;(void)h;(void)o;return FD_NOT_IMPLEMENTED; }
void fd_registry_init(FDRegistry *r) { (void)r; }
FDStatus fd_registry_bind(FDRegistry *r,size_t s,FDPool *p) { (void)r;(void)s;(void)p;return FD_NOT_IMPLEMENTED; }
FDStatus fd_registry_unbind(FDRegistry *r,size_t s) { (void)r;(void)s;return FD_NOT_IMPLEMENTED; }
FDStatus fd_registry_relocate(FDRegistry *r,size_t s,FDPool *p) { (void)r;(void)s;(void)p;return FD_NOT_IMPLEMENTED; }
FDStatus fd_registry_acquire(FDRegistry *r,size_t s,FDValue v,FDRegistryHandle *o) {
    (void)r;(void)s;(void)v;(void)o;return FD_NOT_IMPLEMENTED;
}
FDStatus fd_registry_get(const FDRegistry *r,FDRegistryHandle h,FDValue *o) {
    (void)r;(void)h;(void)o;return FD_NOT_IMPLEMENTED;
}
void fd_frame_cancel(FDFrame *f) { (void)f; }
FDStatus fd_effect(FDEffect e,const unsigned char *w,size_t n,int a,int32_t *c,int32_t *o) {
    (void)e;(void)w;(void)n;(void)a;(void)c;(void)o;return FD_NOT_IMPLEMENTED;
}
FDStatus fd_frame_feed(FDFrame *f,const unsigned char *b,size_t n,int a,int32_t *c,int *done) {
    (void)f;(void)b;(void)n;(void)a;(void)c;(void)done;return FD_NOT_IMPLEMENTED;
}
