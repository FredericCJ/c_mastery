#ifndef CMASTERY_REPLICATED_REPLAY_DEPTH_H
#define CMASTERY_REPLICATED_REPLAY_DEPTH_H
#include "effects.h"
#include "replicated.h"
#include "store.h"

enum
{
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

struct rpr_session
{
    uint64_t timeline;
    uint32_t revision;
    unsigned horizon, head, confirmed;
    struct rp_state initial[2], snapshots[9][2];
    int local[8], remote[8];
    unsigned known[8];
    struct ar_effects effects;
};

/* One replacement notice invalidates speculative presentation for [from,to].
   It does not call an effect sink or cancel an already committed effect. */
struct rpr_notice
{
    unsigned from, to;
    uint32_t old_revision, new_revision;
    int32_t old_value, new_value;
};

struct rpr_effect
{
    uint64_t timeline;
    unsigned tick;
    int32_t value;
};

int rpr_init(uint64_t, unsigned, const struct rp_state initial[2],
             struct rpr_session *);
int rpr_advance(struct rpr_session *, int local_move, struct rpr_notice *);
int rpr_correct(struct rpr_session *, uint64_t, unsigned tick, int remote_move,
                struct rpr_notice *);
int rpr_prepare(struct rpr_session *, struct rpr_effect *);
int rpr_feedback(struct rpr_session *, struct rpr_effect, unsigned result);
int rpr_value(const struct rpr_session *, unsigned tick, int32_t *);
int rpr_invariant(const struct rpr_session *);
int rpr_encode(const struct rpr_session *, unsigned char out[512]);
int rpr_decode(uint64_t, const unsigned char in[512], struct rpr_session *);

struct rpr_durable
{
    store *pages;
    struct rpr_session session;
};

int rpr_create(store *, uint64_t, unsigned, const struct rp_state initial[2],
               struct rpr_durable *);
int rpr_open(store *, uint64_t, struct rpr_durable *);
int rpr_advance_durable(struct rpr_durable *, int, struct rpr_notice *);
int rpr_correct_durable(struct rpr_durable *, uint64_t, unsigned, int,
                        struct rpr_notice *);
int rpr_prepare_durable(struct rpr_durable *, struct rpr_effect *);
int rpr_feedback_durable(struct rpr_durable *, struct rpr_effect, unsigned);

struct rpr_input
{
    uint64_t timeline;
    unsigned tick;
    int movement;
};

int rpr_input_encode(struct rpr_input, unsigned char out[16]);
int rpr_input_decode(const unsigned char in[16], struct rpr_input *);
int rpr_input_frame(struct rpr_input, unsigned char out[18]);
int rpr_input_unframe(const unsigned char *, size_t, unsigned chunk,
                      struct rpr_input *);
#endif
