#include "driver.h"
#include <string.h>
#ifndef NB_MUTANT
#define NB_MUTANT 0
#endif
static uint32_t ip4(const unsigned char *p) {
  return (uint32_t)p[0] << 24 | (uint32_t)p[1] << 16 | (uint32_t)p[2] << 8 |
         p[3];
}
static unsigned u16(const unsigned char *p) {
  return (unsigned)p[0] * 256 + p[1];
}
static int valid_time(const struct nb_net *s, uint64_t now) {
  return s && s->ring && now >= s->now && now <= UINT64_MAX - 240000;
}
int nb_init(struct nb_net *s, struct en_ring *ring, struct na_config c, uint32_t epoch, uint32_t isn,
            unsigned cap, uint64_t now) {
  if (!s || !ring || ring->phase != EN_RUNNING ||
      memcmp(c.local_mac, ring->mac, 6) || now > UINT64_MAX - 240000 ||
      !epoch || !cap || cap > 512)
    return NB_INVALID;
  struct ni_config ips = {ip4(c.local_ip), ip4(c.peer_ip)};
  /* Validate before overwriting the caller's fresh object. */
  unsigned nz = 0;
  for (unsigned i = 0; i < 6; i++)
    nz |= c.local_mac[i];
  if (!nz || (c.local_mac[0] & 1) || !ips.local || !ips.peer ||
      ips.local == ips.peer)
    return NB_INVALID;
  memset(s, 0, sizeof *s);
  s->ring = ring;
  s->config = c;
  s->ring_epoch = ring->epoch;
  s->now = now;
  if (na_init(&s->arp, c, epoch) != NA_OK ||
      ni_init(&s->ip, ips, 2, NI_STRICT, now, 60000) != NI_OK ||
      nc_init(&s->tcp, ips, epoch, isn, cap, 0) != NC_OK)
    return NB_INVALID;
  return nc_tick(&s->tcp, now) == NC_OK ? NB_OK : NB_INVALID;
}
static int stop_result(struct nb_net *s, int e) {
  if (e == ED_BUSY) {
    s->phase = NB_STOPPING;
    return NB_WAIT;
  }
  s->phase = NB_FAILED;
  return NB_FAULT;
}
int nb_stop(struct nb_net *s, uint64_t now) {
  if (!valid_time(s, now))
    return NB_INVALID;
  if (s->phase == NB_QUIESCENT)
    return NB_STOPPED;
  if (s->phase == NB_FAILED)
    return NB_FAULT;
  if (s->phase == NB_STOPPING)
    return NB_WAIT;
  s->now = now;
  if (nc_abort(&s->tcp, now) != NC_OK) {
    s->phase = NB_FAILED;
    return NB_FAULT;
  }
  s->pending_tcp = 0;
  if (NB_MUTANT == 3)
    for (unsigned i = 0; i < 2; i++)
      if (s->copies[i].used && s->copies[i].tcp)
        (void)nc_dma_done(&s->tcp, s->copies[i].transport);
  s->stop_started = now;
  return stop_result(s, en_stop_begin(s->ring, 0, 16, 100));
}
int nb_stop_step(struct nb_net *s, uint64_t now) {
  if (!valid_time(s, now))
    return NB_INVALID;
  if (s->phase == NB_QUIESCENT)
    return NB_STOPPED;
  if (s->phase != NB_STOPPING)
    return NB_FAULT;
  s->now = now;
  uint64_t elapsed = now - s->stop_started;
  /* Map one finite stop window to a fresh local clock; never truncate uptime.
   */
  int e = en_stop_step(s->ring, (uint32_t)(elapsed > 100 ? 100 : elapsed));
  if (e == ED_BUSY)
    return NB_WAIT;
  if (e != ED_OK || s->ring->phase != EN_BOUND) {
    s->phase = NB_FAILED;
    return NB_FAULT;
  }
  for (unsigned i = 0; i < 2; i++)
    if (s->copies[i].used) {
      if (s->copies[i].tcp &&
          nc_dma_done(&s->tcp, s->copies[i].transport) != NC_OK) {
        s->phase = NB_FAILED;
        return NB_FAULT;
      }
      s->copies[i].used = 0;
    }
  s->phase = NB_QUIESCENT;
  return NB_STOPPED;
}
int nb_restart(struct nb_net *s, uint32_t epoch, uint32_t isn, unsigned cap,
               uint64_t now) {
  if (!valid_time(s, now))
    return NB_INVALID;
  if (s->phase != NB_QUIESCENT || s->tcp.dma || s->ring->phase != EN_RUNNING ||
      s->ring->epoch == s->ring_epoch || epoch <= s->tcp.epoch)
    return NB_BUSY;
  return nb_init(s, s->ring, s->config, epoch, isn, cap, now);
}
int nb_offer(struct nb_net *s, const void *p, size_t n) {
  if (!s || s->phase != NB_RUNNING || s->pending_tcp)
    return NB_BUSY;
  return nc_offer(&s->tcp, p, n) == NC_OK ? NB_OK : NB_BUSY;
}
int nb_http_pump(struct nb_net *s, struct http_parser *p, size_t budget, size_t *used,
                 int *event) {
  if (!s || !p || !used || !event || !budget)
    return NB_INVALID;
  if (s->phase != NB_RUNNING || s->pending_tcp)
    return NB_BUSY;
  *event = nc_http_pump(&s->tcp, p, budget, used);
  return NB_OK;
}
int nb_close(struct nb_net *s) {
  if (!s || s->phase != NB_RUNNING || s->pending_tcp)
    return NB_BUSY;
  return nc_close(&s->tcp) == NC_OK ? NB_OK : NB_BUSY;
}
static void retain_control(struct nb_net *s, const struct ni_packet *p) {
  if (s->control_count == 2) {
    s->control_dropped++;
    return;
  }
  s->control[(s->control_head + s->control_count) % 2] = *p;
  s->control_count++;
}
static int expire(struct nb_net *s, uint64_t now) {
  unsigned count = 0;
  if (ni_expire(&s->ip, now, s->expired, &count) != NI_OK)
    return NB_INVALID;
  for (unsigned i = 0; i < count; i++)
    retain_control(s, &s->expired[i]);
  return NB_OK;
}
static int ip_receive(struct nb_net *s, const unsigned char *p, size_t n,
                      uint64_t now) {
  struct ni_view view;
  if (ni_parse(&s->ip.config, p, n, 1, &view) != NI_OK)
    return NB_INVALID;
  if (view.more || view.offset) {
    unsigned slot;
    int e = ni_receive(&s->ip, p, n, now, &slot);
    if (e == NI_WAIT || e == NI_BUSY || e == NI_CONFLICT)
      return NB_WAIT;
    if (e != NI_OK || ni_take(&s->ip, slot, &s->scratch) != NI_OK)
      return NB_INVALID;
    p = s->scratch.bytes;
    n = s->scratch.length;
    if (ni_parse(&s->ip.config, p, n, 0, &view) != NI_OK)
      return NB_INVALID;
  }
  if (view.protocol == 6) {
    if (s->pending_tcp && NB_MUTANT != 7) {
      s->paused_tcp++;
      return NB_WAIT;
    }
    int e = nc_receive(&s->tcp, p, n, now);
    return e == NC_OK || e == NC_WAIT ? NB_OK : NB_INVALID;
  }
  /* Echo output must not alias a reassembled input held in scratch. */
  struct ni_packet *out = &s->expired[0];
  if (ni_echo(&s->ip.config, p, n, out) != NI_OK)
    return NB_INVALID;
  retain_control(s, out);
  return NB_OK;
}
int nb_step(struct nb_net *s, uint64_t now) {
  if (!valid_time(s, now))
    return NB_INVALID;
  if (s->phase != NB_RUNNING)
    return s->phase == NB_QUIESCENT ? NB_STOPPED
           : s->phase == NB_FAILED  ? NB_FAULT
                                    : NB_WAIT;
  s->now = now;
  if (ep_poll(s->ring->phy) != ED_OK)
    return nb_stop(s, now);
  uint32_t status;
  if (en_status(s->ring, &status) != ED_OK)
    return nb_stop(s, now);
  for (unsigned i = 0; i < 2; i++)
    if (s->copies[i].used) {
      int e = en_tx_reap(s->ring, s->copies[i].ring, &status);
      if (e == ED_BUSY)
        continue;
      if (e != ED_OK && e != ED_IO)
        return nb_stop(s, now);
      if (e == ED_IO)
        s->tx_errors++;
      if (s->copies[i].tcp) {
        if (nc_dma_done(&s->tcp, s->copies[i].transport) != NC_OK)
          return nb_stop(s, now);
        if (NB_MUTANT == 2)
          s->tcp.flight = 0;
      }
      s->copies[i].used = 0;
    }
  if (NB_MUTANT != 6 && expire(s, now) != NB_OK)
    return nb_stop(s, now);
  for (unsigned i = 0; i < 4; i++) {
    struct en_frame frame;
    int e = en_rx_take(s->ring, &frame);
    if (e == ED_BUSY)
      break;
    if (e == ED_IO) {
      if (s->ring->phase != EN_RUNNING)
        return nb_stop(s, now);
      s->rx_rejected++;
      continue;
    }
    if (e != ED_OK)
      return nb_stop(s, now);
    s->rx_frames++;
    int accepted = NB_INVALID;
    size_t n = frame.length;
    if (NB_MUTANT == 5)
      n -= 4;
    if (n >= 14 && u16(frame.bytes + 12) == 0x806)
      accepted = na_receive(&s->arp, frame.bytes, n, now) == NA_OK ? NB_OK
                                                                   : NB_INVALID;
    else if (n >= 34 && u16(frame.bytes + 12) == 0x800 &&
             !memcmp(frame.bytes, s->config.local_mac, 6) &&
             !(frame.bytes[6] & 1))
      accepted = ip_receive(s, frame.bytes + 14, n - 14, now);
    if (accepted == NB_INVALID)
      s->rx_rejected++;
    if (NB_MUTANT != 4) {
      if (en_rx_release(s->ring, frame.ticket) != ED_OK)
        return nb_stop(s, now);
      s->rx_released++;
    }
  }
  if (NB_MUTANT == 6 && expire(s, now) != NB_OK)
    return nb_stop(s, now);
  if (!s->pending_tcp &&
      nc_tick(&s->tcp, now) == NC_ABORT) { /* Logical retry exhaustion does not
                                              own the DMA engine. */
  }
  if (!s->arp.pending) {
    if (s->control_count) {
      struct ni_packet *p = &s->control[s->control_head];
      if (na_queue(&s->arp, p->bytes, p->length, now) != NA_OK)
        return nb_stop(s, now);
      s->control_head = (s->control_head + 1) % 2;
      s->control_count--;
    } else if (!s->pending_tcp && nc_prepare(&s->tcp, &s->offered) == NC_OK) {
      if (na_queue(&s->arp, s->offered.bytes, s->offered.length, now) != NA_OK)
        return nb_stop(s, now);
      s->pending_tcp = 1;
    }
  }
  struct na_ticket at;
  int e = na_prepare(&s->arp, now, &s->wire, &at);
  if (e == NA_EXPIRED) {
    if (s->pending_tcp) {
      s->pending_tcp = 0;
      if (nc_abort(&s->tcp, now) != NC_OK)
        return nb_stop(s, now);
    }
    return NB_WAIT;
  }
  if (e == NA_WAIT)
    return NB_OK;
  if (e != NA_OK)
    return nb_stop(s, now);
  struct en_ticket et;
  int copied = en_tx_copy(s->ring, s->wire.bytes, s->wire.length, &et);
  if (copied != ED_OK) {
    (void)na_submit(&s->arp, at, 0);
    if (NB_MUTANT == 1 && s->pending_tcp && at.kind == NA_DATA) {
      (void)nc_submit(&s->tcp, s->offered.ticket, 1, now);
      s->pending_tcp = 0;
    }
    return copied == ED_BUSY ? NB_WAIT : nb_stop(s, now);
  }
  /* Commit both ledgers before publication. A later publish failure does not
   * undo the real copy; the stop path retains this descriptor until quiescent.
   */
  if (et.slot >= 2 || s->copies[et.slot].used)
    return nb_stop(s, now);
  struct nb_copy *copy = &s->copies[et.slot];
  copy->used = 1;
  copy->tcp = s->pending_tcp && at.kind == NA_DATA;
  copy->ring = et;
  copy->transport = s->offered.ticket;
  if (na_submit(&s->arp, at, 1) != NA_OK)
    return nb_stop(s, now);
  if (copy->tcp) {
    if (nc_submit(&s->tcp, copy->transport, 1, now) != NC_OK)
      return nb_stop(s, now);
    s->pending_tcp = 0;
  }
  if (en_tx_publish(s->ring, et) != ED_OK)
    return nb_stop(s, now);
  return NB_OK;
}
