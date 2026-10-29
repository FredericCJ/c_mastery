#ifndef EH_HANDOFF_H
#define EH_HANDOFF_H
#include "ethernet_depth.h"

enum
{
    EH_OK,
    EH_INVALID,
    EH_NO_ROUTE,
    EH_NEIGHBOR,
    EH_FULL,
    EH_STUB = 99
};

struct eh_route
{
    uint32_t local, mask, gateway;
};

struct eh_neighbor
{
    uint32_t address;
    unsigned char mac[6];
};

struct eh_clock
{
    uint32_t anchor, ppm;
    uint64_t lower, upper;
};

int eh_next(const struct eh_route *, uint32_t, uint32_t *);
int eh_frame(const struct eh_route *, const unsigned char *,
             const struct eh_neighbor *, const unsigned char *, size_t,
             unsigned char *, size_t, size_t *);
int eh_correlate(const struct eh_clock *, const uint32_t *, size_t, uint64_t *,
                 uint64_t *);
#endif
