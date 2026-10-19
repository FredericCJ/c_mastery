#ifndef EH_HANDOFF_H
#define EH_HANDOFF_H
#include "ethernet_depth.h"
enum { EH_OK, EH_INVALID, EH_NO_ROUTE, EH_NEIGHBOR, EH_FULL, EH_STUB = 99 };
typedef struct {
  uint32_t local, mask, gateway;
} EhRoute;
typedef struct {
  uint32_t address;
  unsigned char mac[6];
} EhNeighbor;
typedef struct {
  uint32_t anchor, ppm;
  uint64_t lower, upper;
} EhClock;
int eh_next(const EhRoute *, uint32_t, uint32_t *);
int eh_frame(const EhRoute *, const unsigned char *, const EhNeighbor *,
             const unsigned char *, size_t, unsigned char *, size_t, size_t *);
int eh_correlate(const EhClock *, const uint32_t *, size_t, uint64_t *,
                 uint64_t *);
#endif
