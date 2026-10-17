#ifndef CMASTERY_METAPROG_H
#define CMASTERY_METAPROG_H
#include <stddef.h>
#include <stdint.h>
typedef struct { uint32_t x, y; } MPPair;
typedef struct { uint32_t data[4]; size_t head, count; } MPU32Ring;
typedef struct { MPPair data[4]; size_t head, count; } MPPairRing;
int mp_u32_push(MPU32Ring *ring, uint32_t value);
int mp_u32_pop(MPU32Ring *ring, uint32_t *out);
int mp_pair_push(MPPairRing *ring, MPPair value);
int mp_pair_pop(MPPairRing *ring, MPPair *out);
int mp_storage_bound(size_t count, size_t width, size_t limit, size_t *out);
typedef struct { uint32_t limit; unsigned saturate; } MPPolicy;
int mp_reduce(const uint32_t *input, size_t count, MPPolicy policy, uint32_t *out);
int mp_reduce_reject(const uint32_t *input, size_t count, uint32_t limit, uint32_t *out);
int mp_reduce_saturating(const uint32_t *input, size_t count, uint32_t limit, uint32_t *out);
int mp_variadic(unsigned arity, uint32_t seed, unsigned *evaluations, uint32_t *out);
int mp_generic_receivers(uint32_t *mutable_value, const uint32_t *fixed_value, unsigned *evaluations, uint32_t *out);
#define MP_RECORD_FIELDS(X) X(code,uint16_t,2) X(value,uint32_t,4)
#define MP_DECLARE_FIELD(name,type,width) type name;
typedef struct { MP_RECORD_FIELDS(MP_DECLARE_FIELD) } MPRecord;
#undef MP_DECLARE_FIELD
typedef struct { const char *name; size_t offset, width; } MPField;
size_t mp_fields(const MPField **out);
int mp_record_encode(MPRecord record, unsigned char *output, size_t capacity);
typedef struct { uint32_t x, y, cache; } MPKey;
int mp_equal(MPKey a, MPKey b);
uint64_t mp_hash(MPKey value);
typedef int (*MPProperty)(MPPair value, void *context);
int mp_find_counterexample(MPProperty property, void *context, unsigned bound, MPPair *out);
typedef struct { unsigned char *bytes; size_t capacity, used; } MPSink;
typedef struct { unsigned kind; uint32_t number; const char *text; } MPArg;
MPArg mp_arg_number(uint32_t value);
MPArg mp_arg_text(const char *value);
#define MP_ARG(value) _Generic((value), uint32_t: mp_arg_number, char *: mp_arg_text, const char *: mp_arg_text)(value)
int mp_format(MPSink *sink, MPArg argument);
typedef struct { unsigned kind, symbol, context; } MPToken;
int mp_expand(const MPToken *pattern, size_t count, const MPToken *arguments,
              size_t argument_count, unsigned *next_context, MPToken *output,
              size_t capacity, size_t *used);
typedef struct { unsigned dependencies; const unsigned char *source; size_t length; uint64_t tool, configuration; } MPBuildNode;
int mp_build(const MPBuildNode *nodes, size_t count, unsigned *order, uint64_t *keys);
int mp_emit_table(const uint64_t *keys, size_t count, char *output, size_t capacity, size_t *used);
#endif
