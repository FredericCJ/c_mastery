#include "replicated.h"
/* Unsolved scaffold: every admitted operation still needs implementation. */
void rp_log_init(RPLog *log) { *log = (RPLog){0}; }
int rp_log_append(RPLog *log, unsigned key, int value) { (void)log; (void)key; (void)value; return RP_FAILED; }
int rp_log_get(const RPLog *log, unsigned position, unsigned key, int *value) { (void)log; (void)position; (void)key; (void)value; return RP_FAILED; }
int rp_project(RPProjection *projection, RPEvent event) { (void)projection; (void)event; return RP_FAILED; }
int rp_lamport(uint64_t local, uint64_t remote, uint64_t *result) { (void)local; (void)remote; (void)result; return RP_FAILED; }
int rp_vector_receive(RPVector local, RPVector remote, unsigned replica, RPVector *result) { (void)local; (void)remote; (void)replica; (void)result; return RP_FAILED; }
int rp_vector_order(RPVector a, RPVector b) { (void)a; (void)b; return RP_FAILED; }
int rp_hlc(RPHLC local, const RPHLC *remote, uint64_t wall, RPHLC *result) { (void)local; (void)remote; (void)wall; (void)result; return RP_FAILED; }
int rp_pn_add(RPPN *state, unsigned replica, int negative, uint32_t amount) { (void)state; (void)replica; (void)negative; (void)amount; return RP_FAILED; }
void rp_pn_merge(RPPN *state, const RPPN *other) { (void)state; (void)other;  }
int64_t rp_pn_value(const RPPN *state) { (void)state; return 0; }
int rp_register_merge(RPRegister *state, RPRegister other) { (void)state; (void)other; return RP_FAILED; }
int rp_set_add(RPSet *state, unsigned replica, unsigned value, RPSet *delta) { (void)state; (void)replica; (void)value; (void)delta; return RP_FAILED; }
int rp_set_remove(RPSet *state, unsigned value, RPSet *delta) { (void)state; (void)value; (void)delta; return RP_FAILED; }
void rp_set_merge(RPSet *state, const RPSet *other) { (void)state; (void)other;  }
int rp_set_contains(const RPSet *state, unsigned value) { (void)state; (void)value; return RP_FAILED; }
int rp_sequence_insert(RPSequence *sequence, unsigned id, unsigned parent, unsigned char value) { (void)sequence; (void)id; (void)parent; (void)value; return RP_FAILED; }
int rp_sequence_remove(RPSequence *sequence, unsigned id) { (void)sequence; (void)id; return RP_FAILED; }
int rp_sequence_render(const RPSequence *sequence, unsigned char *out, size_t capacity, size_t *length) { (void)sequence; (void)out; (void)capacity; (void)length; return RP_FAILED; }
int rp_ot_insert(unsigned position, unsigned priority, unsigned concurrent_position, unsigned concurrent_priority, unsigned *result) { (void)position; (void)priority; (void)concurrent_position; (void)concurrent_priority; (void)result; return RP_FAILED; }
int rp_sha256(const unsigned char *bytes, size_t length, unsigned char out[32]) { (void)bytes; (void)length; (void)out; return RP_FAILED; }
int rp_merkle(const uint32_t *leaves, size_t count, unsigned char out[32]) { (void)leaves; (void)count; (void)out; return RP_FAILED; }
int rp_merkle_diff(const uint32_t *left, const uint32_t *right, size_t count, unsigned *mask) { (void)left; (void)right; (void)count; (void)mask; return RP_FAILED; }
uint32_t rp_zigzag(int32_t value) { (void)value; return 0; }
int32_t rp_unzigzag(uint32_t value) { (void)value; return 0; }
int rp_decode(const unsigned char *bytes, size_t length, RPMessage *message) { (void)bytes; (void)length; (void)message; return RP_FAILED; }
int rp_encode(const RPMessage *message, unsigned char *bytes, size_t capacity, size_t *length) { (void)message; (void)bytes; (void)capacity; (void)length; return RP_FAILED; }
int rp_step(RPState state, int movement, RPState *result, RPEffect *effect) { (void)state; (void)movement; (void)result; (void)effect; return RP_FAILED; }
int rp_replay(RPState initial, const int *movements, size_t count, RPState *result, RPEffect *effects) { (void)initial; (void)movements; (void)count; (void)result; (void)effects; return RP_FAILED; }
int rp_checkpoint(DHamt *arena, unsigned root, RPState state, unsigned fail_ordinal, unsigned *new_root) { (void)arena; (void)root; (void)state; (void)fail_ordinal; (void)new_root; return RP_FAILED; }
