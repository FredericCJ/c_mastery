#ifndef CMK2_ABI_H
#define CMK2_ABI_H
#include "cmk_abi.h"

/* Version 2 uses new numbers; ABI v1 remains source/binary compatible.
 * Fixed x86-64 teaching profile. Entry functions must call THREAD_EXIT.
 * No POSIX compatibility, asynchronous cancellation or robust mutex claim. */
enum
{
    CMK2_THREAD_CREATE = 0x200,
    CMK2_THREAD_EXIT,
    CMK2_THREAD_JOIN,
    CMK2_CHANNEL_CREATE,
    CMK2_CHANNEL_GRANT,
    CMK2_CHANNEL_SEND,
    CMK2_CHANNEL_RECV,
    CMK2_CHANNEL_CLOSE,
    CMK2_RESERVE_ZERO,
    CMK2_FORK_SINGLE,
    CMK2_WAIT_PROCESS,
    CMK2_BLOCK_SUBMIT,
    CMK2_BLOCK_WAIT,
    CMK2_SHM_CREATE,
    CMK2_SHM_MAP,
    CMK2_SHM_CLOSE,
    CMK2_CPU_INFO,
    CMK2_SET_AFFINITY,
    CMK2_SPAWN,
    CMK2_CHANNEL_DESTROY,
    CMK2_TLB_SYNC,
    CMK2_BLOCK_STATUS
};

struct cmk2_block_request
{
    uint32_t type, page;
    uint64_t buffer;
};

/* One DMA request may be in flight. Submit copies the request/write payload;
 * wait blocks the task. Device MSI-X advances completion; wait blocks until completion or timeout. */
static inline int cmk2_block(void *handle, unsigned type, unsigned page,
                             void *buffer)
{
    struct cmk2_block_request r = {type, page, (uintptr_t)buffer};
    int64_t ticket;
    unsigned tries = 0;
    do
    {
        ticket =
            cmk_call(CMK2_BLOCK_SUBMIT, (uintptr_t)handle, (uintptr_t)&r, 0);
        if (ticket == -2)
            cmk_call(CMK_YIELD, 0, 0, 0);
    } while (ticket == -2 && ++tries < 10000);
    return ticket < 0 ? (int)ticket
                      : (int)cmk_call(CMK2_BLOCK_WAIT, (uint64_t)ticket, 0, 0);
}

enum
{
    CMK2_EINVAL = -1,
    CMK2_EAGAIN = -2,
    CMK2_ECLOSED = -3,
    CMK2_EPERM = -4
};

static inline int64_t cmk2_thread_create(void (*entry)(uintptr_t),
                                         void *stack_end, uintptr_t argument)
{
    return cmk_call(CMK2_THREAD_CREATE, (uintptr_t)entry, (uintptr_t)stack_end,
                    argument);
}

static inline int64_t cmk2_thread_join(uint32_t handle)
{
    return cmk_call(CMK2_THREAD_JOIN, handle, 0, 0);
}

static inline void cmk2_thread_exit(unsigned status)
{
    cmk_call(CMK2_THREAD_EXIT, status, 0, 0);
    for (;;)
    {
    }
}

static inline int64_t cmk2_fork(void)
{
    int64_t r;
    unsigned tries = 0;
    do
    {
        r = cmk_call(CMK2_FORK_SINGLE, 0, 0, 0);
        if (r == CMK2_EAGAIN)
            cmk_call(CMK_YIELD, 0, 0, 0);
    } while (r == CMK2_EAGAIN && ++tries < 10000);
    return r;
}
#endif
