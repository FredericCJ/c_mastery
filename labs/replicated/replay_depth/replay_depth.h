#ifndef CMASTERY_REPLICATED_REPLAY_DEPTH_H
#define CMASTERY_REPLICATED_REPLAY_DEPTH_H
#include "effects.h"
#include "replicated.h"
#include "store.h"
enum {
  RPR_OK,
  RPR_INVALID,
  RPR_FULL,
  RPR_RANGE,
  RPR_STALE,
  RPR_CONFLICT,
  RPR_EMPTY,
  RPR_IO,
  RPR_UNCERTAIN,
  RPR_CORRUPT,
  RPR_STATE,
  RPR_WAIT,
  RPR_UNIMPLEMENTED
};
typedef struct {
  uint64_t timeline;
  uint32_t revision;
  unsigned horizon, head, confirmed;
  RPState initial[2], snapshots[9][2];
  int local[8], remote[8];
  unsigned known[8];
  ArEffects effects;
} RprSession;
/* One replacement notice invalidates speculative presentation for [from,to].
   It does not call an effect sink or cancel an already committed effect. */
typedef struct {
  unsigned from, to;
  uint32_t old_revision, new_revision;
  int32_t old_value, new_value;
} RprNotice;
typedef struct {
  uint64_t timeline;
  unsigned tick;
  int32_t value;
} RprEffect;
int rpr_init(uint64_t, unsigned, const RPState initial[2], RprSession *);
int rpr_advance(RprSession *, int local_move, RprNotice *);
int rpr_correct(RprSession *, uint64_t, unsigned tick, int remote_move,
                RprNotice *);
int rpr_prepare(RprSession *, RprEffect *);
int rpr_feedback(RprSession *, RprEffect, unsigned result);
int rpr_value(const RprSession *, unsigned tick, int32_t *);
int rpr_invariant(const RprSession *);
int rpr_encode(const RprSession *, unsigned char out[512]);
int rpr_decode(uint64_t, const unsigned char in[512], RprSession *);
typedef struct {
  store *pages;
  RprSession session;
} RprDurable;
int rpr_create(store *, uint64_t, unsigned, const RPState initial[2],
               RprDurable *);
int rpr_open(store *, uint64_t, RprDurable *);
int rpr_advance_durable(RprDurable *, int, RprNotice *);
int rpr_correct_durable(RprDurable *, uint64_t, unsigned, int, RprNotice *);
int rpr_prepare_durable(RprDurable *, RprEffect *);
int rpr_feedback_durable(RprDurable *, RprEffect, unsigned);
typedef struct {
  uint64_t timeline;
  unsigned tick;
  int movement;
} RprInput;
int rpr_input_encode(RprInput, unsigned char out[16]);
int rpr_input_decode(const unsigned char in[16], RprInput *);
int rpr_input_frame(RprInput, unsigned char out[18]);
int rpr_input_unframe(const unsigned char *, size_t, unsigned chunk,
                      RprInput *);
#endif
