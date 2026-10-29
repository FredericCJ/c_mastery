#ifndef CHECKPOINT_MEMORY_H
#define CHECKPOINT_MEMORY_H
#include <stddef.h>
enum { CPM_SLOTS=4, CPM_GENERATION_MAX=3 };
enum cpm_status { CPM_OK, CPM_FULL, CPM_BUSY, CPM_INVALID };
struct cpm_span { size_t offset,length; unsigned generation; };
struct cpm_arena { unsigned char *bytes; size_t capacity,used,alignment; unsigned generation,pins; };
struct cpm_record { int value; };
struct cpm_handle { unsigned slot,generation; };
struct cpm_pool {
    struct cpm_record objects[CPM_SLOTS];
    unsigned state[CPM_SLOTS],generation[CPM_SLOTS],pins[CPM_SLOTS];
    unsigned attempts,fail_at;
};
struct cpm_ring { struct cpm_handle items[CPM_SLOTS]; size_t head,count; };
enum cpm_status cpm_arena_init(struct cpm_arena *,unsigned char *,size_t,size_t,size_t);
enum cpm_status cpm_reserve(struct cpm_arena *,size_t,struct cpm_span *);
enum cpm_status cpm_view(const struct cpm_arena *,struct cpm_span,const unsigned char **);
enum cpm_status cpm_pin(struct cpm_arena *,struct cpm_span);
enum cpm_status cpm_unpin(struct cpm_arena *);
enum cpm_status cpm_reset(struct cpm_arena *);
void cpm_pool_init(struct cpm_pool *);
void cpm_ring_init(struct cpm_ring *);
void cpm_fail_at(struct cpm_pool *,unsigned);
enum cpm_status cpm_allocate(struct cpm_pool *,int,struct cpm_handle *);
enum cpm_status cpm_lookup(const struct cpm_pool *,struct cpm_handle,const struct cpm_record **);
enum cpm_status cpm_pool_pin(struct cpm_pool *,struct cpm_handle);
enum cpm_status cpm_pool_unpin(struct cpm_pool *,struct cpm_handle);
enum cpm_status cpm_release(struct cpm_pool *,struct cpm_handle);
enum cpm_status cpm_prefix(struct cpm_pool *,struct cpm_ring *,const int *,size_t,size_t *);
enum cpm_status cpm_atomic(struct cpm_pool *,struct cpm_ring *,const int *,size_t);
enum cpm_status cpm_pop(struct cpm_ring *,struct cpm_handle *);
#endif
