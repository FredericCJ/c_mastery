#define _POSIX_C_SOURCE 200809L
#include "pmu_control.h"

#include "cm_parse.h"
#include <errno.h>
#include <limits.h>
#include <poll.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/* A descriptor number from the environment: plain ASCII decimal in
 * 0..INT_MAX (cm_parse.h), so "", " 3", "+3" or "3x" never name a
 * descriptor. Returns -1 with errno EINVAL otherwise. */
static int descriptor(const char *name)
{
    unsigned value = 0;

    if (cm_parse_unsigned(getenv(name), 0, INT_MAX, &value) != CM_PARSE_OK)
    {
        errno = EINVAL;
        return -1;
    }
    return (int)value; /* value <= INT_MAX */
}

enum
{
    CONTROL_ATTEMPTS = 8, /* bounded EINTR and short-transfer retries */
    ACK_TIMEOUT_MS = 1000
};

/* Write the whole command, retrying EINTR and short writes a bounded number
 * of times. Returns 0, or -1 with errno set (EIO for a zero-length write,
 * EINTR when the retries ran out). */
static int send_command(int control, const char *command)
{
    size_t length = strlen(command);
    size_t sent = 0;

    for (unsigned attempt = 0; (sent < length) && (attempt < CONTROL_ATTEMPTS);
         ++attempt)
    {
        ssize_t result = write(control, command + sent, length - sent);

        if ((result < 0) && (errno == EINTR))
        {
            continue;
        }
        if (result < 0)
        {
            return -1;
        }
        if (result == 0)
        {
            errno = EIO;
            return -1;
        }
        sent += (size_t)result;
    }
    if (sent != length)
    {
        errno = EINTR;
        return -1;
    }
    return 0;
}

/* Wait (bounded) for one more chunk of the acknowledgement. Returns the
 * bytes read, 0 to retry after EINTR, or -1 with errno set (ETIMEDOUT when
 * nothing arrives, EPIPE at end of file). */
static ssize_t read_ack_chunk(int ack, char *buffer, size_t size)
{
    struct pollfd waiter = {ack, POLLIN, 0};
    int ready = poll(&waiter, 1, ACK_TIMEOUT_MS);

    if (ready < 0)
    {
        return (errno == EINTR) ? 0 : -1;
    }
    if (ready == 0)
    {
        errno = ETIMEDOUT;
        return -1;
    }
    ssize_t got = read(ack, buffer, size);
    if (got < 0)
    {
        return (errno == EINTR) ? 0 : -1;
    }
    if (got == 0)
    {
        errno = EPIPE;
        return -1;
    }
    return got;
}

/* perf's evlist__ctlfd_ack writes sizeof(EVLIST_CTL_CMD_ACK_TAG), including
 * the terminating NUL. Consume all of it before the next command. Returns 0,
 * or -1 with errno set (EPROTO for any other reply). */
static int await_ack(int ack)
{
    char answer[sizeof "ack\n"];
    size_t used = 0;

    for (unsigned attempt = 0;
         (used < sizeof answer) && (attempt < CONTROL_ATTEMPTS); ++attempt)
    {
        ssize_t got = read_ack_chunk(ack, answer + used, sizeof answer - used);

        if (got < 0)
        {
            return -1;
        }
        used += (size_t)got;
    }
    if ((used != sizeof answer) ||
        (memcmp(answer, "ack\n", sizeof answer) != 0))
    {
        errno = EPROTO;
        return -1;
    }
    return 0;
}

int pf_perf_control(int enable)
{
    int control = descriptor("PF_PERF_CONTROL_FD");
    if (control < 0)
    {
        return -1;
    }
    int ack = descriptor("PF_PERF_ACK_FD");
    if (ack < 0)
    {
        return -1;
    }
    if (send_command(control, enable ? "enable\n" : "disable\n") != 0)
    {
        return -1;
    }
    return await_ack(ack);
}
