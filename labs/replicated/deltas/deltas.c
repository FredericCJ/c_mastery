#include "deltas.h"
int rpd_set_add(const RpdSet *state, unsigned writer, unsigned value,
                RpdSet *out, RpdSet *delta) {
  (void)state;
  (void)writer;
  (void)value;
  (void)out;
  (void)delta;
  return RPD_UNIMPLEMENTED;
}
int rpd_set_remove(const RpdSet *state, unsigned value, RpdSet *out,
                   RpdSet *delta) {
  (void)state;
  (void)value;
  (void)out;
  (void)delta;
  return RPD_UNIMPLEMENTED;
}
int rpd_set_join(const RpdSet *state, const RpdSet *delta, RpdSet *out) {
  (void)state;
  (void)delta;
  (void)out;
  return RPD_UNIMPLEMENTED;
}
int rpd_contains(const RpdSet *state, unsigned value) {
  (void)state;
  (void)value;
  return RPD_UNIMPLEMENTED;
}
int rpd_encode(const RpdState *state, unsigned char out[128]) {
  (void)state;
  (void)out;
  return RPD_UNIMPLEMENTED;
}
int rpd_decode(const unsigned char wire[128], RpdState *out) {
  (void)wire;
  (void)out;
  return RPD_UNIMPLEMENTED;
}
int rpd_create(store *pages, uint64_t uuid, unsigned self, RpdReplica *out) {
  (void)pages;
  (void)uuid;
  (void)self;
  (void)out;
  return RPD_UNIMPLEMENTED;
}
int rpd_open(store *pages, uint64_t uuid, unsigned self, RpdReplica *out) {
  (void)pages;
  (void)uuid;
  (void)self;
  (void)out;
  return RPD_UNIMPLEMENTED;
}
int rpd_local(RpdReplica *replica, unsigned add, unsigned value) {
  (void)replica;
  (void)add;
  (void)value;
  return RPD_UNIMPLEMENTED;
}
int rpd_send(RpdReplica *replica, unsigned peer, uint64_t peer_epoch,
             RpdPacket *out) {
  (void)replica;
  (void)peer;
  (void)peer_epoch;
  (void)out;
  return RPD_UNIMPLEMENTED;
}
int rpd_receive(RpdReplica *replica, const RpdPacket *packet, RpdAck *out) {
  (void)replica;
  (void)packet;
  (void)out;
  return RPD_UNIMPLEMENTED;
}
int rpd_ack(RpdReplica *replica, uint64_t current_peer_epoch,
            const RpdAck *ack) {
  (void)replica;
  (void)current_peer_epoch;
  (void)ack;
  return RPD_UNIMPLEMENTED;
}
