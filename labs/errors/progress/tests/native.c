#define _POSIX_C_SOURCE 200809L
#include "progress.h"
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#define CHECK(x)                                                             \
    do                                                                       \
    {                                                                        \
        if (!(x))                                                            \
        {                                                                    \
            fprintf(stderr, "ER_CHECK EPT_NATIVE:%d:%s errno%d\n", __LINE__, \
                    #x, errno);                                              \
            return 1;                                                        \
        }                                                                    \
    } while (0)

struct er_port
{
    int fd;
    size_t quota, accepted, durable;
    uint32_t epoch;
    unsigned writes, syncs, mode;
};

static er_progress write_port(void *context, const unsigned char *bytes,
                              size_t n)
{
    struct er_port *p = context;
    ++p->writes;
    if (n > p->quota)
    {
        n = p->quota;
    }
    ssize_t count = write(p->fd, bytes, n);
    er_progress result = {0, ER_IO};
    if (count < 0)
    {
        if (errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR)
        {
            result.code = ER_AGAIN;
        }
        return result;
    }
    result.next = (size_t)count;
    result.code = count ? ER_OK : ER_AGAIN;
    p->accepted += (size_t)count;
    return result;
}

static struct ept_flush_result sync_port(void *context, size_t accepted,
                                         size_t prior, uint32_t epoch)
{
    struct er_port *p = context;
    struct ept_flush_result r = {prior, epoch, EPT_FLUSH_IO};
    if (accepted != p->accepted || prior != p->durable || epoch != p->epoch)
    {
        r.code = EPT_FLUSH_UNKNOWN;
        return r;
    }
    if (p->mode == 1)
    {
        return r; /* Injected pre-call refusal, not an actual fsync error. */
    }
    ++p->syncs;
    if (fsync(p->fd) < 0)
    {
        r.code = EPT_FLUSH_UNKNOWN;
        return r;
    }
    p->durable = accepted;
    ++p->epoch;
    if (p->mode == 2)
    {
        r.code = EPT_FLUSH_UNKNOWN;
        return r;
    } /* Lost reply after known sync. */
    r = (struct ept_flush_result){p->durable, p->epoch,
                                  p->mode == 3 ? EPT_FLUSH_IO : EPT_FLUSH_OK};
    return r;
}

static int pipe_case(void)
{
    int fd[2];
    CHECK(pipe(fd) == 0);
    CHECK(fcntl(fd[1], F_SETFL, O_NONBLOCK) == 0 &&
          fcntl(fd[0], F_SETFL, O_NONBLOCK) == 0);
    unsigned char fill[4096] = {0};
    unsigned rounds = 0;
    ssize_t count;
    while ((count = write(fd[1], fill, sizeof fill)) > 0)
    {
        CHECK(++rounds < 4096);
    }
    CHECK(count < 0 && (errno == EAGAIN || errno == EWOULDBLOCK));
    struct ept_stream stream = {0};
    struct ept_handle handle = {0};
    struct ept_observation o;
    struct er_port p = {.fd = fd[1], .quota = 2};
    unsigned char source[3] = {1, 2, 3};
    ept_init(&stream);
    CHECK(ept_start(&stream, source, 3, &handle) == EPT_OK);
    source[0] = 99;
    CHECK(ept_step(handle, write_port, &p, &o) == EPT_AGAIN &&
          o.accepted == 0 && p.writes == 1);
    while ((count = read(fd[0], fill, sizeof fill)) > 0)
    {
    }
    CHECK(count < 0 && (errno == EAGAIN || errno == EWOULDBLOCK));
    CHECK(ept_step(handle, write_port, &p, &o) == EPT_AGAIN &&
          o.accepted == 2 && o.durable == 0 && p.writes == 2);
    CHECK(ept_cancel(handle, &o) == EPT_CANCELLED);
    CHECK(ept_step(handle, write_port, &p, &o) == EPT_CANCELLED &&
          p.writes == 2);
    CHECK(ept_release(handle, 1, 0) == EPT_BUSY);
    CHECK(ept_release(handle, 1, 1) == EPT_OK);
    CHECK(close(fd[1]) == 0);
    CHECK(read(fd[0], fill, sizeof fill) == 2 && fill[0] == 3 && fill[1] == 1);
    CHECK(read(fd[0], fill, sizeof fill) == 0);
    CHECK(close(fd[0]) == 0);
    return 0;
}

static int file_case(void)
{
    char path[] = "/tmp/cmastery-ept-XXXXXX";
    int fd = mkstemp(path);
    CHECK(fd >= 0);
    CHECK(unlink(path) == 0);
    struct ept_stream stream = {0};
    struct ept_handle handle = {0};
    struct ept_observation o;
    struct er_port p = {.fd = fd, .quota = 2, .mode = 3};
    unsigned char source[3] = {1, 2, 3};
    ept_init(&stream);
    CHECK(ept_start(&stream, source, 3, &handle) == EPT_OK);
    CHECK(ept_step(handle, write_port, &p, &o) == EPT_AGAIN &&
          o.accepted == 2 && o.durable == 0);
    CHECK(ept_checkpoint(handle, sync_port, &p, &o) == EPT_IO &&
          o.durable == 2 && o.epoch == 1 && p.syncs == 1);
    p.quota = 3;
    CHECK(ept_step(handle, write_port, &p, &o) == EPT_DONE && o.accepted == 5 &&
          o.durable == 2);
    p.mode = 1;
    CHECK(ept_checkpoint(handle, sync_port, &p, &o) == EPT_IO &&
          o.durable == 2 && p.syncs == 1);
    p.mode = 2;
    CHECK(ept_checkpoint(handle, sync_port, &p, &o) == EPT_UNCERTAIN &&
          o.durable == 2 && o.epoch == 1 && p.syncs == 2 && p.durable == 5 &&
          p.epoch == 2);
    CHECK(ept_step(handle, write_port, &p, &o) == EPT_UNCERTAIN &&
          p.writes == 2);
    CHECK(ept_reconcile(handle, p.durable, p.epoch, 1, &o) == EPT_OK &&
          o.durable == 5 && o.epoch == 2);
    unsigned char actual[6] = {0}, expected[5] = {3, 1, 2, 3, 6};
    CHECK(lseek(fd, 0, SEEK_SET) == 0);
    CHECK(read(fd, actual, sizeof actual) == 5 &&
          memcmp(actual, expected, 5) == 0);
    CHECK(read(fd, actual, sizeof actual) == 0);
    CHECK(ept_release(handle, 1, 0) == EPT_OK);
    CHECK(close(fd) == 0);
    return 0;
}

int main(void)
{
    CHECK(pipe_case() == 0);
    CHECK(file_case() == 0);
    puts(
        "EPT_NATIVE actual-pipe-EAGAIN quota-two cancellation-prefix actual-fsync-two-epochs lost-reply no-power-loss-proof");
    return 0;
}
