#ifndef CMASTERY_RWPHASE_DEPTH_H
#define CMASTERY_RWPHASE_DEPTH_H
#include "cmx.h"

enum
{
    CPF_IDLE,
    CPF_PENDING,
    CPF_TERMINAL,
    CPF_HELD
};

enum
{
    CPF_READ,
    CPF_WRITE,
    CPF_UPGRADE
};

enum
{
    CPF_STALE = CMX_UNIMPLEMENTED + 1,
    CPF_RETIRED
};

struct cpf_slot
{
    unsigned phase, mode, generation;
    uint32_t serial, arrival;
    int result;
    cmx_latch completion;
};

struct cmx_cpf
{
    cmx_kernel *kernel;
    struct cpf_slot slot[CMX_TASKS];
    uint32_t registered, readers, admissions, phases;
    int writer, upgrader;
    unsigned last_phase, closing, live;
};

struct cpf_handle
{
    struct cmx_cpf *owner;
    unsigned task, generation;
};

struct cpf_ticket
{
    struct cpf_handle handle;
    uint32_t serial;
};

int cpf_init(struct cmx_cpf *, cmx_kernel *);
int cpf_register(struct cmx_cpf *, unsigned, struct cpf_handle *);
int cpf_begin(struct cpf_handle, unsigned mode, cmx_deadline,
              struct cpf_ticket *);
int cpf_upgrade(struct cpf_handle, cmx_deadline, struct cpf_ticket *);
int cpf_result(struct cpf_ticket);
int cpf_cancel(struct cpf_ticket);
int cpf_unlock(struct cpf_handle);
int cpf_close(struct cmx_cpf *);
int cpf_release(struct cpf_handle *);
int cpf_destroy(struct cmx_cpf *);
#endif
