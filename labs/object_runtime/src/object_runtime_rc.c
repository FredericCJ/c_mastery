#include "object_runtime_rc.h"
void orr_init(ORRHeap *h){if(h)*h=(ORRHeap){0};}
int orr_invariant(const ORRHeap *h){(void)h;return 0;}
ORStatus orr_new(ORRHeap *h, ORRHandle *out) { (void)h;(void)out; return OR_UNIMPLEMENTED; }
ORStatus orr_retain(ORRHeap *h, ORRHandle a) { (void)h;(void)a; return OR_UNIMPLEMENTED; }
ORStatus orr_release(ORRHeap *h, ORRHandle a, int defer, uint16_t *out) { (void)h;(void)a;(void)defer;(void)out; return OR_UNIMPLEMENTED; }
ORStatus orr_store(ORRHeap *h, ORRHandle a, unsigned field, ORRHandle b, int defer, uint16_t *out) { (void)h;(void)a;(void)field;(void)b;(void)defer;(void)out; return OR_UNIMPLEMENTED; }
ORStatus orr_flush(ORRHeap *h, unsigned budget, unsigned *processed, uint16_t *out) { (void)h;(void)budget;(void)processed;(void)out; return OR_UNIMPLEMENTED; }
ORStatus orr_prepare(ORRHeap *h, ORRTrial *out) { (void)h;(void)out; return OR_UNIMPLEMENTED; }
ORStatus orr_commit(ORRHeap *h, const ORRTrial *t, uint16_t *out) { (void)h;(void)t;(void)out; return OR_UNIMPLEMENTED; }
ORStatus orr_close(ORRHeap *h) { (void)h; return OR_UNIMPLEMENTED; }
