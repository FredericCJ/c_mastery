#include "clocks.h"
int rpc_make(uint64_t u, uint64_t i, RpcClock *o) {
  (void)u;
  (void)i;
  (void)o;
  return RPC_UNIMPLEMENTED;
}
int rpc_step(const RpcClock *l, const RpcClock *r, uint64_t w, uint64_t a,
             unsigned p, RpcClock *o) {
  (void)l;
  (void)r;
  (void)w;
  (void)a;
  (void)p;
  (void)o;
  return RPC_UNIMPLEMENTED;
}
int rpc_replace(const RpcClock *l, uint64_t i, RpcClock *o) {
  (void)l;
  (void)i;
  (void)o;
  return RPC_UNIMPLEMENTED;
}
int rpc_order(const RpcVector *a, const RpcVector *b, int *o) {
  (void)a;
  (void)b;
  (void)o;
  return RPC_UNIMPLEMENTED;
}
int rpc_encode(const RpcClock *c, unsigned char o[160]) {
  (void)c;
  (void)o;
  return RPC_UNIMPLEMENTED;
}
int rpc_decode(uint64_t u, const unsigned char i[160], RpcClock *o) {
  (void)u;
  (void)i;
  (void)o;
  return RPC_UNIMPLEMENTED;
}
int rpc_save(store *p, const RpcClock *c) {
  (void)p;
  (void)c;
  return RPC_UNIMPLEMENTED;
}
int rpc_load(store *p, uint64_t u, RpcClock *o) {
  (void)p;
  (void)u;
  (void)o;
  return RPC_UNIMPLEMENTED;
}
int rpc_step_durable(store *p, const RpcClock *l, const RpcClock *r, uint64_t w,
                     uint64_t a, unsigned y, RpcClock *o) {
  (void)p;
  (void)l;
  (void)r;
  (void)w;
  (void)a;
  (void)y;
  (void)o;
  return RPC_UNIMPLEMENTED;
}
int rpc_replace_durable(store *p, const RpcClock *l, uint64_t i, RpcClock *o) {
  (void)p;
  (void)l;
  (void)i;
  (void)o;
  return RPC_UNIMPLEMENTED;
}
