#include "replicated.h"
#include <limits.h>
#include <string.h>
#ifndef MOD_MUTATION
#define MOD_MUTATION 0
#endif
_Static_assert(CHAR_BIT == 8 && INT_MAX == 2147483647,
               "Laboratory requires 8-bit bytes and 32-bit int");

void rp_log_init(struct rp_log *log)
{
    memset(log, 0, sizeof *log);
    d_hamt_init(&log->arena);
}

int rp_log_append(struct rp_log *log, unsigned key, int value)
{
    unsigned root;
    int status;
    if (key > 31)
        return RP_INVALID;
    if (log->count == 16)
        return RP_FULL;
    status =
        d_hamt_update(&log->arena, log->roots[log->count], key, value, &root);
    if (status != D_OK)
    {
        if (MOD_MUTATION == 1)
            ++log->count;
        return RP_FAILED;
    }
    log->events[log->count] = (struct rp_event){log->count + 1, key, value};
    log->roots[++log->count] = root;
    return RP_OK;
}

int rp_log_get(const struct rp_log *log, unsigned position, unsigned key,
               int *value)
{
    if (position > log->count || key > 31)
        return RP_INVALID;
    if (MOD_MUTATION == 2)
        position = log->count;
    return d_hamt_get(&log->arena, log->roots[position], key, value) == D_OK
               ? RP_OK
               : RP_GAP;
}

int rp_project(struct rp_projection *projection, struct rp_event event)
{
    if (event.position == 0 || event.position > 16 || event.key > 31)
        return RP_INVALID;
    if (event.position <= projection->count)
    {
        struct rp_event prior = projection->applied[event.position - 1];
        if (prior.key != event.key || prior.value != event.value)
            return RP_CONFLICT;
        if (MOD_MUTATION == 3)
            projection->total += event.value;
        return RP_OK;
    }
    if (event.position != projection->count + 1 && MOD_MUTATION != 4)
        return RP_GAP;
    projection->total +=
        event
            .value; /* <=16 int values: representable in int64_t on this target. */
    projection->applied[projection->count++] = event;
    return RP_OK;
}

int rp_lamport(uint64_t local, uint64_t remote, uint64_t *result)
{
    uint64_t maximum = local > remote ? local : remote;
    if (MOD_MUTATION == 5)
        maximum = local;
    if (maximum == UINT64_MAX)
        return RP_RANGE;
    *result = maximum + 1;
    return RP_OK;
}

int rp_vector_receive(struct rp_vector local, struct rp_vector remote,
                      unsigned replica, struct rp_vector *result)
{
    unsigned i;
    if (replica >= 3)
        return RP_INVALID;
    for (i = 0; i < 3; ++i)
        if (remote.v[i] > local.v[i])
            local.v[i] = remote.v[i];
    if (local.v[replica] == UINT64_MAX)
        return RP_RANGE;
    ++local.v[replica];
    *result = local;
    return RP_OK;
}

int rp_vector_order(struct rp_vector a, struct rp_vector b)
{
    unsigned i;
    int less = 0, greater = 0;
    for (i = 0; i < 3; ++i)
    {
        less |= a.v[i] < b.v[i];
        greater |= a.v[i] > b.v[i];
    }
    return less && greater ? 2 : less ? -1 : greater ? 1 : 0;
}

int rp_hlc(struct rp_hlc local, const struct rp_hlc *remote, uint64_t wall,
           struct rp_hlc *result)
{
    struct rp_hlc next = {wall, 0};
    uint32_t counter = 0;
    int increment = 0;
    if (local.physical > next.physical)
        next.physical = local.physical;
    if (remote && remote->physical > next.physical)
        next.physical = remote->physical;
    if (next.physical == local.physical)
    {
        counter = local.logical;
        increment = 1;
    }
    if (remote && next.physical == remote->physical)
    {
        if (!increment || remote->logical > counter)
            counter = remote->logical;
        increment = 1;
    }
    if (MOD_MUTATION == 6 && remote && remote->physical == local.physical)
        counter = local.logical;
    if (increment)
    {
        if (counter == UINT32_MAX)
            return RP_RANGE;
        next.logical = counter + 1;
    }
    *result = next;
    return RP_OK;
}

