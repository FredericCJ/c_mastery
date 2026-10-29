#ifndef CMASTERY_RWLOCK_DEPTH_H
#define CMASTERY_RWLOCK_DEPTH_H
#include "cmx.h"

enum
{
    CDR_IDLE,
    CDR_PENDING,
    CDR_TERMINAL,
    CDR_HELD
};

enum
{
    CDR_READ,
    CDR_WRITE
};

enum
{
    CDR_STALE = CMX_UNIMPLEMENTED + 1,
    CDR_RETIRED
};

struct cdr_slot
{
    unsigned phase, mode, generation;
    uint32_t serial;
    int result;
};

struct cd_rw
{
    cmx_kernel *kernel;
    cmx_rwlock raw;
    struct cdr_slot slot[CMX_TASKS];
    uint32_t registered;
    unsigned read_limit, live, closing;
};

struct cdr_handle
{
    struct cd_rw *owner;
    unsigned task, generation;
};

struct cdr_ticket
{
    struct cdr_handle handle;
    uint32_t serial;
};

int cdr_init(struct cd_rw *, cmx_kernel *, unsigned read_limit);
int cdr_register(struct cd_rw *, unsigned, struct cdr_handle *);
int cdr_begin(struct cdr_handle, unsigned mode, cmx_deadline,
              struct cdr_ticket *);
int cdr_result(struct cdr_ticket);
int cdr_cancel(struct cdr_ticket);
int cdr_unlock(struct cdr_handle);
int cdr_close(struct cd_rw *);
int cdr_release(struct cdr_handle *);
int cdr_destroy(struct cd_rw *);
#endif
