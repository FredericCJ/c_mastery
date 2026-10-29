#ifndef CMASTERY_OBJECT_RUNTIME_FINALIZERS_H
#define CMASTERY_OBJECT_RUNTIME_FINALIZERS_H
#include "object_runtime.h"

struct orf_handle
{
    uint8_t slot, generation;
};
struct orf_heap;
typedef enum or_status (*orf_finalizer_fn)(struct orf_heap *, struct orf_handle,
                                           void *);

struct orf_heap
{
    struct or_weak weak;
    orf_finalizer_fn callback[16];
    void *context[16];
    uint8_t generation[16], busy, closed;
    uint16_t resources, cleanup_pending, callbacks_run, resources_closed;
};

void orf_init(struct orf_heap *heap);
enum or_status orf_new(struct orf_heap *, orf_finalizer_fn, void *,
                       int resource, int fail, struct orf_handle *);
enum or_status orf_root(struct orf_heap *, struct orf_handle, int present);
enum or_status orf_edge(struct orf_heap *, struct orf_handle, unsigned field,
                        struct orf_handle);
enum or_status orf_pair(struct orf_heap *, unsigned index,
                        struct orf_handle key, struct orf_handle value);
enum or_status orf_weak(struct orf_heap *, unsigned index, struct orf_handle);
enum or_status orf_collect(struct orf_heap *, uint16_t *reclaimed);
enum or_status orf_run_one(struct orf_heap *, struct orf_handle *,
                           enum or_status *callback_status);
enum or_status orf_close_resource(struct orf_heap *, struct orf_handle);
enum or_status orf_cleanup(struct orf_heap *, unsigned budget,
                           unsigned *released);
enum or_status orf_close(struct orf_heap *);
#endif
