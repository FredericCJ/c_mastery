#include "sequence_depth.h"
int rps_init(RpsTree *tree, uint64_t epoch, unsigned placeholder_mode) {
  (void)tree;
  (void)epoch;
  (void)placeholder_mode;
  return RPS_UNIMPLEMENTED;
}
int rps_insert(RpsTree *tree, uint64_t epoch, unsigned id, unsigned parent,
               uint32_t scalar) {
  (void)tree;
  (void)epoch;
  (void)id;
  (void)parent;
  (void)scalar;
  return RPS_UNIMPLEMENTED;
}
int rps_remove(RpsTree *tree, uint64_t epoch, unsigned id) {
  (void)tree;
  (void)epoch;
  (void)id;
  return RPS_UNIMPLEMENTED;
}
int rps_render(const RpsTree *tree, unsigned char *out, size_t capacity,
               size_t *length) {
  (void)tree;
  (void)out;
  (void)capacity;
  (void)length;
  return RPS_UNIMPLEMENTED;
}
int rps_snapshot(const RpsTree *tree, unsigned char out[368]) {
  (void)tree;
  (void)out;
  return RPS_UNIMPLEMENTED;
}
int rps_restore(const unsigned char wire[368], RpsTree *out) {
  (void)wire;
  (void)out;
  return RPS_UNIMPLEMENTED;
}
int rps_compact(RpsTree *a, RpsTree *b, RpsTree *c, size_t outstanding) {
  (void)a;
  (void)b;
  (void)c;
  (void)outstanding;
  return RPS_UNIMPLEMENTED;
}
int rps_server_init(RpsServer *server, uint64_t epoch) {
  (void)server;
  (void)epoch;
  return RPS_UNIMPLEMENTED;
}
int rps_client_init(RpsClient *client, uint64_t epoch, unsigned site) {
  (void)client;
  (void)epoch;
  (void)site;
  return RPS_UNIMPLEMENTED;
}
int rps_prepare(RpsClient *client, unsigned kind, unsigned position,
                uint32_t scalar, RpsRequest *out) {
  (void)client;
  (void)kind;
  (void)position;
  (void)scalar;
  (void)out;
  return RPS_UNIMPLEMENTED;
}
int rps_submit(RpsServer *server, const RpsRequest *request, RpsCommit *out) {
  (void)server;
  (void)request;
  (void)out;
  return RPS_UNIMPLEMENTED;
}
int rps_deliver(RpsClient *client, const RpsCommit *commit) {
  (void)client;
  (void)commit;
  return RPS_UNIMPLEMENTED;
}
