#include "session.h"
#include <limits.h>
#include <string.h>
#ifndef ES_MUTANT
#define ES_MUTANT 0
#endif
static unsigned get16(const uint8_t *p)
{
    return (unsigned)p[0] * 256 + p[1];
}

static void put16(uint8_t *p, unsigned x)
{
    p[0] = (uint8_t)(x >> 8);
    p[1] = (uint8_t)x;
}

int es_init(struct es_peer *s, unsigned self, unsigned profile,
            uint16_t incarnation)
{
    if (!s || (self != 17 && self != 51) || s->self)
    {
        return E7_INVALID;
    }
    struct es_peer trial = {0};
    trial.self = self;
    trial.peer = self == 17 ? 51 : 17;
    trial.setpoint = 2200;
    if (et_tx_open(&trial.tx, profile, incarnation, self) ||
        et_rx_open(&trial.rx, profile, incarnation, self))
    {
        return E7_INVALID;
    }
    *s = trial;
    return E7_OK;
}

int es_permitted(const struct es_peer *s)
{
    return s && s->phase == ES_READY && !s->uncertain && !s->outgoing_kind &&
           s->core.open && s->core.holder == s->self &&
           (ES_MUTANT == 1 || !s->core.token_pending);
}

int es_emit(struct es_peer *s, unsigned kind, uint16_t sid, uint16_t epoch,
            uint16_t cp, const uint8_t *p, size_t n, uint16_t transfer,
            uint32_t now)
{
    if (!s || !s->self || !sid || !epoch || kind < 1 || kind > 7 || n > 80 ||
        (!p && n) || !transfer)
    {
        return E7_INVALID;
    }
    if (s->outgoing_kind || s->uncertain)
    {
        return E7_BUSY;
    }
    struct es_peer trial = *s;
    if (kind == 1)
    {
        if (s->self != 17 || cp || n != 3 || p[0] != 1 || p[1] != 1 ||
            p[2] != 17 || s->phase != ES_CLOSED)
        {
            return E7_REJECTED;
        }
        int r = e7_session_open(&trial.core, sid, epoch, 1, 1);
        if (r)
        {
            return r;
        }
        trial.phase = ES_OPENING;
        trial.sent_checkpoint = trial.sent_length = 0;
        trial.error_session = trial.error_epoch = 0;
    }
    else if (kind == 7)
    {
        if (cp || n || sid != s->error_session || epoch != s->error_epoch)
        {
            return E7_STALE;
        }
    }
    else
    {
        if (!s->core.open || sid != s->core.session || epoch != s->core.epoch)
        {
            return E7_STALE;
        }
        if (kind == 2)
        {
            if (s->self != 51 || s->phase != ES_ACK_NEEDED || cp || n != 3 ||
                p[0] != 1 || p[1] != 1 || p[2] != 17)
            {
                return E7_REJECTED;
            }
            trial.phase = ES_ACKING;
        }
        else if (kind == 3)
        {
            if (!es_permitted(s))
            {
                return E7_BUSY;
            }
            if (!cp)
            {
                return E7_STALE;
            }
            if (cp == s->sent_checkpoint)
            {
                if (n != s->sent_length || memcmp(p, s->sent, n))
                {
                    return E7_REJECTED;
                }
            }
            else if (s->sent_checkpoint == UINT16_MAX ||
                     cp != (unsigned)s->sent_checkpoint + 1)
            {
                return E7_STALE;
            }
            struct edu_record r;
            if (edu_record_decode(p, n, &r) ||
                (s->self == 17 ? r.verb > 2 : r.verb < 3))
            {
                return E7_REJECTED;
            }
        }
        else if (kind == 5)
        {
            if (cp || n != 1 || p[0] != s->peer || !es_permitted(s))
            {
                return E7_REJECTED;
            }
            int r = e7_token_begin(&trial.core, s->self, s->peer);
            if (r)
            {
                return r;
            }
        }
        else if (kind == 4)
        {
            if (n || s->phase != ES_READY || cp != s->sent_checkpoint)
            {
                return E7_REJECTED;
            }
        }
        else if (kind == 6)
        {
            if (cp || n || s->phase != ES_READY || s->core.token_pending ||
                s->core.pending_token)
            {
                return E7_BUSY;
            }
        }
        else
        {
            return E7_REJECTED;
        }
    }
    uint8_t wire[87];
    wire[0] = (uint8_t)kind;
    put16(wire + 1, sid);
    put16(wire + 3, epoch);
    put16(wire + 5, cp);
    if (n)
    {
        memcpy(wire + 7, p, n);
    }
    int r = et_begin(&trial.tx, transfer, wire, n + 7, now);
    if (r)
    {
        return r;
    }
    trial.outgoing_kind = kind;
    trial.outgoing_checkpoint = cp;
    trial.outgoing_id = transfer;
    *s = trial;
    return E7_OK;
}

