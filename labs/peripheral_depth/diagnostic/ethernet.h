#ifndef CM_DIAGNOSTIC_ETHERNET_H
#define CM_DIAGNOSTIC_ETHERNET_H
#include "diagnostic.h"
#include "ethernet_depth.h"
/* IEEE802.3 length frames, fixed local addresses; this adapter is not IPv4/UDP.
 */
int dg_ethernet_build(const struct dg_message *, uint8_t *, size_t, size_t *);
int dg_ethernet_accept(struct dg_session *, const uint8_t *, size_t, struct dg_message *);
int dg_descriptors(struct dg_session *, struct ed_rx *, const struct ed_descriptor *, size_t,
                   struct dg_message *);
int dg_publish(struct ed_tx *, unsigned, uint32_t, const struct dg_message *, void (*)(void *),
               void *);
#endif