int rp_pn_add(struct rp_pn *state, unsigned replica, int negative,
              uint32_t amount)
{
    uint32_t *component;
    if (replica >= 3 || (negative != 0 && negative != 1))
        return RP_INVALID;
    component =
        negative ? &state->negative[replica] : &state->positive[replica];
    if (amount > UINT32_MAX - *component)
        return RP_RANGE;
    *component += amount;
    return RP_OK;
}

void rp_pn_merge(struct rp_pn *state, const struct rp_pn *other)
{
    unsigned i;
    for (i = 0; i < 3; ++i)
    {
        if (MOD_MUTATION == 7)
        {
            state->positive[i] += other->positive[i];
            state->negative[i] += other->negative[i];
        }
        else
        {
            if (other->positive[i] > state->positive[i])
                state->positive[i] = other->positive[i];
            if (other->negative[i] > state->negative[i])
                state->negative[i] = other->negative[i];
        }
    }
}

int64_t rp_pn_value(const struct rp_pn *state)
{
    unsigned i;
    int64_t value = 0;
    for (i = 0; i < 3; ++i)
        value += (int64_t)state->positive[i] - state->negative[i];
    return value;
}

int rp_register_merge(struct rp_register *state, struct rp_register other)
{
    if (state->replica >= 3 || other.replica >= 3)
        return RP_INVALID;
    if (state->time == other.time && state->replica == other.replica &&
        state->value != other.value)
        return RP_CONFLICT;
    if (other.time > state->time ||
        (MOD_MUTATION != 8 && other.time == state->time &&
         other.replica > state->replica))
        *state = other;
    return RP_OK;
}

int rp_set_add(struct rp_set *state, unsigned replica, unsigned value,
               struct rp_set *delta)
{
    struct rp_set change = {{0}, {0}, {0}};
    unsigned bit;
    if (replica >= 2 || value >= 4)
        return RP_INVALID;
    if (state->next[replica] >= 8)
        return RP_FULL;
    bit = replica * 8 + state->next[replica];
    change.adds[value] = (uint16_t)(1u << bit);
    change.next[replica] = state->next[replica] + 1;
    rp_set_merge(state, &change);
    *delta = change;
    return RP_OK;
}

int rp_set_remove(struct rp_set *state, unsigned value, struct rp_set *delta)
{
    struct rp_set change = {{0}, {0}, {0}};
    if (value >= 4)
        return RP_INVALID;
    change.removed[value] = MOD_MUTATION == 9 ? UINT16_MAX : state->adds[value];
    rp_set_merge(state, &change);
    *delta = change;
    return RP_OK;
}

void rp_set_merge(struct rp_set *state, const struct rp_set *other)
{
    unsigned i;
    for (i = 0; i < 4; ++i)
    {
        state->adds[i] |= other->adds[i];
        if (MOD_MUTATION != 10)
            state->removed[i] |= other->removed[i];
    }
    for (i = 0; i < 2; ++i)
        if (other->next[i] > state->next[i])
            state->next[i] = other->next[i];
}

int rp_set_contains(const struct rp_set *state, unsigned value)
{
    return value < 4 &&
           (state->adds[value] & (uint16_t)~state->removed[value]) != 0;
}

static int node_index(const struct rp_sequence *sequence, unsigned id)
{
    unsigned i;
    for (i = 0; i < sequence->count; ++i)
        if (sequence->nodes[i].id == id)
            return (int)i;
    return -1;
}

int rp_sequence_insert(struct rp_sequence *sequence, unsigned id,
                       unsigned parent, unsigned char value)
{
    int existing;
    if (id == 0 || parent >= id)
        return RP_INVALID;
    existing = node_index(sequence, id);
    if (existing >= 0)
        return sequence->nodes[existing].parent == parent &&
                       sequence->nodes[existing].value == value
                   ? RP_OK
                   : RP_CONFLICT;
    if (parent && node_index(sequence, parent) < 0)
        return RP_GAP;
    if (sequence->count == 16)
        return RP_FULL;
    sequence->nodes[sequence->count++] = (struct rp_node){id, parent, value, 0};
    return RP_OK;
}