int es_offer(struct es_peer *s, uint32_t now, uint8_t *p, size_t cap, size_t *n,
             uint32_t *token)
{
    if (!s)
    {
        return E7_INVALID;
    }
    return et_offer(&s->tx, now, p, cap, n, token);
}

int es_admit(struct es_peer *s, uint16_t epoch, uint32_t token,
             unsigned accepted, uint32_t now)
{
    if (!s)
    {
        return E7_INVALID;
    }
    int r = et_accept(&s->tx, epoch, token, accepted, now);
    if (ES_MUTANT == 1 && r == E7_OK && s->outgoing_kind == 5)
    {
        s->core.token_pending = 0;
        s->outgoing_kind = 0;
    }
    return r;
}

int es_ack(struct es_peer *s, const uint8_t *p, size_t n, uint32_t now)
{
    if (!s)
    {
        return E7_INVALID;
    }
    struct es_peer trial = *s;
    int r = et_ack(&trial.tx, p, n, now);
    if (r)
    {
        return r;
    }
    if (!trial.tx.inner.active && trial.outgoing_kind)
    {
        if (trial.outgoing_kind == 5)
        {
            r = e7_token_complete(&trial.core, trial.peer, 1);
            if (r)
            {
                return r;
            }
        }
        if (trial.outgoing_kind == 3)
        {
            trial.sent_checkpoint = trial.outgoing_checkpoint;
            trial.sent_length = (uint8_t)(trial.tx.inner.length - 7);
            memcpy(trial.sent, trial.tx.inner.bytes + 7, trial.sent_length);
        }
        if (trial.outgoing_kind == 7)
        {
            trial.error_session = trial.error_epoch = 0;
        }
        if (trial.outgoing_kind == 2)
        {
            trial.phase = ES_READY;
        }
        if (trial.outgoing_kind == 6)
        {
            r = e7_session_close(&trial.core);
            if (r)
            {
                return r;
            }
            trial.phase = ES_CLOSED;
        }
        trial.outgoing_kind = 0;
    }
    *s = trial;
    return E7_OK;
}

int es_receive(struct es_peer *s, const uint8_t *p, size_t n, uint32_t now,
               uint8_t *ack, size_t cap, size_t *length)
{
    if (!s)
    {
        return E7_INVALID;
    }
    return et_receive(&s->rx, p, n, now, ack, cap, length);
}

