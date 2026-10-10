#include "com_net.h"
#include <string.h>
static const uint8_t local_mac[6] = {2, 0, 0, 0, 0, 2},
                     peer_mac[6] = {2, 0, 0, 0, 0, 1};
static const uint8_t local_ip[4] = {192, 0, 2, 2}, peer_ip[4] = {192, 0, 2, 1};
static uint16_t u16(const uint8_t *p) {
  return (uint16_t)((unsigned)p[0] * 256 + p[1]);
}
static uint32_t u32(const uint8_t *p) {
  return ((uint32_t)u16(p) << 16) | u16(p + 2);
}
static void w16(uint8_t *p, unsigned n) {
  p[0] = (uint8_t)(n >> 8);
  p[1] = (uint8_t)n;
}
static void w32(uint8_t *p, uint32_t n) {
  w16(p, n >> 16);
  w16(p + 2, n);
}
static int before(uint32_t a, uint32_t b) {
  return a != b && (uint32_t)(b - a) < UINT32_C(0x80000000);
}
uint16_t net_checksum(const void *v, size_t n, uint32_t sum) {
  const uint8_t *p = v;
  while (n >= 2) {
    sum += u16(p);
    p += 2;
    n -= 2;
    sum = (sum & 65535) + (sum >> 16);
  }
  if (n)
    sum += (unsigned)p[0] << 8;
  while (sum >> 16)
    sum = (sum & 65535) + (sum >> 16);
  return (uint16_t)~sum;
}
void net_init(NetStack *s, uint32_t isn) {
  memset(s, 0, sizeof *s);
  s->isn = isn;
  s->mss = 256;
  s->rto = 1000;
  http_init(&s->http, "192.0.2.2:8080");
}
void net_link_down(NetStack *s) {
  unsigned incarnation = s->incarnation + 1;
  uint32_t isn = s->isn + 65537;
  net_init(s, isn);
  s->incarnation = incarnation;
}
void net_rtt(NetStack *s, uint32_t r) {
  if (!r)
    r = 1;
  if (!s->rtt_samples) {
    s->srtt = r;
    s->rttvar = r / 2;
  } else {
    uint32_t d = s->srtt > r ? s->srtt - r : r - s->srtt;
    s->rttvar = (uint32_t)((3 * (uint64_t)s->rttvar + d) / 4);
    s->srtt = (uint32_t)((7 * (uint64_t)s->srtt + r) / 8);
  }
  s->rtt_samples++;
  uint64_t variation = 4 * (uint64_t)s->rttvar;
  uint64_t v = s->srtt + (variation ? variation : 1);
  s->rto = (uint32_t)(v < 1000 ? 1000 : v > 60000 ? 60000 : v);
}
static void ethernet(NetPacket *o, unsigned type) {
  memset(o, 0, sizeof *o);
  memcpy(o->bytes, peer_mac, 6);
  memcpy(o->bytes + 6, local_mac, 6);
  w16(o->bytes + 12, type);
}
static void ip_header(NetPacket *o, unsigned protocol, size_t size) {
  ethernet(o, 0x800);
  uint8_t *p = o->bytes + 14;
  p[0] = 0x45;
  w16(p + 2, (unsigned)size + 20);
  w16(p + 6, 0x4000);
  p[8] = 64;
  p[9] = (uint8_t)protocol;
  memcpy(p + 12, local_ip, 4);
  memcpy(p + 16, peer_ip, 4);
  w16(p + 10, net_checksum(p, 20, 0));
  o->length = 34 + size;
  if (o->length < 60)
    o->length = 60;
}
static void tcp(NetStack *s, NetPacket *o, unsigned flags, const void *data,
                size_t n, int retain) {
  size_t hdr = (flags & 2) ? 24 : 20;
  ip_header(o, 6, hdr + n);
  uint8_t *p = o->bytes + 34;
  w16(p, 8080);
  w16(p + 2, s->peer_port);
  w32(p + 4, s->snd_nxt);
  w32(p + 8, s->rcv_nxt);
  p[12] = (uint8_t)(hdr / 4 << 4);
  p[13] = (uint8_t)flags;
  w16(p + 14, 512);
  if (hdr == 24) {
    p[20] = 2;
    p[21] = 4;
    w16(p + 22, 256);
  }
  if (n)
    memcpy(p + hdr, data, n);
  uint32_t pseudo = u16(local_ip) + u16(local_ip + 2) + u16(peer_ip) +
                    u16(peer_ip + 2) + 6 + (uint32_t)(hdr + n);
  w16(p + 16, net_checksum(p, hdr + n, pseudo));
  if (retain) {
    s->retained = *o;
    s->flight = 1;
    s->sent_at = s->now;
    s->due = s->now + s->rto;
    s->retries = 0;
    s->retransmitted = 0;
    s->snd_nxt += (uint32_t)n + !!(flags & 2) + !!(flags & 1);
  }
}
static int output_next(NetStack *s, NetPacket *o) {
  if (s->flight || !(s->state == NET_ESTABLISHED || s->state == NET_CLOSE_WAIT))
    return 0;
  if (s->interim_pending) {
    static const char interim[] = "HTTP/1.1 100 Continue\r\n\r\n";
    if (s->peer_window < sizeof interim - 1)
      return 0;
    tcp(s, o, 0x18, interim, sizeof interim - 1, 1);
    s->interim_pending = 0;
    return 1;
  }
  if (s->response_sent < s->response_length) {
    size_t n = s->response_length - s->response_sent;
    if (n > s->mss)
      n = s->mss;
    if (n > s->peer_window)
      n = s->peer_window;
    if (!n) {
      if (!s->due)
        s->due = s->now + s->rto;
      return 0;
    }
    tcp(s, o, 0x18, s->response + s->response_sent, n, 1);
    s->response_sent += n;
    return 1;
  }
  if (s->response_length && s->response_acked == s->response_length &&
      !s->fin_sent) {
    tcp(s, o, 0x11, 0, 0, 1);
    s->fin_sent = 1;
    s->state = s->peer_fin ? NET_LAST_ACK : NET_FIN_WAIT_1;
    return 1;
  }
  return 0;
}
static int response(NetStack *s) {
  s->http.request.close = 1;
  return http_response(&s->http.request, "Sat, 10 Oct 2026 00:00:00 GMT", 1,
                       s->response, sizeof s->response, &s->response_length);
}
static int drop(NetStack *s) {
  s->drops++;
  return 0;
}
int net_receive(NetStack *s, const void *v, size_t n, uint64_t now,
                NetPacket *o) {
  const uint8_t *p = v;
  o->length = 0;
  if (now < s->now)
    return -1;
  s->now = now;
  if (n < 14 || memcmp(p + 6, peer_mac, 6))
    return drop(s);
  static const uint8_t broadcast[6] = {255, 255, 255, 255, 255, 255};
  unsigned type = u16(p + 12);
  int mine = !memcmp(p, local_mac, 6), bcast = !memcmp(p, broadcast, 6);
  if (type == 0x806) {
    if (!(mine || bcast) || n < 42 || u16(p + 14) != 1 ||
        u16(p + 16) != 0x800 || p[18] != 6 || p[19] != 4 ||
        !(u16(p + 20) == 1 || u16(p + 20) == 2) || memcmp(p + 22, p + 6, 6) ||
        memcmp(p + 28, peer_ip, 4) || memcmp(p + 38, local_ip, 4))
      return drop(s);
    s->peer_known = 1;
    s->arp_expires = now + 30000;
    if (u16(p + 20) == 1) {
      ethernet(o, 0x806);
      uint8_t *a = o->bytes + 14;
      w16(a, 1);
      w16(a + 2, 0x800);
      a[4] = 6;
      a[5] = 4;
      w16(a + 6, 2);
      memcpy(a + 8, local_mac, 6);
      memcpy(a + 14, local_ip, 4);
      memcpy(a + 18, peer_mac, 6);
      memcpy(a + 24, peer_ip, 4);
      o->length = 60;
      return 1;
    }
    return 0;
  }
  if (type != 0x800 || !mine || n < 34 || !s->peer_known ||
      now >= s->arp_expires)
    return drop(s);
  p += 14;
  n -= 14;
  unsigned total = u16(p + 2);
  if (p[0] != 0x45 || total < 20 || total > 1500 || total > n || !p[8] ||
      (u16(p + 6) & 0xbfff) || net_checksum(p, 20, 0) ||
      memcmp(p + 12, peer_ip, 4) || memcmp(p + 16, local_ip, 4))
    return drop(s);
  const uint8_t *q = p + 20;
  size_t size = total - 20;
  if (p[9] == 1) {
    if (size < 8 || size > 72 || q[0] != 8 || q[1] || net_checksum(q, size, 0))
      return drop(s);
    ip_header(o, 1, size);
    memcpy(o->bytes + 34, q, size);
    o->bytes[34] = 0;
    w16(o->bytes + 36, 0);
    w16(o->bytes + 36, net_checksum(o->bytes + 34, size, 0));
    return 1;
  }
  if (p[9] != 6 || size < 20 || u16(q + 2) != 8080)
    return drop(s);
  size_t h = (size_t)(q[12] >> 4) * 4;
  unsigned flags = q[13];
  uint32_t seq = u32(q + 4), ack = u32(q + 8);
  unsigned wnd = u16(q + 14);
  uint32_t pseudo = u16(peer_ip) + u16(peer_ip + 2) + u16(local_ip) +
                    u16(local_ip + 2) + 6 + (uint32_t)size;
  if (h < 20 || h > size || (q[12] & 15) || (flags & 0xe0) ||
      net_checksum(q, size, pseudo))
    return drop(s);
  unsigned mss = 536;
  for (size_t i = 20; i < h;) {
    unsigned kind = q[i++];
    if (!kind)
      break;
    if (kind == 1)
      continue;
    if (i == h)
      return drop(s);
    unsigned len = q[i++];
    if (len < 2 || len - 2 > h - i)
      return drop(s);
    if (kind == 2) {
      if (len != 4 || !(flags & 2) || !u16(q + i))
        return drop(s);
      mss = u16(q + i);
    }
    i += len - 2;
  }
  size_t bytes = size - h;
  if (s->state == NET_LISTEN) {
    if (flags != 2 || bytes)
      return drop(s);
    uint64_t expiry = s->arp_expires;
    unsigned incarnation = s->incarnation, drops = s->drops, resets = s->resets;
    uint32_t isn = s->isn;
    net_init(s, isn);
    s->peer_known = 1;
    s->arp_expires = expiry;
    s->now = now;
    s->incarnation = incarnation;
    s->drops = drops;
    s->resets = resets;
    s->peer_port = u16(q);
    s->rcv_nxt = seq + 1;
    s->snd_una = s->snd_nxt = s->isn;
    s->peer_window = (uint16_t)wnd;
    s->mss = (uint16_t)(mss < 256 ? mss : 256);
    s->wl1 = seq;
    s->wl2 = ack;
    s->state = NET_SYN_RECEIVED;
    tcp(s, o, 0x12, 0, 0, 1);
    return 1;
  }
  if (u16(q) != s->peer_port)
    return drop(s);
  if (s->state == NET_TIME_WAIT) {
    /* Only the already accepted FIN position may refresh this incarnation. */
    if ((flags & 1) && !(flags & (2 | 4)) &&
        seq + (uint32_t)bytes + 1 == s->rcv_nxt) {
      tcp(s, o, 0x10, 0, 0, 0);
      s->timewait_until = now + 240000;
      return 1;
    }
    return drop(s);
  }
  if ((flags & 2) && s->state == NET_SYN_RECEIVED && seq + 1 == s->rcv_nxt) {
    *o = s->retained;
    return 1;
  }
  /* Every live sequence span is below 2^31. Do not cast wrap differences to
   * signed C. */
  size_t span = bytes + !!(flags & 1);
  uint32_t right = s->rcv_nxt + 512;
  int acceptable = span ? (!before(seq, s->rcv_nxt) && before(seq, right)) ||
                              (!before(seq + (uint32_t)span - 1, s->rcv_nxt) &&
                               before(seq + (uint32_t)span - 1, right))
                        : (!before(seq, s->rcv_nxt) && before(seq, right));
  if (!acceptable) {
    if (!(flags & 4)) {
      tcp(s, o, 0x10, 0, 0, 0);
      return 1;
    }
    return drop(s);
  }
  if (flags & 4) {
    if (seq == s->rcv_nxt) {
      s->resets++;
      s->state = NET_LISTEN;
      s->flight = 0;
      return 0;
    }
    tcp(s, o, 0x10, 0, 0, 0);
    return 1;
  }
  if (!(flags & 16) || flags & 2 || before(s->snd_nxt, ack)) {
    tcp(s, o, 0x10, 0, 0, 0);
    return 1;
  }
  if (s->state == NET_SYN_RECEIVED) {
    if (ack != s->snd_nxt)
      return drop(s);
    s->state = NET_ESTABLISHED;
    if (s->retransmitted)
      s->rto = 3000;
  }
  if (!before(ack, s->snd_una)) {
    if (before(s->wl1, seq) || (s->wl1 == seq && !before(ack, s->wl2))) {
      s->peer_window = (uint16_t)wnd;
      s->wl1 = seq;
      s->wl2 = ack;
    }
    if (before(s->snd_una, ack)) {
      uint32_t advanced = ack - s->snd_una;
      s->snd_una = ack;
      if (s->flight)
        s->due = now + s->rto;
      if (s->response_sent > s->response_acked) {
        size_t remain = s->response_sent - s->response_acked;
        s->response_acked += advanced > remain ? remain : advanced;
      }
      if (ack == s->snd_nxt && s->flight) {
        if (!s->retransmitted)
          net_rtt(s, (uint32_t)(now - s->sent_at));
        s->flight = 0;
        s->due = 0;
        if (s->state == NET_FIN_WAIT_1)
          s->state = NET_FIN_WAIT_2;
        else if (s->state == NET_CLOSING) {
          s->state = NET_TIME_WAIT;
          s->timewait_until = now + 240000;
        } else if (s->state == NET_LAST_ACK) {
          s->state = NET_LISTEN;
          return 0;
        }
      }
    }
  }
  size_t skip = before(seq, s->rcv_nxt) ? (size_t)(s->rcv_nxt - seq) : 0;
  if (before(s->rcv_nxt, seq)) {
    tcp(s, o, 0x10, 0, 0, 0);
    return 1;
  }
  if (skip < bytes && !s->response_length) {
    size_t take = bytes - skip;
    if (take > 512)
      take = 512;
    size_t off = 0;
    while (off < take) {
      size_t used;
      int r = http_feed(&s->http, q + h + skip + off, take - off, &used);
      off += used;
      if (r == HTTP_CONTINUE) {
        s->interim_pending = 1;
        continue;
      }
      if (r == HTTP_DONE || r == HTTP_ERROR) {
        if (response(s))
          return -1;
        break;
      }
      if (!used)
        break;
    }
    /* TCP acceptance is independent of whether HTTP accepted the message. */
    s->rcv_nxt += (uint32_t)take;
  }
  uint32_t fin_at = seq + (uint32_t)bytes;
  if ((flags & 1) && fin_at == s->rcv_nxt && !s->peer_fin) {
    s->rcv_nxt++;
    s->peer_fin = 1;
    if (s->state == NET_ESTABLISHED)
      s->state = NET_CLOSE_WAIT;
    else if (s->state == NET_FIN_WAIT_1)
      s->state = NET_CLOSING;
    else if (s->state == NET_FIN_WAIT_2) {
      s->state = NET_TIME_WAIT;
      s->timewait_until = now + 240000;
    }
    if (!s->response_length) {
      http_eof(&s->http);
      if (response(s))
        return -1;
    }
  }
  if (output_next(s, o))
    return 1;
  if (bytes || (flags & 1)) {
    tcp(s, o, 0x10, 0, 0, 0);
    return 1;
  }
  return 0;
}
int net_tick(NetStack *s, uint64_t now, NetPacket *o) {
  o->length = 0;
  if (now < s->now)
    return -1;
  s->now = now;
  if (s->state == NET_TIME_WAIT && now >= s->timewait_until) {
    s->state = NET_LISTEN;
    return 0;
  }
  if (s->flight && now >= s->due) {
    if (s->retries++ >= 3) {
      s->state = NET_LISTEN;
      s->flight = 0;
      return -1;
    }
    s->rto = s->rto > 30000 ? 60000 : s->rto * 2;
    s->due = now + s->rto;
    s->retransmitted = 1;
    *o = s->retained;
    return 1;
  }
  if (!s->flight && s->response_sent < s->response_length && !s->peer_window &&
      s->due && now >= s->due) {
    /* ACK probe of the byte immediately preceding SND.NXT, no new byte
     * accepted. */
    uint32_t saved = s->snd_nxt;
    s->snd_nxt--;
    tcp(s, o, 0x10, 0, 0, 0);
    s->snd_nxt = saved;
    s->rto = s->rto > 30000 ? 60000 : s->rto * 2;
    s->due = now + s->rto;
    return 1;
  }
  return output_next(s, o);
}
