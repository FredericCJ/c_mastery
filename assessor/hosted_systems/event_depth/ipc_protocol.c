#include "ipc.h"
#include "signals.h"
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

static struct timespec deadline(void)
{
    struct timespec t;
    clock_gettime(CLOCK_MONOTONIC, &t);
    t.tv_sec += 3;
    return t;
}

static int receive(int fd, void *buffer, size_t bytes)
{
    unsigned char *p = buffer;
    struct timespec end = deadline();
    while (bytes)
    {
        ssize_t n = read(fd, p, bytes);
        if (n > 0)
        {
            p += n;
            bytes -= (size_t)n;
            continue;
        }
        if (n == 0)
        {
            return -1;
        }
        if (errno == EINTR)
        {
            continue;
        }
        if (errno != EAGAIN && errno != EWOULDBLOCK)
        {
            return -1;
        }
        if (es_wait_fd(fd, end) <= 0)
        {
            return -1;
        }
    }
    return 0;
}

static int send_byte(int fd, unsigned char byte)
{
    return write(fd, &byte, 1) == 1 ? 0 : -1;
}

static int server(int request, int commands, int replies, int witness,
                  unsigned mode)
{
    alarm(6);
    unsigned char command, input[8], response[4];
    if (mode == 0)
    { /* Purge is confirmed before the acceptance gate opens. */
        if (receive(commands, &command, 1) || command != 'P')
        {
            return 2;
        }
        close(request);
        if (send_byte(replies, 'P'))
        {
            return 3;
        }
        if (send_byte(witness, 0))
        {
            return 4;
        }
        return 0;
    }
    if (receive(request, input, 8))
    {
        return 5;
    }
    if (send_byte(replies, 'A'))
    {
        return 6;
    }
    if (mode == 2)
    {
        if (receive(commands, &command, 1) || command != 'C')
        {
            return 7;
        }
        if (send_byte(replies, 'K'))
        {
            return 8; /* request seen; original work still live */
        }
    }
    if (receive(commands, &command, 1) || command != 'G')
    {
        return 9;
    }
    ei_reply(input,
             response); /* harmless synthetic effect; not an external action */
    if (send_byte(witness, 1) || write(witness, response, 4) != 4)
    {
        return 10;
    }
    /* Last access to the original request has occurred before S is published. */
    if (mode == 2 && send_byte(replies, 'S'))
    {
        return 11;
    }
    close(request);
    return 0;
}

#define CHECK(c)                                                       \
    do                                                                 \
    {                                                                  \
        if (!(c))                                                      \
        {                                                              \
            fprintf(stderr, "IPC-PROTOCOL-CHECK:%s:%d errno=%d\n", #c, \
                    __LINE__, errno);                                  \
            return 1;                                                  \
        }                                                              \
    } while (0)

int main(int argc, char **argv)
{
    CHECK(argc == 2);
    unsigned mode = (unsigned)strtoul(argv[1], 0, 10);
    CHECK(mode <= 2);
    alarm(10);
    int request[2], commands[2], replies[2], witness[2];
    CHECK(pipe2(request, O_NONBLOCK | O_CLOEXEC) == 0 &&
          pipe2(commands, O_NONBLOCK | O_CLOEXEC) == 0 &&
          pipe2(replies, O_NONBLOCK | O_CLOEXEC) == 0 &&
          pipe2(witness, O_NONBLOCK | O_CLOEXEC) == 0);
    pid_t child = fork();
    CHECK(child >= 0);
    if (!child)
    {
        close(request[1]);
        close(commands[1]);
        close(replies[0]);
        close(witness[0]);
        int r = server(request[0], commands[0], replies[1], witness[1], mode);
        close(commands[0]);
        close(replies[1]);
        close(witness[1]);
        _exit(r);
    }
    close(request[0]);
    close(commands[0]);
    close(replies[1]);
    close(witness[1]);
    unsigned char data[8] = {0, 1, 2, 3, 128, 254, 255, 0};
    struct ei_request ledger;
    CHECK(ei_init(&ledger, data, 1, 1) == 0 &&
          ei_transition(&ledger, EI_QUEUE) == 0);
    CHECK(write(request[1], data, 8) == 8);
    unsigned char reply;
    if (mode == 0)
    {
        CHECK(send_byte(commands[1], 'P') == 0 &&
              receive(replies[0], &reply, 1) == 0 && reply == 'P');
        CHECK(ei_transition(&ledger, EI_DISCONNECT) == 0 &&
              ledger.outcome == EI_DEFINITELY_NOT_DONE &&
              ledger.owner == EI_NO_OWNER);
    }
    else
    {
        CHECK(receive(replies[0], &reply, 1) == 0 && reply == 'A');
        CHECK(ei_transition(&ledger, EI_ACCEPT) == 0);
        if (mode == 1)
        {
            /* Production reply channel is gone; a separate assessor witness records
             * the effect, but is explicitly unavailable to the protocol client. */
            close(replies[0]);
            replies[0] = -1;
            CHECK(ei_transition(&ledger, EI_DISCONNECT) == 0 &&
                  ledger.owner == EI_SERVER && ledger.outcome == EI_UNCERTAIN);
        }
        else
        {
            CHECK(send_byte(commands[1], 'C') == 0 &&
                  ei_transition(&ledger, EI_CANCEL_REQUEST) == 0);
            CHECK(receive(replies[0], &reply, 1) == 0 && reply == 'K');
            CHECK(ei_transition(&ledger, EI_CANCEL_SEEN) == 0 &&
                  ledger.owner == EI_SERVER && ledger.state == EI_ACCEPTED);
            /* Work may still commit after K. Do not release its logical request yet.
             */
        }
        CHECK(send_byte(commands[1], 'G') == 0);
    }
    close(request[1]);
    unsigned char effect;
    CHECK(receive(witness[0], &effect, 1) == 0 && effect == (mode != 0));
    if (effect)
    {
        unsigned char result[4];
        CHECK(receive(witness[0], result, 4) == 0 && result[0] == 0 &&
              result[1] == 0 && result[2] == 2 && result[3] == 131);
        CHECK(ei_transition(&ledger, EI_EFFECT) == 0);
        if (mode == 2)
        {
            CHECK(receive(replies[0], &reply, 1) == 0 && reply == 'S');
            CHECK(ei_transition(&ledger, EI_STOP_CONFIRMED) == 0 &&
                  ledger.owner == EI_NO_OWNER);
        }
        else
        {
            CHECK(ei_transition(&ledger, EI_COMPLETE) == 0 &&
                  ledger.owner == EI_NO_OWNER);
        }
    }
    CHECK(ei_invariant(&ledger));
    close(commands[1]);
    if (replies[0] >= 0)
    {
        close(replies[0]);
    }
    close(witness[0]);
    int status;
    CHECK(waitpid(child, &status, 0) == child && WIFEXITED(status) &&
          WEXITSTATUS(status) == 0);
    printf("IPC-PROTOCOL:mode=%u;effect=%u;outcome=%u;owner=%u;cancel_seen_is_"
           "quiescence=0;observer_not_client_knowledge=1;\n",
           mode, effect, (unsigned)ledger.outcome, (unsigned)ledger.owner);
    return 0;
}
