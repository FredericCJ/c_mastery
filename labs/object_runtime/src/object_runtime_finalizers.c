#include "object_runtime_finalizers.h"
void orf_init(ORFHeap *h) { if(h) *h=(ORFHeap){0}; }
ORStatus orf_new(ORFHeap *h, ORFFinalizer cb, void *ctx, int resource, int fail, ORFHandle *out) { (void)h;(void)cb;(void)ctx;(void)resource;(void)fail;(void)out; return OR_UNIMPLEMENTED; }
ORStatus orf_root(ORFHeap *h, ORFHandle a, int present) { (void)h;(void)a;(void)present; return OR_UNIMPLEMENTED; }
ORStatus orf_edge(ORFHeap *h, ORFHandle a, unsigned field, ORFHandle b) { (void)h;(void)a;(void)field;(void)b; return OR_UNIMPLEMENTED; }
ORStatus orf_pair(ORFHeap *h, unsigned index, ORFHandle a, ORFHandle b) { (void)h;(void)index;(void)a;(void)b; return OR_UNIMPLEMENTED; }
ORStatus orf_weak(ORFHeap *h, unsigned index, ORFHandle a) { (void)h;(void)index;(void)a; return OR_UNIMPLEMENTED; }
ORStatus orf_collect(ORFHeap *h, uint16_t *out) { (void)h;(void)out; return OR_UNIMPLEMENTED; }
ORStatus orf_run_one(ORFHeap *h, ORFHandle *out, ORStatus *status) { (void)h;(void)out;(void)status; return OR_UNIMPLEMENTED; }
ORStatus orf_close_resource(ORFHeap *h, ORFHandle a) { (void)h;(void)a; return OR_UNIMPLEMENTED; }
ORStatus orf_cleanup(ORFHeap *h, unsigned budget, unsigned *out) { (void)h;(void)budget;(void)out; return OR_UNIMPLEMENTED; }
ORStatus orf_close(ORFHeap *h) { (void)h; return OR_UNIMPLEMENTED; }
