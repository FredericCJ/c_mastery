#include "metaprog.h"

/* Intentionally incomplete; implement the canonical exercise contracts. */
int mp_u32_push(struct mpu32_ring *ring, uint32_t value)
{
    (void)ring;
    (void)value;
    return 0;
}

int mp_u32_pop(struct mpu32_ring *ring, uint32_t *out)
{
    (void)ring;
    (void)out;
    return 0;
}

int mp_pair_push(struct mp_pair_ring *ring, struct mp_pair value)
{
    (void)ring;
    (void)value;
    return 0;
}

int mp_pair_pop(struct mp_pair_ring *ring, struct mp_pair *out)
{
    (void)ring;
    (void)out;
    return 0;
}

int mp_storage_bound(size_t count, size_t width, size_t limit, size_t *out)
{
    (void)count;
    (void)width;
    (void)limit;
    (void)out;
    return 0;
}

int mp_reduce(const uint32_t *input, size_t count, struct mp_policy policy,
              uint32_t *out)
{
    (void)input;
    (void)count;
    (void)policy;
    (void)out;
    return 0;
}

int mp_reduce_reject(const uint32_t *input, size_t count, uint32_t limit,
                     uint32_t *out)
{
    (void)input;
    (void)count;
    (void)limit;
    (void)out;
    return 0;
}

int mp_reduce_saturating(const uint32_t *input, size_t count, uint32_t limit,
                         uint32_t *out)
{
    (void)input;
    (void)count;
    (void)limit;
    (void)out;
    return 0;
}

int mp_variadic(unsigned arity, uint32_t seed, unsigned *evaluations,
                uint32_t *out)
{
    (void)arity;
    (void)seed;
    (void)evaluations;
    (void)out;
    return 0;
}

int mp_generic_receivers(uint32_t *mutable_value, const uint32_t *fixed_value,
                         unsigned *evaluations, uint32_t *out)
{
    (void)mutable_value;
    (void)fixed_value;
    (void)evaluations;
    (void)out;
    return 0;
}

size_t mp_fields(const struct mp_field **out)
{
    (void)out;
    return 0;
}

int mp_record_encode(struct mp_record record, unsigned char *output,
                     size_t capacity)
{
    (void)record;
    (void)output;
    (void)capacity;
    return 0;
}

int mp_equal(struct mp_key a, struct mp_key b)
{
    (void)a;
    (void)b;
    return 0;
}

uint64_t mp_hash(struct mp_key value)
{
    (void)value;
    return 0;
}

int mp_find_counterexample(mp_property_fn property, void *context,
                           unsigned bound, struct mp_pair *out)
{
    (void)property;
    (void)context;
    (void)bound;
    (void)out;
    return 0;
}

struct mp_arg mp_arg_number(uint32_t value)
{
    (void)value;
    return (struct mp_arg){0, 0, NULL};
}

struct mp_arg mp_arg_text(const char *value)
{
    (void)value;
    return (struct mp_arg){0, 0, NULL};
}

int mp_format(struct mp_sink *sink, struct mp_arg argument)
{
    (void)sink;
    (void)argument;
    return 0;
}

int mp_expand(const struct mp_token *pattern, size_t count,
              const struct mp_token *arguments, size_t argument_count,
              unsigned *next_context, struct mp_token *output, size_t capacity,
              size_t *used)
{
    (void)pattern;
    (void)count;
    (void)arguments;
    (void)argument_count;
    (void)next_context;
    (void)output;
    (void)capacity;
    (void)used;
    return 0;
}

int mp_build(const struct mp_build_node *nodes, size_t count, unsigned *order,
             uint64_t *keys)
{
    (void)nodes;
    (void)count;
    (void)order;
    (void)keys;
    return 0;
}

int mp_emit_table(const uint64_t *keys, size_t count, char *output,
                  size_t capacity, size_t *used)
{
    (void)keys;
    (void)count;
    (void)output;
    (void)capacity;
    (void)used;
    return 0;
}
