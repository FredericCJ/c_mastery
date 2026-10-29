#ifndef SF_KERNEL_ABI_H
#define SF_KERNEL_ABI_H
#include "kernel_abi.h"

/* Additive teaching ABI. Existing CMK1/CMK2/SDK values and evidence are retained. */
enum
{
    SF_INFO = 0x400,
    SF_TYPED_THREAD,
    SF_TYPED_JOIN,
    SF_TYPED_SPAWN,
    SF_TYPED_WAIT,
    SF_PROCESS_KILL,
    SF_PROCESS_STATE,
    SF_FAULT_CONTROL,
    SF_MAPPING,
    SF_FAULT_CAUSES,
    SF_CLONE_SATURATION,
    SF_TABLE_LEDGER,
    SF_CHANNEL,
    SF_GRANT,
    SF_SEND,
    SF_RECEIVE,
    SF_CHANNEL_CLOSE,
    SF_CHANNEL_DESTROY,
    SF_BLOCKED,
    SF_SHM_CREATE,
    SF_SHM_GRANT,
    SF_SHM_MAP,
    SF_SHM_UNMAP,
    SF_SHM_CLOSE,
    SF_SEEK_EXTENDED,
    SF_BIND,
    SF_BOOTSTRAP,
    SF_IO_CONTROL,
    SF_IO_SNAPSHOT,
    SF_OPEN,
    SF_ARCHIVE_PROBE,
    SF_FAULT_BARRIER,
    SF_LOAD_FULL,
    SF_END
};

enum
{
    SF_SEND_RIGHT = 1,
    SF_RECEIVE_RIGHT = 2,
    SF_CLOSE_RIGHT = 4,
    SF_READ_RIGHT = 1,
    SF_WRITE_RIGHT = 2,
    SF_EXEC_RIGHT = 4,
    SF_INVALID = -40,
    SF_DENIED = -41,
    SF_FULL = -42,
    SF_CLOSED = -43,
    SF_COPY = -44,
    SF_STALE = -45,
    SF_RETRY = 512,
    SF_UNIMPLEMENTED = -99
};

#define SF_PROCESS_TAG UINT64_C(0x100000000)
#define SF_THREAD_TAG UINT64_C(0x200000000)
#define SF_CHANNEL_TAG UINT64_C(0x300000000)
#define SF_SHARED_TAG UINT64_C(0x400000000)
#define SF_PAYLOAD_MAX 512u

struct sf_span
{
    uint64_t pointer, length;
};

struct sf_receive
{
    uint64_t pointer, capacity, sender;
};

struct sf_grant
{
    uint64_t recipient;
    uint32_t rights, reserved;
};

struct sf_map
{
    uint64_t address, length, offset;
    uint32_t rights, reserved;
};

struct sf_bootstrap
{
    uint64_t request, reply, session;
    uint32_t role, reserved;
};

struct sf_io_snapshot
{
    uint64_t owner;
    uint32_t request_state, ticket, pending, alive, quiescent, reaped, waited,
        device_completed, next_slot;
};

struct sf_ledger
{
    uint32_t processes, threads, ready, blocked, dead, frames, references,
        pending_io;
    uint32_t channel_messages, channel_waiters, shared_open,
        shared_mapping_refs;
};

/* INFO selectors:0 typed current process;1 typed current thread;2 cpu mask;
 * 3 whether the matching parent is waiting for this process;4 diagnostic
 * live process-thread count in high32 and allocated TCB extent in low32.
 * MAPPING returns the PTE of this process only (diagnostic teaching profile).
 * PROCESS_STATE is restricted to a matching child incarnation and reports
 * alive|quiescent<<1|reaped<<2|waited<<3|io_pending<<4|remote-current<<5.
 * PROCESS_KILL c=1 requires a currently executing child under the same lock;
 * otherwise it returns RETRY without mutation. c=0 is ordinary termination.
 * FAULT_CONTROL uses allocation countdown: UINT64_MAX disables,0 fails next,
 * positive n permits n successful fault allocations; b seeds new frames with
 * synthetic0xa5 before the materializer zeros/copies them. It is test-only.
 * FAULT_CAUSES and CLONE_SATURATION are bounded invariant probes, not authority
 * to modify arbitrary mappings. Ordinary workloads never need these controls. */
#endif
