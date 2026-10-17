#ifndef CMASTERY_REPLICATED_H
#define CMASTERY_REPLICATED_H
#include <stddef.h>
#include <stdint.h>
#include "dod.h"
enum RPStatus { RP_OK, RP_INVALID, RP_FULL, RP_RANGE, RP_CONFLICT, RP_GAP, RP_FAILED };
typedef struct { unsigned position, key; int value; } RPEvent;
typedef struct { DHamt arena; RPEvent events[16]; unsigned roots[17], count; } RPLog;
void rp_log_init(RPLog *log);
int rp_log_append(RPLog *log, unsigned key, int value);
int rp_log_get(const RPLog *log, unsigned position, unsigned key, int *value);
typedef struct { RPEvent applied[16]; unsigned count; int64_t total; } RPProjection;
int rp_project(RPProjection *projection, RPEvent event);
int rp_lamport(uint64_t local, uint64_t remote, uint64_t *result);
typedef struct { uint64_t v[3]; } RPVector;
int rp_vector_receive(RPVector local, RPVector remote, unsigned replica, RPVector *result);
int rp_vector_order(RPVector a, RPVector b);
typedef struct { uint64_t physical; uint32_t logical; } RPHLC;
int rp_hlc(RPHLC local, const RPHLC *remote, uint64_t wall, RPHLC *result);
typedef struct { uint32_t positive[3], negative[3]; } RPPN;
int rp_pn_add(RPPN *state, unsigned replica, int negative, uint32_t amount);
void rp_pn_merge(RPPN *state, const RPPN *other);
int64_t rp_pn_value(const RPPN *state);
typedef struct { uint64_t time; unsigned replica; int value; } RPRegister;
int rp_register_merge(RPRegister *state, RPRegister other);
typedef struct { uint16_t adds[4], removed[4]; unsigned next[2]; } RPSet;
int rp_set_add(RPSet *state, unsigned replica, unsigned value, RPSet *delta);
int rp_set_remove(RPSet *state, unsigned value, RPSet *delta);
void rp_set_merge(RPSet *state, const RPSet *other);
int rp_set_contains(const RPSet *state, unsigned value);
typedef struct { unsigned id, parent; unsigned char value, removed; } RPNode;
typedef struct { RPNode nodes[16]; unsigned count; } RPSequence;
int rp_sequence_insert(RPSequence *sequence, unsigned id, unsigned parent, unsigned char value);
int rp_sequence_remove(RPSequence *sequence, unsigned id);
int rp_sequence_render(const RPSequence *sequence, unsigned char *out, size_t capacity, size_t *length);
int rp_ot_insert(unsigned position, unsigned priority, unsigned concurrent_position, unsigned concurrent_priority, unsigned *result);
int rp_sha256(const unsigned char *bytes, size_t length, unsigned char out[32]);
int rp_merkle(const uint32_t *leaves, size_t count, unsigned char out[32]);
int rp_merkle_diff(const uint32_t *left, const uint32_t *right, size_t count, unsigned *mask);
typedef struct { unsigned tag; uint32_t value; } RPField;
typedef struct { RPField fields[8]; unsigned count; } RPMessage;
uint32_t rp_zigzag(int32_t value);
int32_t rp_unzigzag(uint32_t value);
int rp_decode(const unsigned char *bytes, size_t length, RPMessage *message);
int rp_encode(const RPMessage *message, unsigned char *bytes, size_t capacity, size_t *length);
typedef struct { unsigned tick, position, score; uint32_t random; } RPState;
typedef struct { unsigned tick, score; } RPEffect;
int rp_step(RPState state, int movement, RPState *result, RPEffect *effect);
int rp_replay(RPState initial, const int *movements, size_t count, RPState *result, RPEffect *effects);
int rp_checkpoint(DHamt *arena, unsigned root, RPState state, unsigned fail_ordinal, unsigned *new_root);
#endif
