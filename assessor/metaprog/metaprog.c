#include "metaprog.h"
#include <limits.h>
#ifndef MOD_MUTATION
#define MOD_MUTATION 0
#endif
#define MP_PREFIX mp_u32
#define MP_RING struct mpu32_ring
#define MP_ELEMENT uint32_t
#include "ring_template.inc"
#define MP_PREFIX mp_pair
#define MP_RING struct mp_pair_ring
#define MP_ELEMENT struct mp_pair
#include "ring_template.inc"

int mp_storage_bound(size_t count, size_t width, size_t limit, size_t *out)
{
    if (count == 0 || width == 0 ||
        (MOD_MUTATION != 3 && count > SIZE_MAX / width))
        return 0;
    if (MOD_MUTATION != 4 && count * width > limit)
        return 0;
    *out = count * width;
    return 1;
}

int mp_reduce(const uint32_t *input, size_t count, struct mp_policy policy,
              uint32_t *out)
{
    uint32_t value = 0;
    size_t i;
    if (policy.saturate > 1 || (count && !input))
        return 0;
    for (i = 0; i < count; ++i)
    {
        if (input[i] > policy.limit - value)
        {
            if (!policy.saturate && MOD_MUTATION != 5)
                return 0;
            value = policy.limit;
        }
        else
            value += input[i];
    }
    if (MOD_MUTATION == 6 && count == 0)
        value = policy.limit;
    *out = value;
    return 1;
}

#define MP_POLICY_WRAPPER(name, mode)                             \
    int name(const uint32_t *input, size_t count, uint32_t limit, \
             uint32_t *out)                                       \
    {                                                             \
        const struct mp_policy policy = {limit, mode};            \
        return mp_reduce(input, count, policy, out);              \
    }
MP_POLICY_WRAPPER(mp_reduce_reject, 0)
MP_POLICY_WRAPPER(mp_reduce_saturating, 1)
#undef MP_POLICY_WRAPPER

static uint32_t mp_tick(uint32_t value, unsigned *calls)
{
    ++*calls;
    return value;
}

static uint32_t mp_sum1(uint32_t a)
{
    return a;
}

static uint32_t mp_sum2(uint32_t a, uint32_t b)
{
    return a + b;
}

static uint32_t mp_sum3(uint32_t a, uint32_t b, uint32_t c)
{
    return a + b + c;
}

static uint32_t mp_sum4(uint32_t a, uint32_t b, uint32_t c, uint32_t d)
{
    return a + b + c + d;
}

#define MP_SELECT(_1, _2, _3, _4, NAME, ...) NAME
#define MP_SUM(...)                                            \
    MP_SELECT(__VA_ARGS__, mp_sum4, mp_sum3, mp_sum2, mp_sum1, \
              unused)(__VA_ARGS__)

int mp_variadic(unsigned arity, uint32_t seed, unsigned *evaluations,
                uint32_t *out)
{
    uint32_t value;
    unsigned calls[4] = {0, 0, 0, 0};
    if (arity > 4)
        return 0;
    switch (arity)
    {
        case 0:
            value = MOD_MUTATION == 8 ? seed : 0;
            break;
        case 1:
            value = MP_SUM(mp_tick(seed, &calls[0]));
            break;
        case 2:
            value =
                MP_SUM(mp_tick(seed, &calls[0]), mp_tick(seed + 1, &calls[1]));
            break;
        case 3:
            value =
                MP_SUM(mp_tick(seed, &calls[0]), mp_tick(seed + 1, &calls[1]),
                       mp_tick(seed + 2, &calls[2]));
            break;
        default:
            value = MP_SUM(
                mp_tick(seed, &calls[0]), mp_tick(seed + 1, &calls[1]),
                mp_tick(seed + 2, &calls[2]), mp_tick(seed + 3, &calls[3]));
            break;
    }
    if (MOD_MUTATION == 7 && arity)
        value += mp_tick(seed, &calls[0]);
    *out = value;
    *evaluations = calls[0] + calls[1] + calls[2] + calls[3];
    return 1;
}

static uint32_t mp_mut_receiver(uint32_t *value)
{
    return *value + 100u;
}

static uint32_t mp_const_receiver(const uint32_t *value)
{
    return *value + (MOD_MUTATION == 9 ? 100u : 200u);
}

#define MP_RECEIVER(value)           \
    _Generic((value),                \
        uint32_t *: mp_mut_receiver, \
        const uint32_t *: mp_const_receiver)(value)

static uint32_t *mp_mut_pointer(uint32_t *value, unsigned *calls)
{
    ++*calls;
    return value;
}

