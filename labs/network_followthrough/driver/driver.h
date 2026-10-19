#ifndef NB_NETWORK_DRIVER_H
#define NB_NETWORK_DRIVER_H
#include "arp.h"
#include "ring.h"
#include "tcp.h"
enum { NB_OK, NB_WAIT, NB_INVALID, NB_BUSY, NB_STOPPED, NB_FAULT, NB_STUB };
enum { NB_RUNNING, NB_STOPPING, NB_QUIESCENT, NB_FAILED };
typedef struct {
  unsigned used, tcp;
  EnTicket ring;
  NcTicket transport;
} NbCopy;
typedef struct {
  EnRing *ring;
  NaConfig config;
  NaArp arp;
  NiReassembly ip;
  NcStack tcp;
  NbCopy copies[2];
  NcPacket offered;
  NetPacket wire;
  NiPacket scratch, expired[2], control[2];
  unsigned pending_tcp, control_head, control_count, phase;
  unsigned ring_epoch, rx_frames, rx_released, rx_rejected, paused_tcp,
      control_dropped, tx_errors;
  uint64_t now, stop_started;
} NbNet;
/* Fresh storage and a live already-started EnRing; no other TX producer.
 * All calls serialized; all borrowed ring/memory/provider storage stays live.
 */
int nb_init(NbNet *, EnRing *, NaConfig, uint32_t epoch, uint32_t isn,
            unsigned rx_capacity, uint64_t now);
/* One bounded step: poll PHY/status,reap at most2TX,consume at most4RX,
 * enqueue at most one copied frame. No application callback runs within it. */
int nb_step(NbNet *, uint64_t now);
int nb_offer(NbNet *, const void *, size_t);
/* NB status and HTTP parser event are separate outputs. BUSY consumes nothing.
 */
int nb_http_pump(NbNet *, HttpParser *, size_t, size_t *, int *event);
int nb_close(NbNet *);
int nb_stop(NbNet *, uint64_t now);
int nb_stop_step(NbNet *, uint64_t now);
/* Requires positive stop completion and a restarted ring with a new epoch. */
int nb_restart(NbNet *, uint32_t epoch, uint32_t isn, unsigned rx_capacity,
               uint64_t now);
#endif
