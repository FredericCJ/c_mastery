#include "service_io.h"
#include "hosted.h"
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <poll.h>
#include <sys/socket.h>
#include <time.h>
#include <unistd.h>
_Static_assert(CHAR_BIT == 8 && INT_MAX == INT32_MAX && INT_MIN == INT32_MIN,
               "wire uses octets and signed32");

static uint64_t get64(const unsigned char *p)
{
    uint64_t v = 0;
    for (unsigned i = 0; i < 8; ++i)
    {
        v = v * 256 + p[i];
    }
    return v;
}

static uint32_t get32(const unsigned char *p)
{
    uint32_t v = 0;
    for (unsigned i = 0; i < 4; ++i)
    {
        v = v * 256 + p[i];
    }
    return v;
}

static void put64(unsigned char *p, uint64_t v)
{
    for (unsigned i = 8; i; i--)
    {
        p[i - 1] = (unsigned char)v;
        v >>= 8;
    }
}

static void put32(unsigned char *p, uint32_t v)
{
    for (unsigned i = 4; i; i--)
    {
        p[i - 1] = (unsigned char)v;
        v >>= 8;
    }
}

static int from32(uint32_t v)
{
    return v <= INT32_MAX ? (int)v : -1 - (int)(UINT32_MAX - v);
}

static uint32_t to32(int v)
{
    return v >= 0 ? (uint32_t)v : UINT32_MAX - (uint32_t)(-(v + 1));
}

int rzv_request_encode(struct rzj_request r, unsigned char out[RZV_FRAME_BYTES])
{
    if (!out || r.client > 1 || !r.id || r.id > 15 || !r.fence ||
        r.delta < -8 || r.delta > 8)
    {
        return RZ_INVALID;
    }
    unsigned char b[RZV_FRAME_BYTES] = {0};
    b[1] = 16;
    b[2] = 1;
    b[3] = (unsigned char)r.client;
    b[4] = (unsigned char)r.id;
    b[5] = (unsigned char)(r.delta + 8);
    put64(b + 6, r.fence);
    for (unsigned i = 0; i < RZV_FRAME_BYTES; ++i)
    {
        out[i] = b[i];
    }
    return RZ_OK;
}

void rzv_frame_init(struct rzv_frame *f)
{
    if (f)
    {
        *f = (struct rzv_frame){0};
    }
}

int rzv_frame_feed(struct rzv_frame *f, const unsigned char *bytes, size_t size,
                   size_t *consumed, struct rzj_request *out)
{
    if (!f || !consumed || !out || (!bytes && size))
    {
        return RZ_INVALID;
    }
    *consumed = 0;
    if (f->malformed)
    {
        return RZJ_CORRUPT;
    }
    if (f->used == RZV_FRAME_BYTES)
    {
        return RZ_BUSY;
    }
    while (*consumed < size && f->used < RZV_FRAME_BYTES)
    {
        f->bytes[f->used++] = bytes[(*consumed)++];
        if (f->used == 2 && (f->bytes[0] || f->bytes[1] != 16))
        {
            f->malformed = true;
            return RZJ_CORRUPT;
        }
    }
    if (f->used < RZV_FRAME_BYTES)
    {
        return RZ_PENDING;
    }
    const unsigned char *b = f->bytes;
    bool bad = b[2] != 1 || b[3] > 1 || !b[4] || b[4] > 15 || b[5] > 16 ||
               !get64(b + 6);
#if RZF_MUTANT != 101
    for (unsigned i = 14; i < RZV_FRAME_BYTES; ++i)
    {
        bad = bad || b[i] != 0;
    }
#endif
    if (bad)
    {
        f->malformed = true;
        return RZJ_CORRUPT;
    }
    *out = (struct rzj_request){b[3], b[4], get64(b + 6), (int)b[5] - 8};
    return RZ_OK;
}

int rzv_frame_end(struct rzv_frame *f)
{
    if (!f)
    {
        return RZ_INVALID;
    }
    if (f->malformed)
    {
        return RZJ_CORRUPT;
    }
    if (!f->used)
    {
        return RZ_CLOSED;
    }
    if (f->used == RZV_FRAME_BYTES)
    {
        return RZ_DONE;
    }
    f->malformed = true;
    return RZJ_CORRUPT;
}

int rzv_reply_encode(struct rzv_reply reply, unsigned char b[RZV_FRAME_BYTES])
{
    if (!b || reply.status > RZJ_CORRUPT ||
        (reply.status && (reply.duplicate || reply.result)))
    {
        return RZ_INVALID;
    }
    for (unsigned i = 0; i < RZV_FRAME_BYTES; ++i)
    {
        b[i] = 0;
    }
    b[1] = 16;
    b[2] = 1;
    b[3] = (unsigned char)reply.status;
    b[4] = (unsigned char)reply.duplicate;
    put32(b + 6, to32(reply.result));
    put64(b + 10, reply.epoch);
    return RZ_OK;
}