int rp_sequence_remove(struct rp_sequence *sequence, unsigned id)
{
    int index = node_index(sequence, id);
    if (index < 0)
        return RP_GAP;
    sequence->nodes[index].removed = 1;
    return RP_OK;
}

static void render_children(const struct rp_sequence *sequence, unsigned parent,
                            unsigned char out[16], size_t *length)
{
    unsigned used = 0, k, i;
    for (k = 0; k < sequence->count; ++k)
    {
        int best = -1;
        for (i = 0; i < sequence->count; ++i)
            if (!(used & (1u << i)) && sequence->nodes[i].parent == parent)
            {
                if (best < 0 ||
                    (MOD_MUTATION != 11 &&
                     sequence->nodes[i].id > sequence->nodes[best].id))
                    best = (int)i;
            }
        if (best < 0)
            return;
        used |= 1u << (unsigned)best;
        if (!sequence->nodes[best].removed)
            out[(*length)++] = sequence->nodes[best].value;
        if (MOD_MUTATION != 12 || !sequence->nodes[best].removed)
            render_children(sequence, sequence->nodes[best].id, out, length);
    }
}

int rp_sequence_render(const struct rp_sequence *sequence, unsigned char *out,
                       size_t capacity, size_t *length)
{
    unsigned char buffer[16];
    size_t count = 0;
    render_children(sequence, 0, buffer, &count);
    if (count > capacity)
        return RP_FULL;
    if (count)
        memcpy(out, buffer, count);
    *length = count;
    return RP_OK;
}

int rp_ot_insert(unsigned position, unsigned priority,
                 unsigned concurrent_position, unsigned concurrent_priority,
                 unsigned *result)
{
    if (priority == concurrent_priority)
        return RP_CONFLICT;
    if (position > concurrent_position ||
        (position == concurrent_position && priority > concurrent_priority))
    {
        if (position == UINT_MAX)
            return RP_RANGE;
        ++position;
    }
    *result = position;
    return RP_OK;
}

static uint32_t rotate(uint32_t value, unsigned count)
{
    return (value >> count) | (value << (32 - count));
}

static const uint32_t constants[64] = {
    0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1,
    0x923f82a4, 0xab1c5ed5, 0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3,
    0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174, 0xe49b69c1, 0xefbe4786,
    0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
    0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147,
    0x06ca6351, 0x14292967, 0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13,
    0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85, 0xa2bfe8a1, 0xa81a664b,
    0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
    0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a,
    0x5b9cca4f, 0x682e6ff3, 0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208,
    0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2};

int rp_sha256(const unsigned char *bytes, size_t length, unsigned char out[32])
{
    unsigned char padded[128] = {0};
    uint32_t h[8] = {0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a,
                     0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19};
    size_t total, offset;
    unsigned i;
    if (length > 65)
        return RP_RANGE;
    if (length)
        memcpy(padded, bytes, length);
    padded[length] = 0x80;
    total = length < 56 ? 64 : 128;
    padded[total - 2] = (unsigned char)((length * 8) >> 8);
    padded[total - 1] = (unsigned char)(length * 8);
    for (offset = 0; offset < total; offset += 64)
    {
        uint32_t words[64], a = h[0], b = h[1], c = h[2], d = h[3], e = h[4],
                            f = h[5], g = h[6], hh = h[7];
        for (i = 0; i < 16; ++i)
        {
            size_t p = offset + 4 * i;
            words[i] = (uint32_t)padded[p] << 24 |
                       (uint32_t)padded[p + 1] << 16 |
                       (uint32_t)padded[p + 2] << 8 | padded[p + 3];
        }
        for (i = 16; i < 64; ++i)
        {
            uint32_t s0 = rotate(words[i - 15], 7) ^ rotate(words[i - 15], 18) ^
                          (words[i - 15] >> 3),
                     s1 = rotate(words[i - 2], 17) ^ rotate(words[i - 2], 19) ^
                          (words[i - 2] >> 10);
            words[i] = words[i - 16] + s0 + words[i - 7] + s1;
        }
        for (i = 0; i < 64; ++i)
        {
            uint32_t s1 = rotate(e, 6) ^ rotate(e, 11) ^ rotate(e, 25),
                     choice = (e & f) ^ (~e & g),
                     t1 = hh + s1 + choice + constants[i] + words[i];
            uint32_t s0 = rotate(a, 2) ^ rotate(a, 13) ^ rotate(a, 22),
                     majority = (a & b) ^ (a & c) ^ (b & c), t2 = s0 + majority;
            hh = g;
            g = f;
            f = e;
            e = d + t1;
            d = c;
            c = b;
            b = a;
            a = t1 + t2;
        }
        h[0] += a;
        h[1] += b;
        h[2] += c;
        h[3] += d;
        h[4] += e;
        h[5] += f;
        h[6] += g;
        h[7] += hh;
    }
    for (i = 0; i < 8; ++i)
    {
        out[4 * i] = (unsigned char)(h[i] >> 24);
        out[4 * i + 1] = (unsigned char)(h[i] >> 16);
        out[4 * i + 2] = (unsigned char)(h[i] >> 8);
        out[4 * i + 3] = (unsigned char)h[i];
    }
    return RP_OK;
}

