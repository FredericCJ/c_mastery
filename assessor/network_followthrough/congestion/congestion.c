#include "congestion.h"
#ifndef NR_MUTANT
#define NR_MUTANT 0
#endif
static uint32_t minimum(uint32_t a, uint32_t b) { return a < b ? a : b; }
static uint32_t bounded_add(uint32_t a, uint32_t b) {
  return a > NR_LIMIT - b ? NR_LIMIT : a + b;
}
uint32_t nr_flight(const NrControl *s) { return s->nxt - s->una; }
static uint32_t normal_usable(const NrControl *s) {
  uint32_t limit = minimum(s->cwnd, s->rwnd), flight = nr_flight(s);
  return flight >= limit ? 0 : limit - flight;
}
uint32_t nr_usable(const NrControl *s) {
  uint32_t limit = NR_MUTANT == 1 ? s->cwnd : minimum(s->cwnd, s->rwnd),
           flight = nr_flight(s);
  uint32_t available = flight >= limit ? 0 : limit - flight;
  if (s->limited && s->quota &&
      (!s->limited_length[0] || !s->limited_length[1])) {
    uint32_t extended = minimum(bounded_add(s->cwnd, 2 * s->mss), s->rwnd);
    uint32_t extra = extended > flight ? minimum(extended - flight, s->mss) : 0;
    if (extra > available)
      available = extra;
  }
  return available;
}
int nr_init(NrControl *s, uint32_t mss, uint32_t isn, uint32_t window) {
  if (!s || !mss || mss > 1460 || window > 65535)
    return NR_INVALID;
  *s = (NrControl){0};
  s->mss = s->iw = s->cwnd = mss;
  s->ssthresh = 65535;
  s->rwnd = window;
  s->una = s->nxt = isn;
  return NR_OK;
}
int nr_policy(NrControl *s, unsigned enabled) {
  if (!s || enabled > 1)
    return NR_INVALID;
  if (nr_flight(s))
    return NR_BUSY;
  s->limited = enabled;
  s->quota = 0;
  return NR_OK;
}
int nr_send(NrControl *s, uint32_t bytes, unsigned copied, uint64_t now,
            uint32_t *sequence) {
  if (!s || !sequence || !bytes || bytes > s->mss || copied > 1 || now < s->now)
    return NR_INVALID;
  if (bytes > nr_usable(s))
    return NR_BUSY;
  if (!copied)
    return NR_WAIT;
  if (s->limited && bytes > normal_usable(s)) {
    unsigned i = s->limited_length[0] ? 1 : 0;
    s->limited_start[i] = s->nxt;
    s->limited_length[i] = bytes;
    s->quota = 0;
  }
  *sequence = s->nxt;
  s->nxt += bytes;
  s->now = s->last_send = now;
  return NR_OK;
}
static uint32_t threshold(const NrControl *s, unsigned fast) {
  uint32_t flight = nr_flight(s);
  if (fast && NR_MUTANT != 8)
    flight -= s->limited_length[0] + s->limited_length[1];
  uint32_t half = (NR_MUTANT == 2 ? s->cwnd : flight) / 2;
  return half < 2 * s->mss ? 2 * s->mss : half;
}
int nr_ack(NrControl *s, uint32_t ack, uint32_t window, unsigned payload,
           unsigned flags, uint64_t now) {
  if (!s || window > 65535 || now < s->now || !(flags & 16) || (flags & ~0x18u))
    return NR_INVALID;
  uint32_t distance = ack - s->una, flight = nr_flight(s);
  if (distance > flight)
    return distance >= UINT32_C(0x80000000) ? NR_WAIT : NR_INVALID;
  unsigned duplicate = flight && !distance && !payload && window == s->rwnd;
  if (NR_MUTANT == 3)
    duplicate = flight && !distance;
  s->now = now;
  s->rwnd = window;
  if (distance) {
    for (unsigned i = 0; i < 2; i++)
      if (s->limited_length[i]) {
        uint32_t offset = s->limited_start[i] - s->una;
        if (distance > offset) {
          uint32_t used = minimum(distance - offset, s->limited_length[i]);
          s->limited_start[i] += used;
          s->limited_length[i] -= used;
        }
      }
    s->una = ack;
    s->duplicates = 0;
    s->quota = 0;
    s->timeout_seen = 0;
    if (s->recovery) {
      s->cwnd = s->ssthresh;
      s->recovery = 0;
      s->credit = 0;
    } else if (s->cwnd < s->ssthresh)
      s->cwnd = bounded_add(
          s->cwnd, NR_MUTANT == 4 ? s->mss : minimum(distance, s->mss));
    else {
      s->credit += distance;
      if (s->credit >= s->cwnd) {
        s->credit = 0;
        s->cwnd = bounded_add(s->cwnd, s->mss);
      }
    }
    return NR_OK;
  }
  if (!duplicate) {
    if (NR_MUTANT == 9)
      s->recovery_used = 3;
    s->duplicates = 0;
    s->quota = 0;
    return NR_WAIT;
  }
  if (s->duplicates != UINT32_MAX)
    s->duplicates++;
  s->quota = s->limited && !s->recovery && s->duplicates <= 2;
  if (!s->recovery && s->duplicates == 3) {
    s->ssthresh = threshold(s, 1);
    s->cwnd = bounded_add(s->ssthresh, 3 * s->mss);
    s->recovery = 1;
    s->credit = 0;
    s->recovery_limit = (flight + s->mss - 1) / s->mss;
    s->recovery_used = 3;
    return NR_FAST_RETRANSMIT;
  }
  if (s->recovery && (NR_MUTANT == 5 || s->recovery_used < s->recovery_limit)) {
    s->cwnd = bounded_add(s->cwnd, s->mss);
    if (s->recovery_used != UINT32_MAX)
      s->recovery_used++;
  }
  return NR_WAIT;
}
int nr_timeout(NrControl *s, uint64_t now) {
  if (!s || now < s->now)
    return NR_INVALID;
  if (!nr_flight(s))
    return NR_WAIT;
  if (!s->timeout_seen || s->timeout_una != s->una || NR_MUTANT == 6)
    s->ssthresh = threshold(s, 0);
  s->timeout_seen = 1;
  s->timeout_una = s->una;
  s->cwnd = s->mss;
  s->credit = s->duplicates = s->recovery = s->quota = 0;
  s->now = now;
  return NR_OK;
}
int nr_restart(NrControl *s, uint64_t now, uint32_t rto) {
  if (!s || now < s->now || rto < 1000 || rto > 60000)
    return NR_INVALID;
  if (nr_flight(s))
    return NR_BUSY;
  if (now - s->last_send > rto && NR_MUTANT != 7) {
    s->cwnd = minimum(s->iw, s->cwnd);
    s->credit = 0;
  }
  s->now = now;
  return NR_OK;
}
