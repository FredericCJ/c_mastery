#ifndef SD_KERNEL_ABI_H
#define SD_KERNEL_ABI_H
#include "cmk2_abi.h"
#include "cmk_contract.h"

/* Additive teaching ABI; no change to retained CMK v1/v2 numbers. */
enum
{
    SDK_LOAD_PROBE = 0x300,
    SDK_CLONE_PRIVATE,
    SDK_OPEN,
    SDK_READ,
    SDK_SEEK,
    SDK_DUP,
    SDK_CLOSE,
    SDK_CHANNEL,
    SDK_ATTENUATE,
    SDK_SEND,
    SDK_RECEIVE,
    SDK_CHANNEL_CLOSE,
    SDK_CANCEL,
    SDK_BLOCKED,
    SDK_RESERVED_30E,
    SDK_CUT,
    SDK_CRASH,
    SDK_CHANNEL_DESTROY,
    SDK_RECEIVE_PARTIAL,
    SDK_INCARNATION,
    SDK_DEFERRED_WAKE,
    SDK_MUTATE,
    SDK_OPEN_SNAPSHOT,
    SDK_OPEN_LIVE,
    SDK_SHARE_IMMUTABLE,
    SDK_CLONE_IMMUTABLE
};

/* 0x30e is reserved and returns SDK_INVALID; it provides no drain operation.
 * Channel draining uses receive-until-closed, then explicit destroy. */
enum
{
    SDK_SEND_RIGHT = 1,
    SDK_RECEIVE_RIGHT = 2,
    SDK_CLOSE_RIGHT = 4,
    SDK_PARTIAL_END = 65536,
    SDK_RETRY = 256,
    SDK_INVALID = -10,
    SDK_DENIED = -11,
    SDK_FULL = -12,
    SDK_COPY = -13,
    SDK_CLOSED = -14,
    SDK_CANCELLED = -15,
    SDK_STALE = -16
};

struct sdk_span
{
    uint64_t pointer, length;
};

struct sdk_seek
{
    int64_t offset;
    uint32_t origin, reserved;
};
#endif