int rzv_reply_decode(const unsigned char b[RZV_FRAME_BYTES],
                     struct rzv_reply *out)
{
    if (!b || !out)
    {
        return RZ_INVALID;
    }
    if (b[0] || b[1] != 16 || b[2] != 1 || b[3] > RZJ_CORRUPT || b[4] > 1 ||
        b[5] || (b[3] && (b[4] || get32(b + 6))))
    {
        return RZJ_CORRUPT;
    }
    *out = (struct rzv_reply){b[3], b[4] != 0, from32(get32(b + 6)),
                              get64(b + 10)};
    return RZ_OK;
}

int rzv_now(uint64_t *out)
{
    if (!out)
    {
        return RZ_INVALID;
    }
    struct timespec t;
    if (clock_gettime(CLOCK_MONOTONIC, &t) || t.tv_sec < 0 ||
        (uint64_t)t.tv_sec > (uint64_t)LLONG_MAX / 1000000000)
    {
        return RZ_RANGE;
    }
    *out = (uint64_t)t.tv_sec * 1000 + (unsigned long)t.tv_nsec / 1000000;
    return RZ_OK;
}

int rzv_init(struct rzv_service *s, struct rzs_outbox *outbox,
             unsigned capacity, uint64_t now)
{
    if (!s || !outbox || !capacity)
    {
        return RZ_INVALID;
    }
    struct rzf_bucket bucket;
    int status = rzf_bucket_init(&bucket, capacity, 1, 65535, now);
    if (status != RZ_OK)
    {
        return status;
    }
    *s = (struct rzv_service){.outbox = outbox, .limiter = bucket};
    if (pthread_mutex_init(&s->mutex, NULL))
    {
        return RZ_FULL;
    }
    (void)rzb_init(&s->gate, 2, 2, (struct rzb_policy){8, 8, 1, 1}, now);
    return RZ_OK;
}

static int wait_write(int fd, int cancel, const struct rz_scope *scope,
                      uint64_t *now)
{
    for (;;)
    {
        int status = rzv_now(now);
        if (status != RZ_OK)
        {
            return status;
        }
        status = rz_check(scope, *now);
        if (status != RZ_OK)
        {
            return status;
        }
        uint64_t left = scope->deadline - *now;
        int timeout = left > INT_MAX ? INT_MAX : (int)left;
        struct pollfd p[2] = {{fd, POLLOUT, 0}, {cancel, POLLIN, 0}};
        int ready = poll(p, 2, timeout);
        if (ready < 0 && errno == EINTR)
        {
            continue;
        }
        if (ready < 0)
        {
            return RZ_CLOSED;
        }
        if (!ready)
        {
            continue;
        }
        if (p[1].revents & (POLLIN | POLLHUP))
        {
            return RZ_CANCELLED;
        }
        if (p[1].revents & (POLLERR | POLLNVAL))
        {
            return RZ_CLOSED;
        }
        if (p[0].revents & POLLOUT)
        {
            return RZ_OK;
        }
        if (p[0].revents & (POLLHUP | POLLERR | POLLNVAL))
        {
            return RZ_CLOSED;
        }
    }
}

