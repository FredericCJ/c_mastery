#ifndef CMASTERY_OWNERSHIP_FRONTIER_H
#define CMASTERY_OWNERSHIP_FRONTIER_H
#include "ownership.h"

enum ocf_status
{
    OCF_OK,
    OCF_INVALID,
    OCF_NOMEM,
    OCF_INIT_FAILED,
    OCF_NOT_IMPLEMENTED
};

struct ocf_view
{
    const int32_t *block[4];
    size_t length[4];
    unsigned initialized;
};

typedef int (*ocf_initialize_fn)(void *, unsigned, int32_t *, size_t,
                                 const struct ocf_view *);
typedef void (*ocf_destroy_fn)(void *, unsigned, int32_t *, size_t,
                               const struct ocf_view *);

struct ocf_callbacks
{
    void *context;
    ocf_initialize_fn initialize;
    ocf_destroy_fn destroy;
};

struct ocf_plan
{
    unsigned order[4];
    size_t length[4];
};

struct ocf_owner
{
    int32_t *block[4];
    size_t length[4];
    unsigned order[4];
    unsigned count, acquired, initialized;
    struct v4_allocator allocator;
    struct ocf_callbacks callbacks;
};
/* Fresh zero owner. IDs A=0 B=1 C=2 D=3. A precedes B/C; B precedes D.
 * See FRONTIER.md for authentic controls, callback and partial-init obligations. */
enum ocf_status ocf_construct(struct ocf_owner *, struct v4_allocator,
                              struct ocf_callbacks, const struct ocf_plan *);
enum ocf_status ocf_destroy(struct ocf_owner *);
#endif
