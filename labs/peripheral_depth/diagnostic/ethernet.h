#ifndef CM_DIAGNOSTIC_ETHERNET_H
#define CM_DIAGNOSTIC_ETHERNET_H
#include "diagnostic.h"
#include "ethernet_depth.h"
/* IEEE802.3 length frames, fixed local addresses; this adapter is not IPv4/UDP.
 */
int dg_ethernet_build(const DGMessage *, uint8_t *, size_t, size_t *);
int dg_ethernet_accept(DGSession *, const uint8_t *, size_t, DGMessage *);
int dg_descriptors(DGSession *, EDRx *, const EDDescriptor *, size_t,
                   DGMessage *);
int dg_publish(EDTx *, unsigned, uint32_t, const DGMessage *, void (*)(void *),
               void *);
#endif
