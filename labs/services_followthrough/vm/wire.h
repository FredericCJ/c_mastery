#ifndef SF_VM_WIRE_H
#define SF_VM_WIRE_H
#include "service.h"

struct sfv_config
{
    uint64_t clients[2], reply[2];
};

struct sfv_wire
{
    uint32_t operation, capacity;
    struct sfv_request request;
};

struct sfv_response
{
    uint64_t session, id;
    uint32_t status, length;
    uint8_t bytes[SFV_RESULT];
};
#endif