int rzv_serve(struct rzv_service *s, struct rzf_scope *scope, int fd,
              int cancel, size_t chunk, struct rzv_observation *out)
{
    if (!s || !scope || !out || fd < 0 || cancel < 0 || fd == cancel ||
        !chunk || chunk > RZV_FRAME_BYTES ||
        scope->core.deadline > (uint64_t)LLONG_MAX / 1000000)
    {
        return RZ_INVALID;
    }
    int flags = fcntl(fd, F_GETFL);
    if (flags < 0 || !(flags & O_NONBLOCK))
    {
        return RZ_INVALID;
    }
    struct rzf_borrow borrower = {0};
    int status = rzf_borrow_begin(&borrower, scope);
    if (status != RZ_OK)
    {
        return status;
    }
    (void)pthread_mutex_lock(&s->mutex);
    if (s->closed || s->active == UINT_MAX)
    {
        int rejected = s->closed ? RZ_CLOSED : RZ_FULL;
        (void)pthread_mutex_unlock(&s->mutex);
        (void)rzf_borrow_ack(&borrower);
        return rejected;
    }
    ++s->active;
    (void)pthread_mutex_unlock(&s->mutex);
    struct rzv_observation observed = {.admission = RZ_PENDING,
                                       .effect = RZ_PENDING};
    struct rzv_frame frame = {0};
    struct rzj_request request = {0};
    struct rzb_permit permit = {0};
    unsigned char bytes[RZV_FRAME_BYTES];
    uint64_t now = 0, epoch = 0;
    bool admitted = false;
    int outcome = -1;
    for (;;)
    {
        status = rzv_now(&now);
        if (status != RZ_OK)
        {
            goto finish;
        }
        status = rz_check(&scope->core, now);
        if (status != RZ_OK)
        {
            goto finish;
        }
        size_t length = RZV_FRAME_BYTES - frame.used;
        if (length > chunk)
        {
            length = chunk;
        }
        ssize_t n = hs_read_until(fd, bytes, length, cancel,
                                  (long long)(scope->core.deadline * 1000000));
        if (n <= 0)
        {
            status = n == -2   ? RZ_EXPIRED
                     : n == -3 ? RZ_CANCELLED
                     : n < 0   ? RZ_CLOSED
                               : rzv_frame_end(&frame);
            goto finish;
        }
        size_t consumed = 0;
        status = rzv_frame_feed(&frame, bytes, (size_t)n, &consumed, &request);
        observed.read += consumed;
        if (status == RZ_OK)
        {
            break;
        }
        if (status != RZ_PENDING)
        {
            goto finish;
        }
    }
    (void)pthread_mutex_lock(&s->mutex);
    status = rzv_now(&now);
    if (status == RZ_OK)
    {
        status = rz_check(&scope->core, now);
    }
    if (status == RZ_OK && s->closed)
    {
        status = RZ_CLOSED;
    }
    struct rzf_bucket staged = s->limiter;
    if (status == RZ_OK)
    {
        status = rzf_bucket_consume(&staged, now, 1);
    }
    if (status == RZ_OK)
    {
        status = rzb_admit(&s->gate, request.client, 1, scope, now, &permit);
    }
    observed.admission = status;
    epoch = s->outbox->epoch;
    if (status == RZ_OK)
    {
        admitted = true;
        s->limiter = staged;
        observed.charged = true;
        observed.effect = rzs_apply(s->outbox, now / 1000, request,
                                    &observed.result, &observed.duplicate);
        observed.committed = observed.effect == RZ_OK && !observed.duplicate;
        outcome = observed.effect == RZ_OK ? 1 : -1;
        status = observed.effect;
    }
#if RZF_MUTANT == 102
    if (admitted)
    {
        (void)rzb_complete(&s->gate, permit, now, outcome);
        admitted = false;
    }
#endif
    (void)pthread_mutex_unlock(&s->mutex);
    (void)rzv_reply_encode(
        (struct rzv_reply){(unsigned)status,
                           status == RZ_OK && observed.duplicate,
                           status == RZ_OK ? observed.result : 0, epoch},
        bytes);
    while (observed.written < RZV_FRAME_BYTES)
    {
        status = wait_write(fd, cancel, &scope->core, &now);
        if (status != RZ_OK)
        {
            goto finish;
        }
        size_t length = RZV_FRAME_BYTES - observed.written;
        if (length > chunk)
        {
            length = chunk;
        }
        ssize_t n = send(fd, bytes + observed.written, length, MSG_NOSIGNAL);
        if (n < 0 &&
            (errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR))
        {
            continue;
        }
        if (n <= 0)
        {
            status = RZ_CLOSED;
            goto finish;
        }
        observed.written += (size_t)n;
    }
    status = RZ_OK;
finish:
    (void)pthread_mutex_lock(&s->mutex);
    if (admitted)
    {
        if (rzv_now(&now) != RZ_OK || now < s->gate.last)
        {
            now = s->gate.last;
        }
        (void)rzb_complete(&s->gate, permit, now, outcome);
    }
    --s->active;
    (void)pthread_mutex_unlock(&s->mutex);
    (void)rzf_borrow_ack(&borrower);
    observed.cleaned = true;
    *out = observed;
    return status;
}

int rzv_notify(struct rzv_service *s, unsigned operation, unsigned sequence,
               struct rzs_event *out)
{
    if (!s || operation > 2 || (operation == 1 && !out))
    {
        return RZ_INVALID;
    }
    (void)pthread_mutex_lock(&s->mutex);
    int status = operation == 0   ? rzs_forward(s->outbox)
                 : operation == 1 ? rzs_deliver(s->outbox, out)
                                  : rzs_ack(s->outbox, sequence);
    (void)pthread_mutex_unlock(&s->mutex);
    return status;
}

int rzv_close(struct rzv_service *s)
{
    if (!s)
    {
        return RZ_INVALID;
    }
    (void)pthread_mutex_lock(&s->mutex);
    s->closed = true;
    int status = rzb_close(&s->gate);
    (void)pthread_mutex_unlock(&s->mutex);
    return status;
}

int rzv_destroy(struct rzv_service *s)
{
    if (!s)
    {
        return RZ_INVALID;
    }
    (void)pthread_mutex_lock(&s->mutex);
    if (s->active || rzb_quiescent(&s->gate) != RZ_OK)
    {
        (void)pthread_mutex_unlock(&s->mutex);
        return RZ_BUSY;
    }
    (void)pthread_mutex_unlock(&s->mutex);
    return pthread_mutex_destroy(&s->mutex) ? RZ_BUSY : RZ_OK;
}
