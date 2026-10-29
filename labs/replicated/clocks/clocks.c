#include "clocks.h"
int rpc_make(uint64_t u, uint64_t i, struct rpc_clock *o) {
  (void)u;
  (void)i;
  (void)o;
  return RPC_UNIMPLEMENTED;
}
int rpc_step(const struct rpc_clock *l, const struct rpc_clock *r, uint64_t w, uint64_t a,
             unsigned p, struct rpc_clock *o) {
  (void)l;
  (void)r;
  (void)w;
  (void)a;
  (void)p;
  (void)o;
  return RPC_UNIMPLEMENTED;
}
int rpc_replace(const struct rpc_clock *l, uint64_t i, struct rpc_clock *o) {
  (void)l;
  (void)i;
  (void)o;
  return RPC_UNIMPLEMENTED;
}
int rpc_order(const struct rpc_vector *a, const struct rpc_vector *b, int *o) {
  (void)a;
  (void)b;
  (void)o;
  return RPC_UNIMPLEMENTED;
}
int rpc_encode(const struct rpc_clock *c, unsigned char o[160]) {
  (void)c;
  (void)o;
  return RPC_UNIMPLEMENTED;
}
int rpc_decode(uint64_t u, const unsigned char i[160], struct rpc_clock *o) {
  (void)u;
  (void)i;
  (void)o;
  return RPC_UNIMPLEMENTED;
}
int rpc_save(store *p, const struct rpc_clock *c) {
  (void)p;
  (void)c;
  return RPC_UNIMPLEMENTED;
}
int rpc_load(store *p, uint64_t u, struct rpc_clock *o) {
  (void)p;
  (void)u;
  (void)o;
  return RPC_UNIMPLEMENTED;
}
int rpc_step_durable(store *p, const struct rpc_clock *l, const struct rpc_clock *r, uint64_t w,
                     uint64_t a, unsigned y, struct rpc_clock *o) {
  (void)p;
  (void)l;
  (void)r;
  (void)w;
  (void)a;
  (void)y;
  (void)o;
  return RPC_UNIMPLEMENTED;
}
int rpc_replace_durable(store *p, const struct rpc_clock *l, uint64_t i, struct rpc_clock *o) {
  (void)p;
  (void)l;
  (void)i;
  (void)o;
  return RPC_UNIMPLEMENTED;
}