static int process(struct es_peer *s, const uint8_t *p, size_t n, size_t cap,
                   struct es_event *event)
{
    if (n < 7 || n > 87 || p[0] < 1 || p[0] > 7)
    {
        return E7_REJECTED;
    }
    unsigned kind = p[0];
    uint16_t sid = (uint16_t)get16(p + 1), epoch = (uint16_t)get16(p + 3),
             cp = (uint16_t)get16(p + 5);
    size_t len = n - 7;
    const uint8_t *body = p + 7;
    if (!sid || !epoch)
    {
        return E7_REJECTED;
    }
    struct es_event e = {.type = kind, .checkpoint = cp};
    if (kind == 1)
    {
        if (s->self != 51 || cp || len != 3 || body[0] != 1 ||
            (ES_MUTANT != 2 && body[1] != 1) || body[2] != 17)
        {
            return E7_REJECTED;
        }
        int r = e7_session_open(&s->core, sid, epoch, 1, 1);
        if (r != E7_OK && r != E7_DUPLICATE)
        {
            return r;
        }
        if (r == E7_OK)
        {
            s->phase = ES_ACK_NEEDED;
            s->sent_checkpoint = s->sent_length = 0;
            s->error_session = s->error_epoch = 0;
        }
    }
    else
    {
        if (ES_MUTANT == 8 && kind == 4 && !s->core.open)
        {
            *event = e;
            return ES_DELIVERED;
        }
        if (!s->core.open || sid != s->core.session ||
            (ES_MUTANT != 7 && epoch != s->core.epoch))
        {
            return E7_STALE;
        }
        if (kind == 2)
        {
            if (s->self != 17 || s->phase != ES_OPENING || cp || len != 3 ||
                body[0] != 1 || body[1] != 1 || body[2] != 17)
            {
                return E7_REJECTED;
            }
            s->phase = ES_READY;
        }
        else if (kind == 3)
        {
            if (s->phase != ES_READY && s->phase != ES_ACKING)
            {
                return E7_BUSY;
            }
            uint32_t token = 0;
            size_t cached = 0;
            int r = e7_session_prepare(&s->core, s->peer, sid, epoch, cp, body,
                                       len, &token, e.reply,
                                       cap < 80 ? cap : 80, &cached);
            if (r == E7_DUPLICATE)
            {
                if (ES_MUTANT == 9)
                {
                    s->effects++;
                }
                e.duplicate = 1;
                e.length = cached;
                *event = e;
                return ES_DUPLICATE;
            }
            if (r)
            {
                return r;
            }
            struct edu_record record, reply;
            uint8_t response[33];
            size_t z = 0;
            int16_t value = s->setpoint;
            if (ES_MUTANT == 3)
            {
                s->core.receipt[s->peer == 17 ? 0 : 1].checkpoint = cp;
            }
            if (edu_record_decode(body, len, &record) ||
                (s->self == 51 ? record.verb > 2 : record.verb < 3))
            {
                return E7_REJECTED;
            }
            if (s->self == 51)
            {
                if (edu_application(&record, &value, &reply) ||
                    edu_record_encode(&reply, response, &z))
                {
                    return E7_REJECTED;
                }
            }
            if (z > cap && ES_MUTANT != 4)
            {
                return E7_FULL;
            }
            if ((s->self == 51 && s->effects == UINT_MAX) ||
                (s->self == 17 && s->responses == UINT_MAX))
            {
                return E7_FULL;
            }
            r = e7_session_commit(&s->core, token, 1, response, z);
            if (r)
            {
                return r;
            }
            if (s->self == 51)
            {
                s->setpoint = value;
                if (record.verb == 2 && reply.verb != 4)
                {
                    s->effects++;
                }
            }
            else
            {
                s->response = record;
                s->responses++;
            }
            if (z)
            {
                memcpy(e.reply, response, z);
            }
            e.length = z;
        }
        else if (kind == 5)
        {
            if (cp || len != 1 || body[0] != s->self || s->phase != ES_READY)
            {
                return E7_REJECTED;
            }
            int r = e7_token_begin(&s->core, s->peer, s->self);
            if (r)
            {
                return r;
            }
            r = e7_token_complete(&s->core, s->self, 1);
            if (r)
            {
                return r;
            }
            s->last_control = s->rx.inner.completed;
        }
        else if (kind == 4)
        {
            if (len || s->phase != ES_READY)
            {
                return E7_REJECTED;
            }
            int r = e7_session_resume(&s->core, sid,
                                      ES_MUTANT == 7 ? s->core.epoch : epoch,
                                      s->peer, cp);
            if (r)
            {
                return r;
            }
        }
        else if (kind == 6)
        {
            if (cp || len)
            {
                return E7_REJECTED;
            }
            int r = e7_session_close(&s->core);
            if (r)
            {
                return r;
            }
            s->phase = ES_CLOSED;
        }
        else if (kind == 7)
        {
            if (cp || len)
            {
                return E7_REJECTED;
            }
            s->uncertain = 1;
        }
        else
        {
            return E7_REJECTED;
        }
    }
    *event = e;
    return ES_DELIVERED;
}

int es_deliver(struct es_peer *s, size_t cap, struct es_event *out)
{
    if (!s || !out || cap > 80)
    {
        return E7_INVALID;
    }
    uint8_t wire[120];
    size_t n;
    int r = et_peek(&s->rx, wire, sizeof wire, &n);
    if (r)
    {
        return r;
    }
    struct es_peer trial = *s;
    struct es_event event;
    r = process(&trial, wire, n, cap, &event);
    if (r == E7_FULL || r == E7_BUSY)
    {
        return r;
    }
    if (r != ES_DELIVERED && r != ES_DUPLICATE)
    {
        if (ES_MUTANT == 3)
        {
            *s = trial;
        }
        if (n >= 7 && get16(wire + 1) && get16(wire + 3) && wire[0] != 7)
        {
            s->error_session = (uint16_t)get16(wire + 1);
            s->error_epoch = (uint16_t)get16(wire + 3);
        }
        (void)et_release(&s->rx, s->rx.identity.epoch, s->rx.inner.completed);
        return r;
    }
    if (et_release(&trial.rx, trial.rx.identity.epoch,
                   trial.rx.inner.completed))
    {
        return E7_INVALID;
    }
    *s = trial;
    *out = event;
    return r;
}

int es_uncertain(struct es_peer *s, uint16_t incarnation, uint16_t transfer)
{
    if (!s)
    {
        return E7_INVALID;
    }
    if (incarnation != s->tx.identity.epoch || !s->outgoing_kind ||
        transfer != s->outgoing_id)
    {
        return E7_STALE;
    }
    s->uncertain = 1;
    if (ES_MUTANT == 5)
    {
        s->core.token_pending = 0;
        s->outgoing_kind = 0;
        s->uncertain = 0;
    }
    return E7_BUSY;
}
