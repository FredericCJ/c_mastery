#ifndef CMASTERY_ONCE_DEPTH_H
#define CMASTERY_ONCE_DEPTH_H
#include "cmx.h"
#include "bridge.h"

enum
{
    CDO_IDLE,
    CDO_OWNER,
    CDO_PENDING,
    CDO_DONE
};

enum
{
    CDO_INIT = CMX_UNIMPLEMENTED + 1,
    CDO_FAILED,
    CDO_STALE,
    CDO_RETIRED,
    CDO_AGAIN
};

struct cdo_cell
{
    uint32_t value;
};
typedef struct cdo_cell *(*cdo_allocate_fn)(void *);
typedef void (*cdo_drop_fn)(void *, struct cdo_cell *);

struct cdo_table
{
    uint32_t entries[8], count, checksum;
};

struct cdo_slot
{
    unsigned phase, task_generation;
    uint32_t epoch, serial;
    int result;
};

struct cd_once
{
    cmx_kernel *kernel;
    cmx_once wait;
    struct mb_once publication;
    struct cdo_table table;
    struct cdo_slot slot[3];
    struct cdo_cell *resource[3];
    cdo_allocate_fn allocate;
    cdo_drop_fn drop;
    void *allocator_context;
    uint32_t epoch, attempts;
    unsigned registered, stage, resources, resetting, closing, live;
};

struct cdo_handle
{
    struct cd_once *owner;
    unsigned task;
    uint32_t epoch;
};

struct cdo_ticket
{
    struct cdo_handle handle;
    uint32_t epoch, serial;
};

int cdo_init(struct cd_once *, cmx_kernel *, cdo_allocate_fn, cdo_drop_fn,
             void *);
int cdo_register(struct cd_once *, unsigned task, struct cdo_handle *);
int cdo_enter(struct cdo_handle, cmx_deadline, struct cdo_ticket *);
int cdo_allocate(struct cdo_handle);
int cdo_write(struct cdo_handle, unsigned index, uint32_t value);
int cdo_seal(struct cdo_handle);
int cdo_finish(struct cdo_handle, unsigned success);
int cdo_result(struct cdo_ticket, struct cdo_table *);
int cdo_read(struct cdo_handle, struct cdo_table *);
int cdo_cancel(struct cdo_ticket);
int cdo_prepare_reset(struct cd_once *);
int cdo_release(struct cdo_handle *);
int cdo_reset(struct cd_once *);
int cdo_close(struct cd_once *);
int cdo_destroy(struct cd_once *);
#endif