int rp_merkle(const uint32_t *leaves, size_t count, unsigned char out[32])
{
    unsigned char buffer[65];
    size_t split = 1;
    if (count > 8)
        return RP_RANGE;
    if (!count)
        return rp_sha256(NULL, 0, out);
    if (count == 1)
    {
        buffer[0] = 0;
        buffer[1] = (unsigned char)(leaves[0] >> 24);
        buffer[2] = (unsigned char)(leaves[0] >> 16);
        buffer[3] = (unsigned char)(leaves[0] >> 8);
        buffer[4] = (unsigned char)leaves[0];
        return rp_sha256(buffer + (MOD_MUTATION == 13),
                         MOD_MUTATION == 13 ? 4 : 5, out);
    }
    while (split * 2 < count)
        split *= 2;
    buffer[0] = 1;
    (void)rp_merkle(leaves, split, buffer + 1);
    (void)rp_merkle(leaves + split, count - split, buffer + 33);
    if (MOD_MUTATION == 14)
        memcpy(buffer + 33, buffer + 1, 32);
    return rp_sha256(buffer, 65, out);
}

static unsigned diff_tree(const uint32_t *left, const uint32_t *right,
                          size_t count, unsigned offset)
{
    unsigned char a[32], b[32];
    size_t split = 1;
    (void)rp_merkle(left, count, a);
    (void)rp_merkle(right, count, b);
    if (!memcmp(a, b, 32))
        return 0;
    if (count == 1)
        return 1u << offset;
    while (split * 2 < count)
        split *= 2;
    return diff_tree(left, right, split, offset) |
           diff_tree(left + split, right + split, count - split,
                     offset + (unsigned)split);
}

int rp_merkle_diff(const uint32_t *left, const uint32_t *right, size_t count,
                   unsigned *mask)
{
    if (count > 8)
        return RP_RANGE;
    *mask = count ? diff_tree(left, right, count, 0) : 0;
    return RP_OK;
}

uint32_t rp_zigzag(int32_t value)
{
    if (MOD_MUTATION == 16 && value < 0)
        return (uint32_t)(-(int64_t)value) * 2u;
    return ((uint32_t)value << 1) ^ (0u - (uint32_t)(value < 0));
}

int32_t rp_unzigzag(uint32_t value)
{
    return value & 1u ? -(int32_t)(value >> 1) - 1 : (int32_t)(value >> 1);
}

static int read_varint(const unsigned char *bytes, size_t length,
                       size_t *position, uint32_t *value)
{
    unsigned i;
    uint32_t result = 0;
    for (i = 0; i < 5; ++i)
    {
        unsigned byte;
        if (*position == length)
            return RP_INVALID;
        byte = bytes[(*position)++];
        if (i == 4 && byte > 15)
            return RP_RANGE;
        result |= (uint32_t)(byte & 127u) << (7 * i);
        if (!(byte & 128u))
        {
            if (i && byte == 0 && MOD_MUTATION != 15)
                return RP_INVALID;
            *value = result;
            return RP_OK;
        }
    }
    return RP_INVALID;
}

