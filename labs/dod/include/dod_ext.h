#ifndef CMASTERY_DOD_EXT_H
#define CMASTERY_DOD_EXT_H
#include "dod.h"
#define D_ROW_FIELDS(F) F(int32_t, x) F(int32_t, velocity) F(unsigned, cold)
#define D_FIELD(type, name) type name;

struct d_row
{
    D_ROW_FIELDS(D_FIELD)
};

#undef D_FIELD
#define D_FIELD(type, name) type name[8];

struct d_columns8
{
    D_ROW_FIELDS(D_FIELD) size_t count;
};

#undef D_FIELD

struct d_tile
{
    int32_t x[3], velocity[3];
    unsigned cold[3];
};

struct d_tiles
{
    struct d_tile tiles[3];
    size_t count;
    unsigned has_cold;
};

int d_rows_to_columns(const struct d_row *, size_t, struct d_columns8 *);
int d_columns_to_rows(const struct d_columns8 *, struct d_row *, size_t);
int d_rows_step(struct d_row *, size_t, int32_t);
int d_columns8_step(struct d_columns8 *, int32_t);
int d_rows_step_many(struct d_row *, size_t, int32_t);
int d_columns_step_many(int32_t *, const int32_t *, size_t, int32_t);
int d_rows_to_arrays(const struct d_row *, size_t, int32_t *, int32_t *,
                     unsigned *, size_t);
int d_arrays_to_rows(const int32_t *, const int32_t *, const unsigned *, size_t,
                     struct d_row *, size_t);
int d_rows_to_tiles(const struct d_row *, size_t, unsigned, struct d_tiles *);
int d_tiles_to_rows(const struct d_tiles *, struct d_row *, size_t);
uint32_t d_swar16_add(uint32_t, uint32_t);

typedef void (*d_destroy_fn)(void *, void *);

struct d_segment
{
    struct d_sparse pages[2];
    void *resource[2][4];
    d_destroy_fn destroy;
    void *context;
};

struct d_segment_handle
{
    const struct d_segment *owner;
    unsigned page;
    struct d_handle handle;
};

void d_segment_init(struct d_segment *, d_destroy_fn, void *);
int d_segment_add(struct d_segment *, int, void *, struct d_segment_handle *);
int d_segment_get(const struct d_segment *, struct d_segment_handle, int *);
int d_segment_borrow(const struct d_segment *, struct d_segment_handle,
                     const int **);
int d_segment_remove(struct d_segment *, struct d_segment_handle);

struct d_query_world
{
    struct d_ecs ecs;
    unsigned version;
};

struct d_query
{
    const struct d_query_world *owner;
    unsigned version, required, excluded, ids[4], count;
};

void d_query_world_init(struct d_query_world *);
int d_query_set(struct d_query_world *, unsigned, unsigned, int, int);
int d_query_get(const struct d_query_world *, unsigned required,
                unsigned excluded, struct d_query *);
#endif
