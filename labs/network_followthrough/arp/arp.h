#ifndef CMASTERY_NEIGHBOR_ARP_H
#define CMASTERY_NEIGHBOR_ARP_H
#include "com_net.h"

enum
{
    NA_OK,
    NA_INVALID,
    NA_BUSY,
    NA_STALE,
    NA_WAIT,
    NA_EXPIRED,
    NA_RANGE,
    NA_UNIMPLEMENTED = 99
};

enum
{
    NA_REQUEST = 1,
    NA_DATA = 2,
    NA_REPLY = 3
};

struct na_config
{
    unsigned char local_mac[6], local_ip[4], peer_ip[4];
};

struct na_ticket
{
    uint32_t epoch, serial;
    unsigned kind;
};

struct na_arp
{
    struct na_config config;
    unsigned char peer_mac[6], reply_mac[6], datagram[1500];
    uint64_t last, expires, started, deadline;
    uint32_t epoch, serial;
    uint16_t length;
    unsigned known, pending, reply_pending, prepared, prepared_slot;
    unsigned slots_done, requests, resolved;
};

/* Fresh/quiescent storage; epoch is externally unique while old tickets exist.
 */
int na_init(struct na_arp *, struct na_config, uint32_t epoch);
/* Opaque caller-validated IPv4 datagram for config.peer_ip; copy on success.
 * No alias with state. Second pending datagram is BUSY, caller retains it. */
int na_queue(struct na_arp *, const void *, size_t, uint64_t now_ms);
/* Complete frame span excluding FCS; lower layer supplies actual received
 * count. Validation precedes association/reply publication. Calls externally
 * serialized. During a prepared lower-send transaction, receive returns BUSY
 * unchanged. */
int na_receive(struct na_arp *, const void *, size_t, uint64_t now_ms);
/* Owns a copied output frame. One prepare/submit transaction at a time.
 * Caller performs a copy-admission lower send; accepted means lower owns a
 * copy, NOT DMA completion or remote delivery. No API call may interleave lower
 * send and submit. Refusal retains queued bytes; resolve the ticket exactly
 * once. */
int na_prepare(struct na_arp *, uint64_t now_ms, struct net_packet *,
               struct na_ticket *);
int na_submit(struct na_arp *, struct na_ticket, unsigned accepted);
#endif
