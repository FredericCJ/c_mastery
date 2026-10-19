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
typedef struct {
  unsigned id, parent;
  uint32_t scalar;
  unsigned inserted, removed;
} RpsItem;
typedef struct {
  uint64_t epoch;
  unsigned mode, count, waiting_count;
  RpsItem items[16];
  unsigned waiting[16];
  RPSequence topology;
} RpsTree;
int rps_init(RpsTree *tree, uint64_t epoch, unsigned placeholder_mode);
int rps_insert(RpsTree *tree, uint64_t epoch, unsigned id, unsigned parent,
               uint32_t scalar);
int rps_remove(RpsTree *tree, uint64_t epoch, unsigned id);
int rps_render(const RpsTree *tree, unsigned char *out, size_t capacity,
               size_t *length);
int rps_snapshot(const RpsTree *tree, unsigned char out[368]);
int rps_restore(const unsigned char wire[368], RpsTree *out);
/* Original local coordinator: all three controls must be distinct, quiescent,
   and have the same fully admitted state; outstanding counts copied messages.
 */
int rps_compact(RpsTree *a, RpsTree *b, RpsTree *c, size_t outstanding);

typedef struct {
  unsigned kind, position;
  uint32_t scalar;
} RpsOp;
typedef struct {
  uint64_t epoch;
  unsigned base, site, sequence, vector[3];
  RpsOp op;
} RpsRequest;
typedef struct {
  uint64_t epoch;
  unsigned revision, site, sequence, before[3];
  RpsOp op;
} RpsCommit;
typedef struct {
  uint64_t epoch;
  unsigned count, lengths[17], vectors[17][3];
  uint32_t documents[17][32];
  RpsRequest originals[16];
  RpsCommit commits[16];
} RpsServer;
typedef struct {
  uint64_t epoch;
  unsigned revision, length, vector[3], site, pending;
  RpsRequest request;
  uint32_t document[32];
  RpsCommit commits[16];
} RpsClient;
int rps_server_init(RpsServer *server, uint64_t epoch);
int rps_client_init(RpsClient *client, uint64_t epoch, unsigned site);
/* kind1 inserts one scalar at gap0..length; kind2 deletes position0..length-1.
 */
int rps_prepare(RpsClient *client, unsigned kind, unsigned position,
                uint32_t scalar, RpsRequest *out);
int rps_submit(RpsServer *server, const RpsRequest *request, RpsCommit *out);
int rps_deliver(RpsClient *client, const RpsCommit *commit);
#endif
