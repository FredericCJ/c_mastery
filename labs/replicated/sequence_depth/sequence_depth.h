#ifndef CMASTERY_REPLICATED_SEQUENCE_DEPTH_H
#define CMASTERY_REPLICATED_SEQUENCE_DEPTH_H
#include "replicated.h"
enum {
  RPS_OK,
  RPS_INVALID,
  RPS_FULL,
  RPS_CONFLICT,
  RPS_GAP,
  RPS_STALE,
  RPS_RANGE,
  RPS_BUSY,
  RPS_UNIMPLEMENTED
};
struct rps_item {
  unsigned id, parent;
  uint32_t scalar;
  unsigned inserted, removed;
};
struct rps_tree {
  uint64_t epoch;
  unsigned mode, count, waiting_count;
  struct rps_item items[16];
  unsigned waiting[16];
  struct rp_sequence topology;
};
int rps_init(struct rps_tree *tree, uint64_t epoch, unsigned placeholder_mode);
int rps_insert(struct rps_tree *tree, uint64_t epoch, unsigned id, unsigned parent,
               uint32_t scalar);
int rps_remove(struct rps_tree *tree, uint64_t epoch, unsigned id);
int rps_render(const struct rps_tree *tree, unsigned char *out, size_t capacity,
               size_t *length);
int rps_snapshot(const struct rps_tree *tree, unsigned char out[368]);
int rps_restore(const unsigned char wire[368], struct rps_tree *out);
/* Original local coordinator: all three controls must be distinct, quiescent,
   and have the same fully admitted state; outstanding counts copied messages.
 */
int rps_compact(struct rps_tree *a, struct rps_tree *b, struct rps_tree *c, size_t outstanding);

struct rps_op {
  unsigned kind, position;
  uint32_t scalar;
};
struct rps_request {
  uint64_t epoch;
  unsigned base, site, sequence, vector[3];
  struct rps_op op;
};
struct rps_commit {
  uint64_t epoch;
  unsigned revision, site, sequence, before[3];
  struct rps_op op;
};
struct rps_server {
  uint64_t epoch;
  unsigned count, lengths[17], vectors[17][3];
  uint32_t documents[17][32];
  struct rps_request originals[16];
  struct rps_commit commits[16];
};
struct rps_client {
  uint64_t epoch;
  unsigned revision, length, vector[3], site, pending;
  struct rps_request request;
  uint32_t document[32];
  struct rps_commit commits[16];
};
int rps_server_init(struct rps_server *server, uint64_t epoch);
int rps_client_init(struct rps_client *client, uint64_t epoch, unsigned site);
/* kind1 inserts one scalar at gap0..length; kind2 deletes position0..length-1.
 */
int rps_prepare(struct rps_client *client, unsigned kind, unsigned position,
                uint32_t scalar, struct rps_request *out);
int rps_submit(struct rps_server *server, const struct rps_request *request, struct rps_commit *out);
int rps_deliver(struct rps_client *client, const struct rps_commit *commit);
#endif