static const uint32_t *mp_const_pointer(const uint32_t *value, unsigned *calls)
{
    ++*calls;
    return value;
}

int mp_generic_receivers(uint32_t *mutable_value, const uint32_t *fixed_value,
                         unsigned *evaluations, uint32_t *out)
{
    unsigned calls = 0;
    uint32_t a, b;
    a = MP_RECEIVER(mp_mut_pointer(mutable_value, &calls));
    b = MP_RECEIVER(mp_const_pointer(fixed_value, &calls));
    if (MOD_MUTATION == 10)
        (void)mp_const_pointer(fixed_value, &calls);
    *out = a + b;
    *evaluations = calls;
    return 1;
}

#define MP_FIELD(name, type, width) \
    {#name, offsetof(struct mp_record, name), width},
static const struct mp_field mp_descriptors[] = {MP_RECORD_FIELDS(MP_FIELD)};
#undef MP_FIELD

size_t mp_fields(const struct mp_field **out)
{
    *out = mp_descriptors;
    return MOD_MUTATION == 12
               ? 1
               : sizeof mp_descriptors / sizeof mp_descriptors[0];
}

int mp_record_encode(struct mp_record record, unsigned char *output,
                     size_t capacity)
{
    if (capacity < 6)
        return 0;
    output[0] =
        (unsigned char)(MOD_MUTATION == 11 ? record.code : record.code >> 8);
    output[1] =
        (unsigned char)(MOD_MUTATION == 11 ? record.code >> 8 : record.code);
    output[2] = (unsigned char)(record.value >> 24);
    output[3] = (unsigned char)(record.value >> 16);
    output[4] = (unsigned char)(record.value >> 8);
    output[5] = (unsigned char)record.value;
    return 1;
}

int mp_equal(struct mp_key a, struct mp_key b)
{
    return a.x == b.x && (MOD_MUTATION == 13 || a.y == b.y);
}

static uint64_t mp_hash_byte(uint64_t hash, unsigned char byte)
{
    return (hash ^ byte) * UINT64_C(1099511628211);
}

static uint64_t mp_hash_word(uint64_t hash, uint64_t value)
{
    unsigned i;
    for (i = 0; i < 8; ++i)
    {
        hash = mp_hash_byte(hash, (unsigned char)value);
        value >>= 8;
    }
    return hash;
}

uint64_t mp_hash(struct mp_key value)
{
    uint64_t hash = mp_hash_word(UINT64_C(14695981039346656037), value.x);
    hash = mp_hash_word(hash, value.y);
    if (MOD_MUTATION == 14)
        hash = mp_hash_word(hash, value.cache);
    return hash;
}

int mp_find_counterexample(mp_property_fn property, void *context,
                           unsigned bound, struct mp_pair *out)
{
    unsigned x, y;
    if (!property || bound > 255)
        return -1;
    for (x = 0; x <= bound; ++x)
        for (y = 0; y <= bound; ++y)
        {
            struct mp_pair candidate = {x, y};
            if (!property(candidate, context))
            {
                int changed = 1;
                while (changed)
                {
                    struct mp_pair trial = candidate;
                    changed = 0;
                    if (candidate.x)
                    {
                        --trial.x;
                        if (!property(trial, context))
                        {
                            candidate = trial;
                            changed = 1;
                            continue;
                        }
                    }
                    trial = candidate;
                    if (candidate.y)
                    {
                        --trial.y;
                        if (!property(trial, context))
                        {
                            candidate = trial;
                            changed = 1;
                        }
                    }
                }
                *out = candidate;
                return 1;
            }
        }
    return 0;
}

struct mp_arg mp_arg_number(uint32_t value)
{
    struct mp_arg a = {1, value, NULL};
    return a;
}

struct mp_arg mp_arg_text(const char *value)
{
    struct mp_arg a = {2, 0, value};
    return a;
}

int mp_format(struct mp_sink *sink, struct mp_arg argument)
{
    unsigned char temporary[10];
    size_t n = 0, i;
    const unsigned char *text;
    if (sink->used > sink->capacity || (sink->capacity && !sink->bytes))
        return 0;
    if (argument.kind == 1)
    {
        uint32_t value = argument.number;
        if (MOD_MUTATION != 16 || value != 0)
            do
            {
                temporary[n++] = (unsigned char)('0' + value % 10);
                value /= 10;
            } while (value);
        if (n > sink->capacity - sink->used)
        {
            if (MOD_MUTATION == 15)
                sink->used += n;
            return 0;
        }
        for (i = 0; i < n; ++i)
            sink->bytes[sink->used + i] = temporary[n - 1 - i];
    }
    else if (argument.kind == 2 && argument.text)
    {
        text = (const unsigned char *)argument.text;
        while (text[n])
        {
            if (n == sink->capacity - sink->used)
            {
                if (MOD_MUTATION == 15)
                    ++sink->used;
                return 0;
            }
            ++n;
        }
        for (i = 0; i < n; ++i)
            sink->bytes[sink->used + i] = text[i];
    }
    else
        return 0;
    sink->used += n;
    return 1;
}

int mp_expand(const struct mp_token *pattern, size_t count,
              const struct mp_token *arguments, size_t argument_count,
              unsigned *next_context, struct mp_token *output, size_t capacity,
              size_t *used)
{
    struct mp_token temporary[32];
    size_t i;
    unsigned fresh = *next_context;
    if (count > 32 || count > capacity || (count && !pattern) ||
        (argument_count && !arguments) || !fresh ||
        (MOD_MUTATION != 18 && fresh == UINT_MAX))
        return 0;
    for (i = 0; i < count; ++i)
    {
        temporary[i] = pattern[i];
        if (pattern[i].kind == 1)
        {
            if (pattern[i].symbol >= argument_count)
                return 0;
            temporary[i] = arguments[pattern[i].symbol];
            if (MOD_MUTATION == 17)
                temporary[i].context = fresh;
        }
        else if (pattern[i].kind == 2)
            temporary[i].context = fresh;
        else if (pattern[i].kind != 0)
            return 0;
    }
    for (i = 0; i < count; ++i)
        output[i] = temporary[i];
    *next_context = fresh + 1;
    *used = count;
    return 1;
}

int mp_build(const struct mp_build_node *nodes, size_t count, unsigned *order,
             uint64_t *keys)
{
    unsigned done = 0, valid;
    size_t step, j;
    unsigned temporary_order[8];
    uint64_t temporary_keys[8] = {0};
    if (count > 8 || (count && !nodes))
        return 0;
    valid = (1u << (unsigned)count) - 1u;
    for (j = 0; j < count; ++j)
        if ((nodes[j].dependencies & ~valid) ||
            (nodes[j].length && !nodes[j].source))
            return 0;
    for (step = 0; step < count; ++step)
    {
        size_t chosen = count, k;
        uint64_t key;
        for (j = 0; j < count; ++j)
            if (!(done & (1u << (unsigned)j)) &&
                !(nodes[j].dependencies & ~done))
            {
                chosen = j;
                break;
            }
        if (chosen == count)
            return 0;
        key =
            mp_hash_word(UINT64_C(14695981039346656037), nodes[chosen].length);
        for (k = 0; k < nodes[chosen].length; ++k)
            key = mp_hash_byte(key, nodes[chosen].source[k]);
        key = mp_hash_word(key, nodes[chosen].tool);
        if (MOD_MUTATION != 19)
            key = mp_hash_word(key, nodes[chosen].configuration);
        for (j = 0; j < count; ++j)
            if (nodes[chosen].dependencies & (1u << (unsigned)j))
            {
                key = mp_hash_word(key, j);
                if (MOD_MUTATION != 20)
                    key = mp_hash_word(key, temporary_keys[j]);
            }
        temporary_order[step] = (unsigned)chosen;
        temporary_keys[chosen] = key;
        done |= 1u << (unsigned)chosen;
    }
    for (j = 0; j < count; ++j)
    {
        order[j] = temporary_order[j];
        keys[j] = temporary_keys[j];
    }
    return 1;
}

int mp_emit_table(const uint64_t *keys, size_t count, char *output,
                  size_t capacity, size_t *used)
{
    const char prefix[] =
        "#include <stdint.h>\nconst uint64_t generated_keys[] = {";
    const char digits[] = "0123456789abcdef";
    char temporary[256];
    size_t n = 0, i;
    unsigned j;
    if (count == 0 || count > 8 || !keys)
        return 0;
    for (i = 0; i < sizeof prefix - 1; ++i)
        temporary[n++] = prefix[i];
    for (i = 0; i < count; ++i)
    {
        temporary[n++] = '0';
        temporary[n++] = 'x';
        for (j = 0; j < 16; ++j)
            temporary[n++] = digits[(keys[i] >> (60 - 4 * j)) & 15];
        temporary[n++] = 'U';
        temporary[n++] = 'L';
        temporary[n++] = 'L';
        temporary[n++] = ',';
    }
    temporary[n++] = '}';
    temporary[n++] = ';';
    temporary[n++] = '\n';
    if (n + 1 > capacity)
        return 0;
    for (i = 0; i < n; ++i)
        output[i] = temporary[i];
    output[n] = 0;
    *used = n;
    return 1;
}
