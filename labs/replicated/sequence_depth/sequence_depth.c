#include "sequence_depth.h"
int rps_init(struct rps_tree *tree, uint64_t epoch, unsigned placeholder_mode) {
  (void)tree;
  (void)epoch;
  (void)placeholder_mode;
  return RPS_UNIMPLEMENTED;
}
int rps_insert(struct rps_tree *tree, uint64_t epoch, unsigned id, unsigned parent,
               uint32_t scalar) {
  (void)tree;
  (void)epoch;
  (void)id;
  (void)parent;
  (void)scalar;
  return RPS_UNIMPLEMENTED;
}
int rps_remove(struct rps_tree *tree, uint64_t epoch, unsigned id) {
  (void)tree;
  (void)epoch;
  (void)id;
  return RPS_UNIMPLEMENTED;
}
int rps_render(const struct rps_tree *tree, unsigned char *out, size_t capacity,
               size_t *length) {
  (void)tree;
  (void)out;
  (void)capacity;
  (void)length;
  return RPS_UNIMPLEMENTED;
}
int rps_snapshot(const struct rps_tree *tree, unsigned char out[368]) {
  (void)tree;
  (void)out;
  return RPS_UNIMPLEMENTED;
}
int rps_restore(const unsigned char wire[368], struct rps_tree *out) {
  (void)wire;
  (void)out;
  return RPS_UNIMPLEMENTED;
}
int rps_compact(struct rps_tree *a, struct rps_tree *b, struct rps_tree *c, size_t outstanding) {
  (void)a;
  (void)b;
  (void)c;
  (void)outstanding;
  return RPS_UNIMPLEMENTED;
}
int rps_server_init(struct rps_server *server, uint64_t epoch) {
  (void)server;
  (void)epoch;
  return RPS_UNIMPLEMENTED;
}
int rps_client_init(struct rps_client *client, uint64_t epoch, unsigned site) {
  (void)client;
  (void)epoch;
  (void)site;
  return RPS_UNIMPLEMENTED;
}
int rps_prepare(struct rps_client *client, unsigned kind, unsigned position,
                uint32_t scalar, struct rps_request *out) {
  (void)client;
  (void)kind;
  (void)position;
  (void)scalar;
  (void)out;
  return RPS_UNIMPLEMENTED;
}
int rps_submit(struct rps_server *server, const struct rps_request *request, struct rps_commit *out) {
  (void)server;
  (void)request;
  (void)out;
  return RPS_UNIMPLEMENTED;
}
int rps_deliver(struct rps_client *client, const struct rps_commit *commit) {
  (void)client;
  (void)commit;
  return RPS_UNIMPLEMENTED;
}
