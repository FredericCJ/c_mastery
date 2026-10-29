#ifndef CMASTERY_METAPROG_H
#define CMASTERY_METAPROG_H
#include <stddef.h>
#include <stdint.h>
struct mp_pair { uint32_t x, y; };
struct mpu32_ring { uint32_t data[4]; size_t head, count; };
struct mp_pair_ring { struct mp_pair data[4]; size_t head, count; };
int mp_u32_push(struct mpu32_ring *ring, uint32_t value);
int mp_u32_pop(struct mpu32_ring *ring, uint32_t *out);
int mp_pair_push(struct mp_pair_ring *ring, struct mp_pair value);
int mp_pair_pop(struct mp_pair_ring *ring, struct mp_pair *out);
int mp_storage_bound(size_t count, size_t width, size_t limit, size_t *out);
struct mp_policy { uint32_t limit; unsigned saturate; };
int mp_reduce(const uint32_t *input, size_t count, struct mp_policy policy, uint32_t *out);
int mp_reduce_reject(const uint32_t *input, size_t count, uint32_t limit, uint32_t *out);
int mp_reduce_saturating(const uint32_t *input, size_t count, uint32_t limit, uint32_t *out);
int mp_variadic(unsigned arity, uint32_t seed, unsigned *evaluations, uint32_t *out);
int mp_generic_receivers(uint32_t *mutable_value, const uint32_t *fixed_value, unsigned *evaluations, uint32_t *out);
#define MP_RECORD_FIELDS(X) X(code,uint16_t,2) X(value,uint32_t,4)
#define MP_DECLARE_FIELD(name,type,width) type name;
struct mp_record { MP_RECORD_FIELDS(MP_DECLARE_FIELD) };
#undef MP_DECLARE_FIELD
struct mp_field { const char *name; size_t offset, width; };
size_t mp_fields(const struct mp_field **out);
int mp_record_encode(struct mp_record record, unsigned char *output, size_t capacity);
struct mp_key { uint32_t x, y, cache; };
int mp_equal(struct mp_key a, struct mp_key b);
uint64_t mp_hash(struct mp_key value);
typedef int (*mp_property_fn)(struct mp_pair value, void *context);
int mp_find_counterexample(mp_property_fn property, void *context, unsigned bound, struct mp_pair *out);
struct mp_sink { unsigned char *bytes; size_t capacity, used; };
struct mp_arg { unsigned kind; uint32_t number; const char *text; };
struct mp_arg mp_arg_number(uint32_t value);
struct mp_arg mp_arg_text(const char *value);
#define MP_ARG(value) _Generic((value), uint32_t: mp_arg_number, char *: mp_arg_text, const char *: mp_arg_text)(value)
int mp_format(struct mp_sink *sink, struct mp_arg argument);
struct mp_token { unsigned kind, symbol, context; };
int mp_expand(const struct mp_token *pattern, size_t count, const struct mp_token *arguments,
              size_t argument_count, unsigned *next_context, struct mp_token *output,
              size_t capacity, size_t *used);
struct mp_build_node { unsigned dependencies; const unsigned char *source; size_t length; uint64_t tool, configuration; };
int mp_build(const struct mp_build_node *nodes, size_t count, unsigned *order, uint64_t *keys);
int mp_emit_table(const uint64_t *keys, size_t count, char *output, size_t capacity, size_t *used);
#endif