int rp_decode(const unsigned char *bytes, size_t length,
              struct rp_message *message)
{
    struct rp_message next = {{{0, 0}}, 0};
    size_t position = 0;
    unsigned seen = 0;
    while (position < length)
    {
        uint32_t tag, value;
        int status = read_varint(bytes, length, &position, &tag);
        if (status != RP_OK)
            return status;
        if ((tag & 7u) != 0 || tag < 8 || tag > 120 ||
            (seen & (1u << (tag >> 3))))
            return RP_INVALID;
        status = read_varint(bytes, length, &position, &value);
        if (status != RP_OK)
            return status;
        if (next.count == 8)
            return RP_FULL;
        seen |= 1u << (tag >> 3);
        next.fields[next.count++] = (struct rp_field){tag >> 3, value};
    }
    if ((seen & 6u) != 6u)
        return RP_INVALID;
    *message = next;
    return RP_OK;
}

static void write_varint(uint32_t value, unsigned char bytes[48],
                         size_t *position)
{
    do
    {
        unsigned char byte = (unsigned char)(value & 127u);
        value >>= 7;
        if (value)
            byte |= 128u;
        bytes[(*position)++] = byte;
    } while (value);
}

int rp_encode(const struct rp_message *message, unsigned char *bytes,
              size_t capacity, size_t *length)
{
    unsigned seen = 0, i, tag;
    unsigned char buffer[48];
    size_t position = 0;
    if (message->count > 8)
        return RP_INVALID;
    for (i = 0; i < message->count; ++i)
    {
        tag = message->fields[i].tag;
        if (tag < 1 || tag > 15 || (seen & (1u << tag)))
            return RP_INVALID;
        seen |= 1u << tag;
    }
    if ((seen & 6u) != 6u)
        return RP_INVALID;
    for (tag = 1; tag <= 15; ++tag)
        for (i = 0; i < message->count; ++i)
            if (message->fields[i].tag == tag)
            {
                write_varint(tag << 3, buffer, &position);
                write_varint(message->fields[i].value, buffer, &position);
            }
    if (position > capacity)
        return RP_FULL;
    memcpy(bytes, buffer, position);
    *length = position;
    return RP_OK;
}

int rp_step(struct rp_state state, int movement, struct rp_state *result,
            struct rp_effect *effect)
{
    if (state.tick >= 8 || state.position >= 16 || state.score > 65535 ||
        movement < -1 || movement > 1)
        return RP_INVALID;
    state.position = (unsigned)((int)state.position + 16 + movement) % 16;
    state.random = state.random * UINT32_C(1664525) + UINT32_C(1013904223);
    if (state.score > 65535 - (state.random & 3u))
        return RP_RANGE;
    if (MOD_MUTATION != 17)
        state.score += state.random & 3u;
    ++state.tick;
    *result = state;
    *effect = (struct rp_effect){state.tick, state.score};
    return RP_OK;
}

int rp_replay(struct rp_state initial, const int *movements, size_t count,
              struct rp_state *result, struct rp_effect *effects)
{
    struct rp_state state = initial;
    struct rp_effect staged[8];
    size_t i;
    if (count > 8)
        return RP_RANGE;
    for (i = 0; i < count; ++i)
    {
        int status = rp_step(state, movements[i], &state, &staged[i]);
        if (status != RP_OK)
            return status;
    }
    if (count)
        memcpy(effects, staged, count * sizeof *effects);
    *result = state;
    return RP_OK;
}

int rp_checkpoint(struct d_hamt *arena, unsigned root, struct rp_state state,
                  unsigned fail_ordinal, unsigned *new_root)
{
    unsigned saved_used = arena->used, saved_fail = arena->fail_after, first,
             second;
    int status;
    if (state.tick > 8 || state.position >= 16 || state.score > 65535 ||
        fail_ordinal > 6)
        return RP_INVALID;
    arena->fail_after = fail_ordinal <= 3 ? fail_ordinal : 0;
    status = d_hamt_update(arena, root, 0, (int)state.position, &first);
    if (status == D_OK)
    {
        arena->fail_after = fail_ordinal > 3 ? fail_ordinal - 3 : 0;
        status = d_hamt_update(arena, first, 1, (int)state.score, &second);
    }
    arena->fail_after = saved_fail;
    if (status != D_OK)
    {
        if (MOD_MUTATION != 18)
            arena->used = saved_used;
        return RP_FAILED;
    }
    *new_root = second;
    return RP_OK;
}
