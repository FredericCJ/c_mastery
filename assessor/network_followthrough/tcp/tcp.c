#include "tcp.h"
#include <string.h>
#ifndef NC_MUTANT
#define NC_MUTANT 0
#endif
static unsigned u16(const unsigned char *p) {
  return (unsigned)p[0] * 256 + p[1];
}
static uint32_t u32(const unsigned char *p) {
  return (uint32_t)u16(p) << 16 | u16(p + 2);
}
static void w16(unsigned char *p, unsigned x) {
  p[0] = (unsigned char)(x >> 8);
  p[1] = (unsigned char)x;
}
static void w32(unsigned char *p, uint32_t x) {
  w16(p, x >> 16);
  w16(p + 2, x);
}
static int before(uint32_t a, uint32_t b) {
  return a != b && b - a < UINT32_C(0x80000000);
}
static unsigned sum(const void *p, size_t n) {
  struct s_checksum s = {0, 0, 0};
  (void)s_checksum_add(&s, p, n);
  uint16_t value = 0;
  (void)s_checksum_finish(&s, &value);
  return value;
}
static unsigned tcp_sum(const unsigned char *ip, const unsigned char *p,
                        size_t n) {
  unsigned char tail[4] = {0, 6, (unsigned char)(n >> 8), (unsigned char)n};
  struct s_checksum s = {0, 0, 0};
  (void)s_checksum_add(&s, ip + 12, 8);
  (void)s_checksum_add(&s, tail, 4);
  (void)s_checksum_add(&s, p, n);
  uint16_t value = 0;
  (void)s_checksum_finish(&s, &value);
  return value;
}
static int time_ok(const struct nc_stack *s, uint64_t now) {
  return now >= s->now && now <= UINT64_MAX - 240000;
}
static uint32_t twice(uint32_t r) { return r > 30000 ? 60000 : r * 2; }
int nc_rtt_init(struct nc_rtt *r, unsigned g, unsigned shift) {
  if (!r || !g || g > 1000 || (shift != 0 && shift != 8))
    return NC_INVALID;
  *r = (struct nc_rtt){0, 0, 1000, 0, g, shift};
  return NC_OK;
}
int nc_rtt_sample(struct nc_rtt *r, unsigned ms, unsigned retransmitted) {
  if (!r || ms > 60000 || retransmitted > 1)
    return NC_INVALID;
  if (retransmitted && NC_MUTANT != 1)
    return NC_WAIT;
  if (!ms)
    ms = 1;
  uint32_t value = (uint32_t)ms << r->shift;
  if (!r->samples) {
    r->srtt = value;
    r->variance = value / 2;
  } else {
    uint32_t old = r->srtt;
    r->srtt = (7 * r->srtt + value) / 8;
    if (NC_MUTANT == 2)
      old = r->srtt;
    uint32_t d = old > value ? old - value : value - old;
    r->variance = (3 * r->variance + d) / 4;
  }
  uint32_t variation = 4 * r->variance, g = r->granularity << r->shift;
  uint32_t raw = r->srtt + (variation > g ? variation : g);
  uint32_t milliseconds = (raw + ((1u << r->shift) - 1)) >> r->shift;
  r->rto = milliseconds < 1000    ? 1000
           : milliseconds > 60000 ? 60000
                                  : milliseconds;
  if (r->samples != UINT32_MAX)
    r->samples++;
  return NC_OK;
}
int nc_init(struct nc_stack *s, struct ni_config c, uint32_t epoch, uint32_t isn, unsigned cap,
            unsigned shift) {
  if (!s || !epoch || !cap || cap > 512 || (shift != 0 && shift != 8) ||
      c.local == c.peer)
    return NC_INVALID;
  memset(s, 0, sizeof *s);
  s->config = c;
  s->epoch = epoch;
  s->isn = isn;
  s->snd_una = s->snd_nxt = isn;
  s->mss = 256;
  s->rto = s->probe_rto = 1000;
  (void)nw_rx_init(&s->rx, cap);
  return nc_rtt_init(&s->rtt, 1, shift);
}
static void abort_connection(struct nc_stack *s) {
  s->state = NC_CLOSED;
  s->flight = s->pending_valid = s->ack_needed = 0;
  s->due = s->probe_due = 0;
  s->tx_at = s->tx_length = 0;
  s->rx.head = s->rx.count = 0;
  /* Lower DMA is a distinct owner and must still be reaped. */
}
int nc_abort(struct nc_stack *s, uint64_t now) {
  if (!s || !s->epoch || !time_ok(s, now)) return NC_RANGE;
  s->now = now;
  abort_connection(s);
  return NC_OK;
}
unsigned nc_usable(const struct nc_stack *s) {
  uint32_t flight = s->snd_nxt - s->snd_una;
  return flight >= s->peer_window ? 0 : s->peer_window - flight;
}
int nc_tick(struct nc_stack *s, uint64_t now) {
  if (!s || !time_ok(s, now))
    return NC_RANGE;
  s->now = now;
  if (s->state == NC_TIME_WAIT && now >= s->timewait_until) {
    abort_connection(s);
    return NC_OK;
  }
  if (s->flight && now >= s->due && s->retries == 3) {
    abort_connection(s);
    return NC_ABORT;
  }
  return NC_OK;
}
static void packet(struct nc_stack *s, unsigned flags, uint32_t seq, const void *data,
                   size_t n) {
  struct nc_packet *o = &s->pending;
  unsigned h = (flags & 2) ? 24 : 20;
  memset(o, 0, sizeof *o);
  unsigned char *ip = o->bytes, *p = ip + 20;
  ip[0] = 0x45;
  w16(ip + 2, (unsigned)(20 + h + n));
  w16(ip + 6, 0x4000);
  ip[8] = 64;
  ip[9] = 6;
  w32(ip + 12, s->config.local);
  w32(ip + 16, s->config.peer);
  w16(ip + 10, sum(ip, 20));
  w16(p, 8080);
  w16(p + 2, s->port);
  w32(p + 4, seq);
  w32(p + 8, s->rcv_nxt);
  p[12] = (unsigned char)(h << 2);
  p[13] = (unsigned char)flags;
  w16(p + 14, NC_MUTANT == 3 ? 512 : nw_rx_window(&s->rx));
  if (h == 24) {
    p[20] = 2;
    p[21] = 4;
    w16(p + 22, 256);
  }
  if (n)
    memcpy(p + h, data, n);
  w16(p + 16, tcp_sum(ip, p, h + n));
  o->length = 20 + h + n;
}
int nc_offer(struct nc_stack *s, const void *v, size_t n) {
  if (!s || !v || !n || n > sizeof s->tx)
    return NC_INVALID;
  if ((s->state != NC_ESTABLISHED && s->state != NC_CLOSE_WAIT) ||
      s->close_requested || s->flight || s->tx_at != s->tx_length ||
      s->pending_valid)
    return NC_BUSY;
  memcpy(s->tx, v, n);
  s->tx_length = n;
  s->tx_at = 0;
  if (!s->peer_window) {
    s->probe_due = s->now + s->rto;
    s->probe_rto = s->rto;
  }
  return NC_OK;
}
int nc_close(struct nc_stack *s) {
  if (!s || (s->state != NC_ESTABLISHED && s->state != NC_CLOSE_WAIT))
    return NC_INVALID;
  s->close_requested = 1;
  return NC_OK;
}
int nc_prepare(struct nc_stack *s, struct nc_packet *out) {
  if (!s || !out)
    return NC_INVALID;
  if (s->state == NC_CLOSED || s->state == NC_LISTEN)
    return NC_WAIT;
  if (s->dma)
    return NC_BUSY;
  if (s->pending_valid) {
    *out = s->pending;
    return NC_OK;
  }
  if (s->serial == UINT32_MAX || s->exhausted) {
    s->exhausted = 1;
    return NC_RANGE;
  }
  s->pending_span = s->pending_data = 0;
  if (s->flight && s->now >= s->due) {
    s->pending = s->retained;
    s->pending_kind = NC_RETRY;
    /* Retransmission keeps the original bytes/ACK/window and sequence. */
  } else if (s->state == NC_SYN_RECEIVED && !s->flight) {
    packet(s, 0x12, s->snd_nxt, 0, 0);
    s->pending_kind = NC_NEW;
    s->pending_span = 1;
  } else if (!s->flight &&
             (s->state == NC_ESTABLISHED || s->state == NC_CLOSE_WAIT) &&
             s->tx_at < s->tx_length && nc_usable(s)) {
    size_t n = s->tx_length - s->tx_at;
    if (n > s->mss)
      n = s->mss;
    if (n > nc_usable(s))
      n = nc_usable(s);
    packet(s, 0x18, s->snd_nxt, s->tx + s->tx_at, n);
    s->pending_kind = NC_NEW;
    s->pending_span = s->pending_data = (unsigned)n;
  } else if (!s->flight &&
             (s->state == NC_ESTABLISHED || s->state == NC_CLOSE_WAIT) &&
             s->close_requested && s->tx_at == s->tx_length && s->peer_window) {
    packet(s, 0x11, s->snd_nxt, 0, 0);
    s->pending_kind = NC_NEW;
    s->pending_span = 1;
  } else if (s->ack_needed) {
    packet(s, 0x10, s->snd_nxt, 0, 0);
    s->pending_kind = NC_ACK;
  } else if (!s->flight && s->tx_at < s->tx_length && !s->peer_window &&
             s->probe_due && s->now >= s->probe_due) {
    packet(s, 0x10, s->snd_nxt - 1, 0, 0);
    s->pending_kind = NC_PROBE;
  } else
    return NC_WAIT;
  s->pending.ticket = (struct nc_ticket){s->epoch, ++s->serial};
  s->pending_valid = 1;
  if (NC_MUTANT == 4 && s->pending_kind == NC_NEW) {
    s->flight = 1;
    s->due = s->now + s->rto;
    s->snd_nxt += s->pending_span;
  }
  *out = s->pending;
  return NC_OK;
}
int nc_submit(struct nc_stack *s, struct nc_ticket t, unsigned copied, uint64_t now) {
  if (!s || copied > 1 || !time_ok(s, now))
    return NC_INVALID;
  if (!s->pending_valid || t.epoch != s->epoch ||
      t.serial != s->pending.ticket.serial)
    return NC_STALE;
  if (!copied) {
    s->now = now;
    return NC_WAIT;
  }
  if (s->dma)
    return NC_BUSY;
  s->now = now;
  s->dma = 1;
  s->dma_serial = t.serial;
  if (s->pending_kind == NC_NEW) {
    s->retained = s->pending;
    s->flight = 1;
    s->sent_at = now;
    s->due = now + s->rto;
    s->retries = s->retransmitted = 0;
    s->snd_nxt += s->pending_span;
    s->tx_at += s->pending_data;
    if (s->pending.bytes[33] & 1)
      s->state = s->peer_fin ? NC_LAST_ACK : NC_FIN_WAIT_1;
  } else if (s->pending_kind == NC_RETRY) {
    s->retries++;
    s->retransmitted = 1;
    s->rto = twice(s->rto);
    s->due = now + s->rto;
    if (s->state == NC_SYN_RECEIVED)
      s->syn_lost = 1;
  } else if (s->pending_kind == NC_PROBE) {
    s->probe_rto = twice(s->probe_rto);
    s->probe_due = now + s->probe_rto;
  }
  s->ack_needed = 0;
  s->pending_valid = 0;
  return NC_OK;
}
int nc_dma_done(struct nc_stack *s, struct nc_ticket t) {
  if (!s || !s->dma || t.epoch != s->epoch || t.serial != s->dma_serial)
    return NC_STALE;
  s->dma = 0;
  if (NC_MUTANT == 5) {
    s->flight = 0;
    s->due = 0;
  }
  return NC_OK;
}
int nc_consume(struct nc_stack *s, size_t n) {
  if (!s || nw_rx_consume(&s->rx, n) != NW_OK)
    return NC_INVALID;
  if (n) {
    s->ack_needed = 1;
    s->pending_valid = 0;
  }
  return NC_OK;
}
int nc_http_pump(struct nc_stack *s, struct http_parser *p, size_t budget, size_t *used) {
  if (!s || !p || !used || !budget)
    return HTTP_ERROR;
  int rc = nw_http_pump(&s->rx, p, budget, used);
  if (*used) {
    s->ack_needed = 1;
    s->pending_valid = 0;
  }
  return rc;
}
int nc_http_eof(struct nc_stack *s, struct http_parser *p) {
  if (!s || !p || !s->peer_fin || s->rx.count)
    return HTTP_NEED;
  return http_eof(p);
}
int nc_receive(struct nc_stack *s, const unsigned char *bytes, size_t length,
               uint64_t now) {
  if (!s || !bytes || !time_ok(s, now))
    return NC_INVALID;
  struct ni_view v;
  if (ni_parse(&s->config, bytes, length, 0, &v) != NI_OK || v.protocol != 6 ||
      v.length < 20)
    return NC_INVALID;
  const unsigned char *p = v.payload;
  unsigned h = (p[12] >> 4) * 4, flags = p[13], mss = 536;
  if (h < 20 || h > v.length || p[12] % 16 || flags & 0xe0 ||
      u16(p + 2) != 8080 || !u16(p) || tcp_sum(v.header, p, v.length))
    return NC_INVALID;
  for (unsigned i = 20; i < h;) {
    unsigned k = p[i++];
    if (!k)
      break;
    if (k == 1)
      continue;
    if (i == h)
      return NC_INVALID;
    unsigned n = p[i++];
    if (n < 2 || n - 2 > h - i)
      return NC_INVALID;
    if (k == 2) {
      if (n != 4 || !(flags & 2) || !u16(p + i))
        return NC_INVALID;
      mss = u16(p + i);
    }
    i += n - 2;
  }
  size_t n = v.length - h;
  uint32_t seq = u32(p + 4), ack = u32(p + 8);
  unsigned wnd = u16(p + 14);
  if (s->state == NC_CLOSED)
    return NC_WAIT;
  if (s->state == NC_LISTEN) {
    if (flags != 2 || n)
      return NC_INVALID;
    s->now = now;
    s->state = NC_SYN_RECEIVED;
    s->port = u16(p);
    s->rcv_nxt = seq + 1;
    s->peer_window = wnd;
    s->mss = mss < 256 ? mss : 256;
    s->wl1 = seq;
    s->wl2 = 0;
    return NC_OK;
  }
  if (u16(p) != s->port)
    return NC_INVALID;
  s->now = now;
  s->pending_valid = 0;
  if (s->state == NC_TIME_WAIT) {
    if ((flags & 1) && !(flags & 6) && seq + (uint32_t)n + 1 == s->rcv_nxt) {
      s->ack_needed = 1;
      s->timewait_until = now + 240000;
      return NC_OK;
    }
    return NC_WAIT;
  }
  unsigned capacity = nw_rx_window(&s->rx);
  uint32_t span = (uint32_t)n + !!(flags & 1);
  /* RST admission uses SEG.SEQ, never the end of an attached payload. */
  if (flags & 4) {
    if (seq != s->rcv_nxt && (!capacity || before(seq, s->rcv_nxt) ||
                              !before(seq, s->rcv_nxt + capacity)))
      return NC_WAIT;
    if (seq == s->rcv_nxt || NC_MUTANT == 6) {
      abort_connection(s);
      return NC_ABORT;
    }
    s->ack_needed = 1;
    return NC_WAIT;
  }
  int acceptable =
      capacity ? (span ? (!before(seq, s->rcv_nxt) &&
                          before(seq, s->rcv_nxt + capacity)) ||
                             (!before(seq + span - 1, s->rcv_nxt) &&
                              before(seq + span - 1, s->rcv_nxt + capacity))
                       : (!before(seq, s->rcv_nxt) &&
                          before(seq, s->rcv_nxt + capacity)))
               : !span && seq == s->rcv_nxt;
  if (!acceptable) {
    if (!(flags & 4))
      s->ack_needed = 1;
    return NC_WAIT;
  }
  if (!(flags & 16) || flags & 2 || before(s->snd_nxt, ack)) {
    s->ack_needed = 1;
    return NC_WAIT;
  }
  if (s->state == NC_SYN_RECEIVED) {
    if (!s->flight || ack != s->snd_nxt)
      return NC_WAIT;
    s->state = NC_ESTABLISHED;
  }
  if (!before(ack, s->snd_una)) {
    if (NC_MUTANT == 7 || before(s->wl1, seq) ||
        (s->wl1 == seq && !before(ack, s->wl2))) {
      s->peer_window = wnd;
      s->wl1 = seq;
      s->wl2 = ack;
      if (wnd)
        s->probe_due = 0;
      else if (!s->probe_due) {
        s->probe_due = now + s->rto;
        s->probe_rto = s->rto;
      }
    }
    if (before(s->snd_una, ack)) {
      s->snd_una = ack;
      if (s->flight)
        s->due = now + s->rto;
      if (ack == s->snd_nxt && s->flight) {
        if (!s->retransmitted && now - s->sent_at <= 60000) {
          (void)nc_rtt_sample(&s->rtt, (unsigned)(now - s->sent_at), 0);
          s->rto = s->rtt.rto;
        }
        if (s->syn_lost && s->state == NC_ESTABLISHED &&
            s->retained.bytes[33] & 2) {
          if (s->rto < 3000)
            s->rto = 3000;
          s->syn_lost = 0;
        }
        s->flight = 0;
        s->due = 0;
        if (s->state == NC_FIN_WAIT_1)
          s->state = NC_FIN_WAIT_2;
        else if (s->state == NC_CLOSING) {
          s->state = NC_TIME_WAIT;
          s->timewait_until = now + 240000;
        } else if (s->state == NC_LAST_ACK) {
          abort_connection(s);
          return NC_OK;
        }
      }
    }
  }
  if (before(s->rcv_nxt, seq)) {
    s->ack_needed = 1;
    return NC_WAIT;
  }
  int take_fin = capacity && ((seq + (uint32_t)n) - s->rcv_nxt) < capacity;
  size_t skip = before(seq, s->rcv_nxt) ? (size_t)(s->rcv_nxt - seq) : 0;
  if (skip < n && !s->peer_fin) {
    size_t take = n - skip;
    if (take > capacity)
      take = capacity;
    if (nw_rx_offer(&s->rx, p + h + skip, take) != NW_OK)
      return NC_INVALID;
    s->rcv_nxt += (uint32_t)(NC_MUTANT == 8 ? n - skip : take);
  }
  if ((flags & 1) && seq + (uint32_t)n == s->rcv_nxt && !s->peer_fin &&
      take_fin) {
    s->rcv_nxt++;
    s->peer_fin = 1;
    if (s->state == NC_ESTABLISHED)
      s->state = NC_CLOSE_WAIT;
    else if (s->state == NC_FIN_WAIT_1)
      s->state = NC_CLOSING;
    else if (s->state == NC_FIN_WAIT_2) {
      s->state = NC_TIME_WAIT;
      s->timewait_until = now + 240000;
    }
  }
  if (n || (flags & 1))
    s->ack_needed = 1;
  return NC_OK;
}
