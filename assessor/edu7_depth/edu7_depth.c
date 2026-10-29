#include "edu7_depth.h"
#include <limits.h>
static int boolean(unsigned x) { return x <= 1; }
int e7_grant_begin(struct e7_grant *s, uint32_t now, uint32_t *out) {
  if (!s || !out || now < s->last || now > UINT32_MAX - 25000)
    return E7_INVALID;
  if (s->phase || now < s->turn_due)
    return E7_BUSY;
  if (s->generation == UINT32_MAX)
    return E7_FULL;
  s->generation++;
  s->last = now;
  s->turn_due = now + 25000;
  s->start_earliest = now + 200;
  s->start_deadline = now + 2200;
  s->phase = 1;
  *out = s->generation;
  return E7_OK;
}
int e7_grant_first(struct e7_grant *s, uint32_t token, uint32_t now) {
  if (!s || now < s->last || now > UINT32_MAX - 10000)
    return E7_INVALID;
  if (!token || token != s->generation || s->phase != 1)
    return E7_STALE;
  if (now < s->start_earliest)
    return E7_BUSY;
  if (now > s->start_deadline) {
    s->phase = 0;
    s->last = now;
    return E7_START_TIMEOUT;
  }
  s->phase = 2;
  s->end_deadline = now + 10000;
  s->last = now;
  return E7_OK;
}
int e7_grant_finish(struct e7_grant *s, uint32_t token, uint32_t now) {
  if (!s || now < s->last)
    return E7_INVALID;
  if (!token || token != s->generation || s->phase != 2)
    return E7_STALE;
  s->last = now;
  s->phase = 0;
  return now > s->end_deadline ? E7_END_TIMEOUT : E7_OK;
}
int e7_grant_tick(struct e7_grant *s, uint32_t now) {
  if (!s || now < s->last)
    return E7_INVALID;
  s->last = now;
  if (s->phase == 1 && now > s->start_deadline) {
    s->phase = 0;
    return E7_START_TIMEOUT;
  }
#ifdef E7_MUTATE_GRANT
  if (s->phase == 2 && now > s->start_deadline) {
    s->phase = 0;
    return E7_END_TIMEOUT;
  }
#endif
  if (s->phase == 2 && now > s->end_deadline) {
    s->phase = 0;
    return E7_END_TIMEOUT;
  }
  return E7_OK;
}
int e7_link_admit(unsigned kind, unsigned local, unsigned destination,
                  unsigned granted, unsigned transmitting, unsigned fault,
                  unsigned *out) {
  if (!out || kind > 2 || local < 1 || local > 2 || destination < 1 ||
      destination > 2 || !boolean(granted) || !boolean(transmitting) ||
      !boolean(fault))
    return E7_INVALID;
  if (transmitting) {
    *out = 0;
    return E7_OK;
  }
  if (fault || destination != local || (kind == 1 && local != 2))
    return E7_REJECTED;
#ifndef E7_MUTATE_GRANT2
  if (kind != 1 && !granted)
    return E7_REJECTED;
#endif
  *out = kind == 0;
  return E7_OK;
}
static int sender_time(const struct e7_sender *s, uint32_t now) {
  return now >= s->last && now <= UINT32_MAX - 500;
}
int e7_send_begin(struct e7_sender *s, uint16_t id, unsigned src, unsigned dst,
                  const uint8_t *p, size_t n, uint32_t now) {
  size_t i;
  if (!s || !p || !n || n > 120 || !id ||
      !((src == 7 && dst == 8) || (src == 8 && dst == 7)) ||
      !sender_time(s, now))
    return E7_INVALID;
  if (s->active)
    return E7_BUSY;
  if (id <= s->id)
    return E7_STALE;
  for (i = 0; i < n; i++)
    s->bytes[i] = p[i];
  s->length = (uint8_t)n;
  s->id = id;
  s->source = (uint8_t)src;
  s->destination = (uint8_t)dst;
  s->index = 0;
  s->count = (uint8_t)((n + 23) / 24);
  s->active = 1;
  s->waiting = s->retries = s->blocked = s->offered = 0;
  s->credit = 1;
  s->last = s->due = now;
  return E7_OK;
}
int e7_send_offer(struct e7_sender *s, uint32_t now, struct e7_fragment *out, uint32_t *token) {
  struct e7_fragment f = {0};
  unsigned pos, n, i;
  if (!s || !out || !token || !sender_time(s, now))
    return E7_INVALID;
  if (!s->active)
    return E7_STALE;
  if (s->offered)
    return E7_BUSY;
  if (s->blocked && now - s->blocked_since >= 500) {
    s->active = 0;
    s->last = now;
    return E7_TIMEOUT;
  }
  if (now < s->due)
    return E7_WAIT;
  if (s->waiting && s->credit && s->retries == 3) {
    s->active = 0;
    s->last = now;
    return E7_TIMEOUT;
  }
  if (s->offer_token == UINT32_MAX)
    return E7_FULL;
  f.id = s->id;
  f.source = s->source;
  f.destination = s->destination;
  f.index = s->index;
  f.count = s->count;
  f.credit = s->credit;
  f.type = s->credit ? 1 : 3;
  if (s->credit) {
    pos = (unsigned)s->index * 24;
    n = s->length - pos;
    if (n > 24)
      n = 24;
    f.length = (uint8_t)n;
    for (i = 0; i < n; i++)
      f.data[i] = s->bytes[pos + i];
  }
  s->offer = f;
  s->offered = 1;
  s->last = now;
  *token = ++s->offer_token;
  *out = f;
#ifdef E7_MUTATE_OFFER
  s->due = now + 100;
  s->waiting = 1;
#endif
  return E7_OK;
}
int e7_send_accept(struct e7_sender *s, uint32_t token, unsigned accepted,
                   uint32_t now) {
  if (!s || !boolean(accepted) || !sender_time(s, now))
    return E7_INVALID;
  if (!s->active || !s->offered || !token || token != s->offer_token)
    return E7_STALE;
  s->last = now;
  if (s->blocked && now - s->blocked_since >= 500) {
    s->active = s->offered = 0;
    return E7_TIMEOUT;
  }
  s->offered = 0;
  if (!accepted)
    return E7_BUSY;
  if (s->offer.type == 1) {
    if (s->waiting)
      s->retries++;
    s->waiting = 1;
  }
  s->due = now + 100;
  return E7_OK;
}
int e7_send_ack(struct e7_sender *s, const struct e7_fragment *a, uint32_t now) {
  if (!s || !a || !sender_time(s, now) || !boolean(a->credit))
    return E7_INVALID;
  if (!s->active || s->offered || a->type != 2 || a->length || a->id != s->id ||
      a->source != s->destination || a->destination != s->source ||
      a->index != s->index || a->count != s->count)
    return E7_STALE;
  if (!s->waiting && !s->blocked)
    return E7_STALE;
  s->last = now;
  if (s->blocked && now - s->blocked_since >= 500) {
    s->active = 0;
    return E7_TIMEOUT;
  }
  if (s->waiting) {
    s->index++;
    s->waiting = s->retries = 0;
    if (s->index == s->count) {
      s->active = 0;
      return E7_OK;
    }
  }
  if (!a->credit) {
#ifndef E7_MUTATE_OFFER2
    if (!s->blocked)
      s->blocked_since = now;
#else
    s->blocked_since = now;
#endif
    s->blocked = 1;
    s->credit = 0;
    s->due = now + 100;
  } else {
    s->blocked = 0;
    s->credit = 1;
    s->due = now;
  }
  return E7_OK;
}
int e7_send_expire(struct e7_sender *s, uint32_t now) {
  if (!s || !sender_time(s, now))
    return E7_INVALID;
  s->last = now;
  if (s->active && s->blocked && now - s->blocked_since >= 500) {
    s->active = s->offered = 0;
    return E7_TIMEOUT;
  }
  return E7_OK;
}
static int direction(unsigned sender) {
  return sender == 17 ? 0 : sender == 51 ? 1 : -1;
}
static int equal(const uint8_t *a, const uint8_t *b, size_t n) {
  size_t i;
  for (i = 0; i < n; i++)
    if (a[i] != b[i])
      return 0;
  return 1;
}
int e7_session_open(struct e7_session *s, uint16_t session, uint16_t epoch,
                    unsigned schema, unsigned unit) {
  unsigned i;
  if (!s || !session || !epoch || schema != 1 || unit != 1)
    return E7_INVALID;
  if (s->open)
    return s->session == session && s->epoch == epoch ? E7_DUPLICATE : E7_BUSY;
  if (epoch <= s->epoch)
    return E7_STALE;
  s->session = session;
  s->epoch = epoch;
  s->holder = 17;
  s->token_pending = s->pending_length = 0;
  s->pending_token = 0;
  s->open = 1;
  for (i = 0; i < 2; i++) {
    s->receipt[i].checkpoint = 0;
    s->receipt[i].request_length = s->receipt[i].response_length = 0;
  }
  return E7_OK;
}
int e7_session_prepare(struct e7_session *s, unsigned sender, uint16_t session,
                       uint16_t epoch, uint16_t cp, const uint8_t *p, size_t n,
                       uint32_t *token, uint8_t *cached, size_t cap,
                       size_t *cached_length) {
  int d = direction(sender);
  struct e7_receipt *r;
  size_t i;
  if (!s || !p || !n || n > 80 || !token || !cached_length ||
      (!cached && cap) || !cp || d < 0)
    return E7_INVALID;
  if (!s->open || s->session != session || s->epoch != epoch)
    return E7_STALE;
  r = &s->receipt[d];
  if (cp == r->checkpoint) {
    if (n != r->request_length || !equal(p, r->request, n))
      return E7_REJECTED;
    if (cap < r->response_length)
      return E7_FULL;
    for (i = 0; i < r->response_length; i++)
      cached[i] = r->response[i];
    *cached_length = r->response_length;
    *token = 0;
    return E7_DUPLICATE;
  }
  if (s->pending_token || s->token_pending || s->holder != sender)
    return E7_BUSY;
  if (r->checkpoint == UINT16_MAX || cp != (unsigned)r->checkpoint + 1)
    return E7_STALE;
  if (s->generation == UINT32_MAX)
    return E7_FULL;
  s->pending_direction = (uint8_t)d;
  s->pending_checkpoint = cp;
  s->pending_length = (uint8_t)n;
  for (i = 0; i < n; i++)
    s->pending[i] = p[i];
  s->pending_token = ++s->generation;
  *token = s->pending_token;
  *cached_length = 0;
#ifdef E7_MUTATE_SESSION
  r->checkpoint = cp;
#endif
  return E7_OK;
}
int e7_session_commit(struct e7_session *s, uint32_t token, unsigned accepted,
                      const uint8_t *response, size_t n) {
  struct e7_receipt *r;
  size_t i;
  if (!s || !boolean(accepted) || n > 80 || (!response && n) ||
      (!accepted && n))
    return E7_INVALID;
  if (!s->open || !token || token != s->pending_token)
    return E7_STALE;
  if (accepted) {
    r = &s->receipt[s->pending_direction];
    r->checkpoint = s->pending_checkpoint;
    r->request_length = s->pending_length;
    r->response_length = (uint8_t)n;
    for (i = 0; i < r->request_length; i++)
      r->request[i] = s->pending[i];
    for (i = 0; i < n; i++)
      r->response[i] = response[i];
#ifdef E7_MUTATE_SESSION2
    s->receipt[1 - s->pending_direction].checkpoint = r->checkpoint;
#endif
  }
  s->pending_token = 0;
  s->pending_length = 0;
  return E7_OK;
}
int e7_token_begin(struct e7_session *s, unsigned sender, unsigned next) {
  if (!s || direction(sender) < 0 || direction(next) < 0 || sender == next)
    return E7_INVALID;
  if (!s->open || s->holder != sender || s->token_pending || s->pending_token)
    return E7_BUSY;
  s->token_pending = (uint8_t)next;
  return E7_OK;
}
int e7_token_complete(struct e7_session *s, unsigned next, unsigned confirmed) {
  if (!s || direction(next) < 0 || !boolean(confirmed))
    return E7_INVALID;
  if (!s->open || s->token_pending != next)
    return E7_STALE;
  if (!confirmed)
    return E7_BUSY;
  s->holder = (uint8_t)next;
  s->token_pending = 0;
  return E7_OK;
}
int e7_session_resume(const struct e7_session *s, uint16_t session, uint16_t epoch,
                      unsigned sender, uint16_t cp) {
  int d = direction(sender);
  if (!s || d < 0)
    return E7_INVALID;
  return s->open && s->session == session && s->epoch == epoch &&
                 s->receipt[d].checkpoint == cp
             ? E7_OK
             : E7_STALE;
}
int e7_session_close(struct e7_session *s) {
  if (!s)
    return E7_INVALID;
  if (s->pending_token || s->token_pending)
    return E7_BUSY;
  s->open = 0;
  return E7_OK;
}
