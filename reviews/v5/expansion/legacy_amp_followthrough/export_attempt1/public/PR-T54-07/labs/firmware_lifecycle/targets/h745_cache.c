#include "h745_cache.h"
int fw_cache_range(uint32_t a,uint32_t n,enum FWCacheOp op,FWCacheEmit emit,void *ctx)
{ (void)a;(void)n;(void)op;(void)emit;(void)ctx;return 0; }
enum FWPeerResult fw_peer_step(FWPeer *p,unsigned c,uint32_t g,enum FWPeerOp op)
{ (void)p;(void)c;(void)g;(void)op;return H7_INVALID; }
