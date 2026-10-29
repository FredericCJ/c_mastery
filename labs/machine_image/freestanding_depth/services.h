#ifndef CMASTERY_FREE_SERVICES_H
#define CMASTERY_FREE_SERVICES_H
#include "free.h"

/* Synchronous callback: borrows the offered suffix only until return. On every
 * status, accepted must be <= count; accepted bytes cannot be replayed. No
 * callback may reenter this owner. FR_OK requires positive progress. */
struct fr_progress
{
    enum fr_status status;
    size_t accepted;
};
typedef struct fr_progress (*fr_output)(void *, const unsigned char *, size_t);

struct fr_service
{
    unsigned char bytes[24];
    size_t used, offset;
    unsigned ready, active;
    fr_output output;
    void *context;
};
enum fr_status fr_service_init(struct fr_service *, fr_output, void *);
enum fr_status fr_service_ready(struct fr_service *);
enum fr_status fr_service_begin(struct fr_service *, const unsigned char *,
                                size_t);
enum fr_status fr_service_pump(struct fr_service *);
/* Always refuses; neither owner nor out changes, even after ready. */
enum fr_status fr_heap_extend(struct fr_service *, size_t, void **);

struct fr_arena
{
    _Alignas(max_align_t) unsigned char bytes[64];
    size_t used;
};
enum fr_status fr_allocate(struct fr_arena *, size_t, size_t, void **);

/* Addresses below are offsets into RAM. ROM is a separate immutable object.
 * Data/BSS/retained/stack must be pairwise disjoint; stack endpoints aligned8.
 * Admission is atomic. A cut after k writes models reset, not C concurrency. */
struct fr_region
{
    size_t begin, end;
};

struct fr_start_plan
{
    struct fr_region data, bss, retained, stack;
    size_t load;
};
enum fr_status fr_startup(unsigned char *, size_t, const unsigned char *,
                          size_t, const struct fr_start_plan *, size_t);

struct fr_task
{
    unsigned active, generation, value, hooks;
};

struct fr_tasks
{
    struct fr_task slot[4];
};

struct fr_handle
{
    unsigned index, generation;
};
enum fr_status fr_task_create(struct fr_tasks *, unsigned, struct fr_handle *);
enum fr_status fr_task_hook(struct fr_tasks *, struct fr_handle,
                            const struct fr_service *);
enum fr_status fr_task_read(const struct fr_tasks *, struct fr_handle,
                            unsigned *);
enum fr_status fr_task_destroy(struct fr_tasks *, struct fr_handle);
#endif
