#include "adt.h"
#include <limits.h>
#ifndef MOD_MUTATION
#define MOD_MUTATION 0
#endif

int ad_measure(struct ad_value value, uint64_t *out) {
    uint64_t result;
    switch (value.tag) {
    case AD_EMPTY: result = 0; break;
    case AD_NUMBER: result = value.data.number; break;
    case AD_PAIR: result = (uint64_t)value.data.pair.x +
        (MOD_MUTATION == 1 ? 0 : value.data.pair.y); break;
    default: if (MOD_MUTATION != 2) return 0; result = 0; break;
    }
    *out = result; return 1;
}
int ad_number(struct ad_value value, uint32_t *out) {
    if (value.tag != AD_NUMBER) return 0;
    *out = value.data.number; return 1;
}
int ad_milliseconds(struct ad_seconds value, struct ad_millis *out) {
    if (MOD_MUTATION != 3 && value.value > UINT32_MAX / 1000u) return 0;
    out->value = value.value * (MOD_MUTATION == 4 ? 100u : 1000u); return 1;
}
int ad_flags(uint32_t raw, struct ad_flags *out) {
    if (MOD_MUTATION != 5 && (raw & ~UINT32_C(7)) != 0) return 0;
    if (MOD_MUTATION == 6 && raw == 0) return 0;
    out->bits = raw & UINT32_C(7); return 1;
}
int ad_text(const unsigned char *bytes, size_t length, struct ad_text *out) {
    size_t i = 0, count = 0;
    if (length > 65535 || (length != 0 && bytes == NULL)) return 0;
    while (i < length) {
        uint32_t code, minimum; unsigned tail, j; unsigned char first = bytes[i++];
        if (first < 0x80) { code = first; minimum = 0; tail = 0; }
        else if (first >= 0xc0 && first <= 0xdf) { code = first & 31u; minimum = 0x80; tail = 1; }
        else if (first <= 0xef && first >= 0xe0) { code = first & 15u; minimum = 0x800; tail = 2; }
        else if (first >= 0xf0 && first <= 0xf4) { code = first & 7u; minimum = 0x10000; tail = 3; }
        else return 0;
        if (tail > length - i) return 0;
        for (j = 0; j < tail; ++j) {
            unsigned char c = bytes[i++];
            if ((c & 0xc0u) != 0x80u) return 0;
            code = (code << 6) | (c & 63u);
        }
        if (MOD_MUTATION != 7 && code < minimum) return 0;
        if (code > 0x10ffff || (MOD_MUTATION != 8 && code >= 0xd800 && code <= 0xdfff)) return 0;
        ++count;
    }
    out->bytes = bytes; out->length = length; out->scalars = count; return 1;
}
void ad_pool_init(struct ad_pool *pool) {
    unsigned i;
    for (i = 0; i < 4; ++i) { pool->slot[i].generation = 1; pool->slot[i].live = 0; pool->slot[i].retired = 0; pool->slot[i].value = 0; }
}
int ad_pool_acquire(struct ad_pool *pool, uint32_t value, struct ad_brand *out) {
    unsigned i;
    for (i = 0; i < 4; ++i) if (!pool->slot[i].live && !pool->slot[i].retired) {
        pool->slot[i].live = 1; pool->slot[i].value = value;
        out->owner = pool; out->index = i; out->generation = pool->slot[i].generation; return 1;
    }
    return 0;
}
static int ad_brand_valid(const struct ad_pool *pool, struct ad_brand handle) {
    return (MOD_MUTATION == 9 || handle.owner == pool) && handle.index < 4 &&
        pool->slot[handle.index].live &&
        (MOD_MUTATION == 10 || handle.generation == pool->slot[handle.index].generation);
}
int ad_pool_get(const struct ad_pool *pool, struct ad_brand handle, uint32_t *out) {
    if (!ad_brand_valid(pool, handle)) return 0;
    *out = pool->slot[handle.index].value; return 1;
}
int ad_pool_release(struct ad_pool *pool, struct ad_brand handle) {
    struct ad_slot *slot;
    if (!ad_brand_valid(pool, handle)) return 0;
    slot = &pool->slot[handle.index]; slot->live = 0;
    if (slot->generation == UINT_MAX) slot->retired = 1;
    else ++slot->generation;
    return 1;
}
int ad_view(const uint32_t *data, size_t capacity, size_t rows, size_t columns,
            size_t row_stride, size_t column_stride, struct ad_view *out) {
    size_t a = 0, b = 0;
    if (rows && columns) {
        if (!data || !row_stride || !column_stride) return 0;
        if (rows - 1 > SIZE_MAX / row_stride || columns - 1 > SIZE_MAX / column_stride) return 0;
        a = (rows - 1) * row_stride; b = (columns - 1) * column_stride;
        if (a > SIZE_MAX - b || a + b == SIZE_MAX) return 0;
        if (MOD_MUTATION != 12 && a + b + 1 > capacity) return 0;
    }
    out->data = data; out->capacity = capacity; out->rows = rows; out->columns = columns;
    out->row_stride = row_stride; out->column_stride = column_stride; return 1;
}
int ad_view_get(struct ad_view view, size_t row, size_t column, uint32_t *out) {
    size_t offset;
    if (row >= view.rows || column >= view.columns) return 0;
    offset = row * (MOD_MUTATION == 11 ? view.columns : view.row_stride) + column * view.column_stride;
    if (offset >= view.capacity) return 0;
    *out = view.data[offset]; return 1;
}
int ad_eval(const struct ad_node *nodes, size_t count, struct ad_node_index root, uint32_t *out) {
    uint32_t values[32] = {0}; size_t i;
    if (!nodes || count == 0 || count > 32 || root.index >= count) return 0;
    for (i = 0; i < count; ++i) {
        if (nodes[i].kind == 0) values[i] = nodes[i].value;
        else if (nodes[i].kind == 1) {
            unsigned a = nodes[i].left.index, b = nodes[i].right.index;
            if (a >= count || b >= count) return 0;
            if (MOD_MUTATION != 13 && (a >= i || b >= i)) return 0;
            if (MOD_MUTATION != 14 && values[a] > UINT32_MAX - values[b]) return 0;
            values[i] = values[a] + values[b];
        } else return 0;
    }
    *out = values[root.index]; return 1;
}
struct ad_pair ad_lens_x(struct ad_pair source, uint32_t replacement) {
    source.x = replacement;
    if (MOD_MUTATION == 15) source.y = replacement;
    return source;
}
struct ad_value ad_review_number(uint32_t value) {
    struct ad_value result; result.tag = AD_NUMBER; result.data.number = value; return result;
}
int ad_preview_number(struct ad_value value, uint32_t *out) {
    if (MOD_MUTATION == 16 && value.tag == AD_EMPTY) { *out = 0; return 1; }
    return ad_number(value, out);
}
int ad_evolve_read(const unsigned char *bytes, size_t length, struct ad_evolving *out) {
    unsigned tag; size_t payload;
    if (length < 2 || !bytes) return 0;
    tag = bytes[0]; payload = bytes[1];
    if (payload != length - 2 || tag == 0) return 0;
    if (MOD_MUTATION != 18 && ((tag == 1 && payload != 2) || (tag == 2 && payload != 0))) return 0;
    if (tag > 2 && (tag & 128u)) return 0;
    out->tag = tag; out->payload = bytes + 2;
    out->length = (MOD_MUTATION == 17 && tag > 2) ? 0 : payload; return 1;
}
int ad_evolve_write(struct ad_evolving value, unsigned char *bytes, size_t capacity, size_t *used) {
    size_t i;
    if (value.tag == 0 || value.tag > 127 || value.length > 255 ||
        (value.tag == 1 && value.length != 2) || (value.tag == 2 && value.length != 0) ||
        (value.length && !value.payload) || capacity < value.length + 2 || !bytes) return 0;
    bytes[0] = (unsigned char)value.tag; bytes[1] = (unsigned char)value.length;
    for (i = 0; i < value.length; ++i) bytes[i+2] = value.payload[i];
    *used = value.length + 2; return 1;
}
enum ad_packet_error ad_packet(const unsigned char *bytes, size_t length, struct ad_packet *out) {
    size_t payload; unsigned kind;
    if (length < 3 || !bytes) return AD_PACKET_HEADER;
    kind = bytes[0]; payload = ((size_t)bytes[1] << 8) | bytes[2];
    if (kind != 1 && kind != 2) return AD_PACKET_KIND;
    if (MOD_MUTATION == 19 ? payload > length - 3 : payload != length - 3) return AD_PACKET_LENGTH;
    if (MOD_MUTATION != 20 && kind == 2 && payload != 0) return AD_PACKET_ACK;
    out->kind = kind; out->payload = bytes + 3; out->length = payload; return AD_PACKET_OK;
}
