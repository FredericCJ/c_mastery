#include "ipc.h"
#include "signals.h"
#include <errno.h>
#include <string.h>

int ei_exchange(struct ei_io o, const unsigned char request[8],
                unsigned char response[4], struct timespec deadline,
                struct ei_progress *p)
{
    if (!o.now || !o.transfer || !o.wait || !request || !response || !p ||
        deadline.tv_sec < 0 || deadline.tv_nsec < 0 ||
        deadline.tv_nsec >= 1000000000L || p->sent > 8 || p->received > 4 ||
        (p->received && p->sent != 8))
    {
        errno = EINVAL;
        return -1;
    }
    unsigned char bytes[8];
    memcpy(bytes, request, 8);
    for (;;)
    {
        struct timespec now, left;
        if (o.now(o.context, &now))
            return -1;
#ifdef EI_MUTANT_RESET_BUDGET
        deadline.tv_sec = now.tv_sec + 1;
        deadline.tv_nsec = now.tv_nsec;
#endif
        int available = es_remaining(now, deadline, &left);
        if (available <= 0)
        {
            errno = available < 0 ? EINVAL : ETIMEDOUT;
            return -1;
        }
        if (p->received == 4)
        {
            memcpy(response, p->reply, 4);
            return 0;
        }
        int sending = p->sent < 8;
        size_t remaining = sending ? 8 - p->sent : 4 - p->received;
        unsigned char *cursor =
            sending ? bytes + p->sent : p->reply + p->received;
        ssize_t n = o.transfer(o.context, sending, cursor, remaining);
        if (n > 0)
        {
            if ((size_t)n > remaining)
            {
                errno = EPROTO;
                return -1;
            }
            if (sending)
                p->sent += (size_t)n;
            else
                p->received += (size_t)n;
            continue;
        }
        if (n == 0)
        {
            errno = EPIPE;
            return -1;
        }
        if (errno == EINTR)
            continue;
        if (errno != EAGAIN && errno != EWOULDBLOCK)
            return -1;
        /* Recompute AFTER the failed I/O: the callback may have consumed time. */
        if (o.now(o.context, &now))
            return -1;
        available = es_remaining(now, deadline, &left);
        if (available <= 0)
        {
            errno = available < 0 ? EINVAL : ETIMEDOUT;
            return -1;
        }
        if (o.wait(o.context, sending, left) < 0 && errno != EINTR)
            return -1;
    }
}
