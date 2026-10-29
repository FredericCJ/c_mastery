#include "adt.h"

/* Intentionally incomplete; implement the canonical exercise contracts. */
int ad_measure(struct ad_value value, uint64_t *out)
{
    (void)value;
    (void)out;
    return 0;
}

int ad_number(struct ad_value value, uint32_t *out)
{
    (void)value;
    (void)out;
    return 0;
}

int ad_milliseconds(struct ad_seconds value, struct ad_millis *out)
{
    (void)value;
    (void)out;
    return 0;
}

int ad_flags(uint32_t raw, struct ad_flags *out)
{
    (void)raw;
    (void)out;
    return 0;
}

int ad_text(const unsigned char *bytes, size_t length, struct ad_text *out)
{
    (void)bytes;
    (void)length;
    (void)out;
    return 0;
}

void ad_pool_init(struct ad_pool *pool)
{
    (void)pool;
}

int ad_pool_acquire(struct ad_pool *pool, uint32_t value, struct ad_brand *out)
{
    (void)pool;
    (void)value;
    (void)out;
    return 0;
}

int ad_pool_get(const struct ad_pool *pool, struct ad_brand handle,
                uint32_t *out)
{
    (void)pool;
    (void)handle;
    (void)out;
    return 0;
}

int ad_pool_release(struct ad_pool *pool, struct ad_brand handle)
{
    (void)pool;
    (void)handle;
    return 0;
}

int ad_view(const uint32_t *data, size_t capacity, size_t rows, size_t columns,
            size_t row_stride, size_t column_stride, struct ad_view *out)
{
    (void)data;
    (void)capacity;
    (void)rows;
    (void)columns;
    (void)row_stride;
    (void)column_stride;
    (void)out;
    return 0;
}

int ad_view_get(struct ad_view view, size_t row, size_t column, uint32_t *out)
{
    (void)view;
    (void)row;
    (void)column;
    (void)out;
    return 0;
}

int ad_eval(const struct ad_node *nodes, size_t count,
            struct ad_node_index root, uint32_t *out)
{
    (void)nodes;
    (void)count;
    (void)root;
    (void)out;
    return 0;
}

struct ad_pair ad_lens_x(struct ad_pair source, uint32_t replacement)
{
    (void)source;
    (void)replacement;
    return (struct ad_pair){0, 0};
}

struct ad_value ad_review_number(uint32_t value)
{
    (void)value;
    return (struct ad_value){AD_EMPTY, {.number = 0}};
}

int ad_preview_number(struct ad_value value, uint32_t *out)
{
    (void)value;
    (void)out;
    return 0;
}

int ad_evolve_read(const unsigned char *bytes, size_t length,
                   struct ad_evolving *out)
{
    (void)bytes;
    (void)length;
    (void)out;
    return 0;
}

int ad_evolve_write(struct ad_evolving value, unsigned char *bytes,
                    size_t capacity, size_t *used)
{
    (void)value;
    (void)bytes;
    (void)capacity;
    (void)used;
    return 0;
}

enum ad_packet_error ad_packet(const unsigned char *bytes, size_t length,
                               struct ad_packet *out)
{
    (void)bytes;
    (void)length;
    (void)out;
    return AD_PACKET_HEADER;
}
