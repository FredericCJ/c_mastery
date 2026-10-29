#ifndef REQUEST_ADAPTER_ABI_H
#define REQUEST_ADAPTER_ABI_H
#include "cmk2_abi.h"
#include "request.h"

enum
{
    RQA_CANCEL = 0x380,
    RQA_HOLD,
    RQA_TIMEOUT,
    RQA_RESET_BEGIN,
    RQA_RESET_ACK,
    RQA_QUERY,
    RQA_USED,
    RQA_PENDING,
    RQA_TERMINALS,
    RQA_RESET_STATUS,
    RQA_INDICES,
    RQA_DEAD_PENDING,
    RQA_WAITERS
};
#endif
