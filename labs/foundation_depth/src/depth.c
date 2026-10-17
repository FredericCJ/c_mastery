#include "depth.h"
/* These compiling stubs deliberately fail every exercise semantic gate.
 * Implement only the selected exercise, preserving the shared interface.
 * Full contracts are in CONTRACTS.md; no supplied stub is a worked solution. */
FDClass fd_verdict(FDCase q) { (void)q; return FD_UNKNOWN; }
FDStatus fd_repair_step(int *v, int *p) { (void)v; (void)p; return FD_NOT_IMPLEMENTED; }
FDStatus fd_extent(size_t o,size_t c,size_t w,size_t l,size_t *r) {
    (void)o;(void)c;(void)w;(void)l;(void)r;return FD_NOT_IMPLEMENTED;
}
FDStatus fd_from_signed(intmax_t v,size_t *r) { (void)v;(void)r;return FD_NOT_IMPLEMENTED; }
FDStatus fd_take(FDCursor *c,size_t n,const unsigned char **p) { (void)c;(void)n;(void)p;return FD_NOT_IMPLEMENTED; }
FDStatus fd_subview(const FDCursor *c,size_t o,size_t n,FDCursor *r) { (void)c;(void)o;(void)n;(void)r;return FD_NOT_IMPLEMENTED; }
FDStatus fd_encode(FDRecord r,unsigned char *p,size_t n) { (void)r;(void)p;(void)n;return FD_NOT_IMPLEMENTED; }
FDStatus fd_decode(const unsigned char *p,size_t n,FDRecord *r) { (void)p;(void)n;(void)r;return FD_NOT_IMPLEMENTED; }
FDStatus fd_slot_acquire(FDSlot *s,FDAllocator a) { (void)s;(void)a;return FD_NOT_IMPLEMENTED; }
FDStatus fd_slot_initialize(FDSlot *s,FDInitialize i,FDDestroy d,void *c) { (void)s;(void)i;(void)d;(void)c;return FD_NOT_IMPLEMENTED; }
FDStatus fd_slot_publish(FDSlot *s,const FDValue **r) { (void)s;(void)r;return FD_NOT_IMPLEMENTED; }
void fd_slot_reset(FDSlot *s) { (void)s; }
void fd_transform(unsigned char *restrict d,const unsigned char *restrict s,size_t n,unsigned char m) { (void)d;(void)s;(void)n;(void)m; }
FDStatus fd_move(unsigned char *b,size_t l,size_t d,size_t s,size_t n) { (void)b;(void)l;(void)d;(void)s;(void)n;return FD_NOT_IMPLEMENTED; }
void fd_pool_init(FDPool *p) { (void)p; }
FDStatus fd_pool_acquire(FDPool *p,FDValue v,FDHandle *r) { (void)p;(void)v;(void)r;return FD_NOT_IMPLEMENTED; }
FDStatus fd_pool_get(const FDPool *p,FDHandle h,FDValue *r) { (void)p;(void)h;(void)r;return FD_NOT_IMPLEMENTED; }
FDStatus fd_pool_release(FDPool *p,FDHandle h) { (void)p;(void)h;return FD_NOT_IMPLEMENTED; }
FDStatus fd_apply(const unsigned char *p,size_t n,int a,int32_t *r) { (void)p;(void)n;(void)a;(void)r;return FD_NOT_IMPLEMENTED; }
