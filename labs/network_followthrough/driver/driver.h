#ifndef NB_NETWORK_DRIVER_H
#define NB_NETWORK_DRIVER_H
#include "arp.h"
#include "ring.h"
#include "tcp.h"

enum
{
    NB_OK,
    NB_WAIT,
    NB_INVALID,
    NB_BUSY,
    NB_STOPPED,
    NB_FAULT,
    NB_STUB
};

enum
{
    NB_RUNNING,
    NB_STOPPING,
    NB_QUIESCENT,
    NB_FAILED
};

struct nb_copy
{
    unsigned used, tcp;
    struct en_ticket ring;
    struct nc_ticket transport;
};

struct nb_net
{
    struct en_ring *ring;
    struct na_config config;
    struct na_arp arp;
    struct ni_reassembly ip;
    struct nc_stack tcp;
    struct nb_copy copies[2];
    struct nc_packet offered;
    struct net_packet wire;
    struct ni_packet scratch, expired[2], control[2];
    unsigned pending_tcp, control_head, control_count, phase;
    unsigned ring_epoch, rx_frames, rx_released, rx_rejected, paused_tcp,
        control_dropped, tx_errors;
    uint64_t now, stop_started;
};

/* Fresh storage and a live already-started EnRing; no other TX producer.
 * All calls serialized; all borrowed ring/memory/provider storage stays live.
 */
int nb_init(struct nb_net *, struct en_ring *, struct na_config, uint32_t epoch,
            uint32_t isn, unsigned rx_capacity, uint64_t now);
/* One bounded step: poll PHY/status,reap at most2TX,consume at most4RX,
 * enqueue at most one copied frame. No application callback runs within it. */
int nb_step(struct nb_net *, uint64_t now);
int nb_offer(struct nb_net *, const void *, size_t);
/* NB status and HTTP parser event are separate outputs. BUSY consumes nothing.
 */
int nb_http_pump(struct nb_net *, struct http_parser *, size_t, size_t *,
                 int *event);
int nb_close(struct nb_net *);
int nb_stop(struct nb_net *, uint64_t now);
int nb_stop_step(struct nb_net *, uint64_t now);
/* Requires positive stop completion and a restarted ring with a new epoch. */
int nb_restart(struct nb_net *, uint32_t epoch, uint32_t isn,
               unsigned rx_capacity, uint64_t now);
#endif
