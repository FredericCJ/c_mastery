#ifndef KF_GUEST_ABI_H
#define KF_GUEST_ABI_H
#include "cmk_abi.h"
#ifndef KF_CASE
#define KF_CASE 0
#endif
#ifndef KF_SENTINEL_VA
#define KF_SENTINEL_VA UINT64_C(0x20000000)
#endif
#ifndef KF_QUANTUM
#define KF_QUANTUM 11932
#endif
#define KF_STATS 64
#define KF_POLL 65
#define KF_ALLOC_SET 67
#ifndef KF_POLL_ITER
#define KF_POLL_ITER 2000000
#endif
#ifndef KF_COMPLETION_DELAY
#define KF_COMPLETION_DELAY 0
#endif
#ifndef KF_FAULT_SLOT
#define KF_FAULT_SLOT 3
#endif
struct kf_stats
{
    uint32_t version, size;
    uint64_t pid, free_frames, ticks, preemptions, device_submissions;
};

enum
{
    KF_NORMAL = 0,
    KF_USER_DIVIDE = 1,
    KF_KERNEL_DIVIDE = 2,
    KF_NESTED = 3,
    KF_DOUBLE_FAULT = 4,
    KF_BUDGET = 5,
    KF_FIXED = 6,
    KF_FLAWED = 7,
    KF_UPPER = 8,
    KF_NX = 9,
    KF_GUARD = 10,
    KF_SKIP_GUARD = 11,
    KF_PROBE = 12,
    KF_BAD_REGISTER = 13,
    KF_NO_EOI = 14,
    KF_LATENCY = 15,
    KF_METADATA = 16
};
#endif
