#ifndef CMASTERY_REPLICATED_H
#define CMASTERY_REPLICATED_H
#include <stddef.h>
#include <stdint.h>
#include "dod.h"
enum rp_status { RP_OK, RP_INVALID, RP_FULL, RP_RANGE, RP_CONFLICT, RP_GAP, RP_FAILED };
struct rp_event { unsigned position, key; int value; };
struct rp_log { struct d_hamt arena; struct rp_event events[16]; unsigned roots[17], count; };
void rp_log_init(struct rp_log *log);
int rp_log_append(struct rp_log *log, unsigned key, int value);
int rp_log_get(const struct rp_log *log, unsigned position, unsigned key, int *value);
struct rp_projection { struct rp_event applied[16]; unsigned count; int64_t total; };
int rp_project(struct rp_projection *projection, struct rp_event event);
int rp_lamport(uint64_t local, uint64_t remote, uint64_t *result);
struct rp_vector { uint64_t v[3]; };
int rp_vector_receive(struct rp_vector local, struct rp_vector remote, unsigned replica, struct rp_vector *result);
int rp_vector_order(struct rp_vector a, struct rp_vector b);
struct rp_hlc { uint64_t physical; uint32_t logical; };
int rp_hlc(struct rp_hlc local, const struct rp_hlc *remote, uint64_t wall, struct rp_hlc *result);
struct rp_pn { uint32_t positive[3], negative[3]; };
int rp_pn_add(struct rp_pn *state, unsigned replica, int negative, uint32_t amount);
void rp_pn_merge(struct rp_pn *state, const struct rp_pn *other);
int64_t rp_pn_value(const struct rp_pn *state);
struct rp_register { uint64_t time; unsigned replica; int value; };
int rp_register_merge(struct rp_register *state, struct rp_register other);
struct rp_set { uint16_t adds[4], removed[4]; unsigned next[2]; };
int rp_set_add(struct rp_set *state, unsigned replica, unsigned value, struct rp_set *delta);
int rp_set_remove(struct rp_set *state, unsigned value, struct rp_set *delta);
void rp_set_merge(struct rp_set *state, const struct rp_set *other);
int rp_set_contains(const struct rp_set *state, unsigned value);
struct rp_node { unsigned id, parent; unsigned char value, removed; };
struct rp_sequence { struct rp_node nodes[16]; unsigned count; };
int rp_sequence_insert(struct rp_sequence *sequence, unsigned id, unsigned parent, unsigned char value);
int rp_sequence_remove(struct rp_sequence *sequence, unsigned id);
int rp_sequence_render(const struct rp_sequence *sequence, unsigned char *out, size_t capacity, size_t *length);
int rp_ot_insert(unsigned position, unsigned priority, unsigned concurrent_position, unsigned concurrent_priority, unsigned *result);
int rp_sha256(const unsigned char *bytes, size_t length, unsigned char out[32]);
int rp_merkle(const uint32_t *leaves, size_t count, unsigned char out[32]);
int rp_merkle_diff(const uint32_t *left, const uint32_t *right, size_t count, unsigned *mask);
struct rp_field { unsigned tag; uint32_t value; };
struct rp_message { struct rp_field fields[8]; unsigned count; };
uint32_t rp_zigzag(int32_t value);
int32_t rp_unzigzag(uint32_t value);
int rp_decode(const unsigned char *bytes, size_t length, struct rp_message *message);
int rp_encode(const struct rp_message *message, unsigned char *bytes, size_t capacity, size_t *length);
struct rp_state { unsigned tick, position, score; uint32_t random; };
struct rp_effect { unsigned tick, score; };
int rp_step(struct rp_state state, int movement, struct rp_state *result, struct rp_effect *effect);
int rp_replay(struct rp_state initial, const int *movements, size_t count, struct rp_state *result, struct rp_effect *effects);
int rp_checkpoint(struct d_hamt *arena, unsigned root, struct rp_state state, unsigned fail_ordinal, unsigned *new_root);
#endif
