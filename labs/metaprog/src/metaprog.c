#include "metaprog.h"
/* Intentionally incomplete; implement the canonical exercise contracts. */
int mp_u32_push(MPU32Ring *ring, uint32_t value) { (void)ring; (void)value; return 0; }
int mp_u32_pop(MPU32Ring *ring, uint32_t *out) { (void)ring; (void)out; return 0; }
int mp_pair_push(MPPairRing *ring, MPPair value) { (void)ring; (void)value; return 0; }
int mp_pair_pop(MPPairRing *ring, MPPair *out) { (void)ring; (void)out; return 0; }
int mp_storage_bound(size_t count, size_t width, size_t limit, size_t *out) { (void)count; (void)width; (void)limit; (void)out; return 0; }
int mp_reduce(const uint32_t *input, size_t count, MPPolicy policy, uint32_t *out) { (void)input; (void)count; (void)policy; (void)out; return 0; }
int mp_reduce_reject(const uint32_t *input, size_t count, uint32_t limit, uint32_t *out) { (void)input; (void)count; (void)limit; (void)out; return 0; }
int mp_reduce_saturating(const uint32_t *input, size_t count, uint32_t limit, uint32_t *out) { (void)input; (void)count; (void)limit; (void)out; return 0; }
int mp_variadic(unsigned arity, uint32_t seed, unsigned *evaluations, uint32_t *out) { (void)arity; (void)seed; (void)evaluations; (void)out; return 0; }
int mp_generic_receivers(uint32_t *mutable_value, const uint32_t *fixed_value, unsigned *evaluations, uint32_t *out) { (void)mutable_value; (void)fixed_value; (void)evaluations; (void)out; return 0; }
size_t mp_fields(const MPField **out) { (void)out; return 0; }
int mp_record_encode(MPRecord record, unsigned char *output, size_t capacity) { (void)record; (void)output; (void)capacity; return 0; }
int mp_equal(MPKey a, MPKey b) { (void)a; (void)b; return 0; }
uint64_t mp_hash(MPKey value) { (void)value; return 0; }
int mp_find_counterexample(MPProperty property, void *context, unsigned bound, MPPair *out) { (void)property; (void)context; (void)bound; (void)out; return 0; }
MPArg mp_arg_number(uint32_t value) { (void)value; return (MPArg){0,0,NULL}; }
MPArg mp_arg_text(const char *value) { (void)value; return (MPArg){0,0,NULL}; }
int mp_format(MPSink *sink, MPArg argument) { (void)sink; (void)argument; return 0; }
int mp_expand(const MPToken *pattern, size_t count, const MPToken *arguments,
              size_t argument_count, unsigned *next_context, MPToken *output,
              size_t capacity, size_t *used) { (void)pattern; (void)count; (void)arguments; (void)argument_count; (void)next_context; (void)output; (void)capacity; (void)used; return 0; }
int mp_build(const MPBuildNode *nodes, size_t count, unsigned *order, uint64_t *keys) { (void)nodes; (void)count; (void)order; (void)keys; return 0; }
int mp_emit_table(const uint64_t *keys, size_t count, char *output, size_t capacity, size_t *used) { (void)keys; (void)count; (void)output; (void)capacity; (void)used; return 0; }
