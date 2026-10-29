#include "dod_ext.h"

int d_rows_to_columns(const struct d_row *a, size_t b, struct d_columns8 *c)
{
    (void)a;
    (void)b;
    (void)c;
    return D_FAILED;
}

int d_columns_to_rows(const struct d_columns8 *a, struct d_row *b, size_t c)
{
    (void)a;
    (void)b;
    (void)c;
    return D_FAILED;
}

int d_rows_step(struct d_row *a, size_t b, int32_t c)
{
    (void)a;
    (void)b;
    (void)c;
    return D_FAILED;
}

int d_columns8_step(struct d_columns8 *a, int32_t b)
{
    (void)a;
    (void)b;
    return D_FAILED;
}

int d_rows_step_many(struct d_row *a, size_t b, int32_t c)
{
    (void)a;
    (void)b;
    (void)c;
    return D_FAILED;
}

int d_columns_step_many(int32_t *a, const int32_t *b, size_t c, int32_t d)
{
    (void)a;
    (void)b;
    (void)c;
    (void)d;
    return D_FAILED;
}

int d_rows_to_arrays(const struct d_row *r, size_t n, int32_t *x, int32_t *v,
                     unsigned *c, size_t cap)
{
    (void)r;
    (void)n;
    (void)x;
    (void)v;
    (void)c;
    (void)cap;
    return D_FAILED;
}

int d_arrays_to_rows(const int32_t *x, const int32_t *v, const unsigned *c,
                     size_t n, struct d_row *r, size_t cap)
{
    (void)x;
    (void)v;
    (void)c;
    (void)n;
    (void)r;
    (void)cap;
    return D_FAILED;
}

int d_rows_to_tiles(const struct d_row *a, size_t b, unsigned c,
                    struct d_tiles *d)
{
    (void)a;
    (void)b;
    (void)c;
    (void)d;
    return D_FAILED;
}

int d_tiles_to_rows(const struct d_tiles *a, struct d_row *b, size_t c)
{
    (void)a;
    (void)b;
    (void)c;
    return D_FAILED;
}

uint32_t d_swar16_add(uint32_t a, uint32_t b)
{
    (void)a;
    (void)b;
    return 0;
}

void d_segment_init(struct d_segment *a, d_destroy_fn b, void *c)
{
    (void)a;
    (void)b;
    (void)c;
}

int d_segment_add(struct d_segment *a, int b, void *c,
                  struct d_segment_handle *d)
{
    (void)a;
    (void)b;
    (void)c;
    (void)d;
    return D_FAILED;
}

int d_segment_get(const struct d_segment *a, struct d_segment_handle b, int *c)
{
    (void)a;
    (void)b;
    (void)c;
    return D_FAILED;
}

int d_segment_borrow(const struct d_segment *a, struct d_segment_handle b,
                     const int **c)
{
    (void)a;
    (void)b;
    (void)c;
    return D_FAILED;
}

int d_segment_remove(struct d_segment *a, struct d_segment_handle b)
{
    (void)a;
    (void)b;
    return D_FAILED;
}

void d_query_world_init(struct d_query_world *a)
{
    (void)a;
}

int d_query_set(struct d_query_world *a, unsigned b, unsigned c, int d, int e)
{
    (void)a;
    (void)b;
    (void)c;
    (void)d;
    (void)e;
    return D_FAILED;
}

int d_query_get(const struct d_query_world *a, unsigned b, unsigned c,
                struct d_query *d)
{
    (void)a;
    (void)b;
    (void)c;
    (void)d;
    return D_FAILED;
}
