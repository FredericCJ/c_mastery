/*
 * Progress, error-record and backend checks for the span writer (writer.h).
 *
 * Hosted Linux x86-64 only: it uses nonblocking pipes, poll(2) and a raw
 * write(2) syscall adapter. Every failed CHECK prints "CWF_PROGRESS:<expr>" on
 * stderr and exits 41; success prints CWF_PROGRESS_PASS.
 *
 * Compile with -DCWF_LOG to also publish a struct cwf_error copy through the
 * optional reporter; the core result must be identical with it compiled out.
 */
#define _POSIX_C_SOURCE 200809L
#include "writer.h"
#include "adapters.h"
#include <errno.h>
#include <fcntl.h>
#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#define CHECK(x)                                      \
    do                                                \
    {                                                 \
        if (!(x))                                     \
        {                                             \
            fprintf(stderr, "CWF_PROGRESS:%s\n", #x); \
            exit(41);                                 \
        }                                             \
    } while (0)

enum progress_limit
{
    SCRIPT_CALLS = 8,
    SCRIPT_OUTPUT_BYTES = 16,
    PAYLOAD_BYTES = 9,
    PIPE_WRITE_LIMIT_BYTES = 3,
    PIPE_FILL_CHUNK_BYTES = 4096,
    PIPE_FILL_MAX_WRITES = 4096,
    PIPE_CYCLES = 3,
    POLL_READY_TIMEOUT_MS = 1000
};

/*
 * Scripted writer: results[at] is what call number `at` returns. Records the
 * remaining length and the offset into `base` of every call, and copies the
 * accepted bytes to out.
 */
struct cwf_script
{
    long results[SCRIPT_CALLS];
    size_t at;
    size_t remaining[SCRIPT_CALLS];
    size_t offset[SCRIPT_CALLS];
    size_t used;
    unsigned char out[SCRIPT_OUTPUT_BYTES];
    const unsigned char *base;
};

static struct cpw_result scripted(void *context, const unsigned char *p,
                                  size_t n)
{
    struct cwf_script *s = context;
    CHECK(s->at < SCRIPT_CALLS);
    const size_t i = s->at++;
    s->remaining[i] = n;
    s->offset[i] = (size_t)(p - s->base);
    const struct cpw_result r = cpw_raw(s->results[i]);
    if (r.kind == CPW_PROGRESS)
    {
        CHECK(r.accepted <= n && r.accepted <= sizeof s->out - s->used);
        memcpy(s->out + s->used, p, r.accepted);
        s->used += r.accepted;
    }
    return r;
}

/* Raw Linux x86-64 write(2); returns the byte count or a negative errno. */
static long raw_write(int fd, const unsigned char *p, size_t n)
{
#if defined(__x86_64__) && defined(__linux__)
    register long result __asm__("rax") = 1;
    __asm__ volatile("syscall"
                     : "+a"(result)
                     : "D"((long)fd), "S"(p), "d"(n)
                     : "rcx", "r11", "memory");
    return result;
#else
#error Selected raw adapter requires Linux x86-64
#endif
}

/*
 * Descriptor-backed writer. Each call offers at most PIPE_WRITE_LIMIT_BYTES so
 * that three calls are needed for the nine-byte payload.
 */
struct cwf_fd
{
    int fd;
    int raw;
    unsigned calls;
    struct cpw_result last;
};

static struct cpw_result descriptor(void *context, const unsigned char *p,
                                    size_t n)
{
    struct cwf_fd *f = context;
    ++f->calls;
    if (n > PIPE_WRITE_LIMIT_BYTES)
    {
        n = PIPE_WRITE_LIMIT_BYTES;
    }
    if (f->raw)
    {
        f->last = cpw_raw(raw_write(f->fd, p, n));
    }
    else
    {
        /* Capture errno immediately, before any other call can change it. */
        const ssize_t r = write(f->fd, p, n);
        const int saved = r < 0 ? errno : 0;
        f->last = cpw_libc((long)r, saved);
    }
    return f->last;
}

/* Fills a nonblocking pipe until EAGAIN and returns the bytes it accepted. */
static size_t fill(int fd)
{
    unsigned char bytes[PIPE_FILL_CHUNK_BYTES];
    memset(bytes, 0x55, sizeof bytes);
    size_t total = 0;
    for (unsigned turn = 0; turn < PIPE_FILL_MAX_WRITES; ++turn)
    {
        const ssize_t n = write(fd, bytes, sizeof bytes);
        if (n < 0)
        {
            CHECK(errno == EAGAIN);
            return total;
        }
        CHECK(n > 0);
        total += (size_t)n;
    }
    CHECK(0);
    return 0;
}

/* Reads exactly n filler bytes back out of the pipe. */
static void drain(int fd, size_t n)
{
    unsigned char bytes[PIPE_FILL_CHUNK_BYTES];
    while (n)
    {
        const size_t take = n < sizeof bytes ? n : sizeof bytes;
        const ssize_t r = read(fd, bytes, take);
        CHECK(r > 0);
        for (ssize_t i = 0; i < r; ++i)
        {
            CHECK(bytes[i] == 0x55);
        }
        n -= (size_t)r;
    }
}

/*
 * Real nonblocking pipe, three cycles of "full pipe -> EAGAIN retry" then
 * "drained pipe -> 3 bytes accepted", taking exactly one backend callback per
 * step. A closed descriptor must then produce EBADF as a terminal failure.
 */
static void check_pipe_backpressure(int use_raw_syscall)
{
    int fds[2];
    CHECK(pipe(fds) == 0);
    CHECK(fcntl(fds[0], F_SETFL, O_NONBLOCK) == 0 &&
          fcntl(fds[1], F_SETFL, O_NONBLOCK) == 0);
    const unsigned char bytes[] = "ABCDEFGHI";
    unsigned char got[PAYLOAD_BYTES];
    struct cpw_transfer t;
    struct cwf_fd f = {fds[1], use_raw_syscall, 0, {CPW_RETRY, 0, 0}};
    CHECK(cpw_begin(&t, bytes, PAYLOAD_BYTES, 99) == CPW_PENDING);
    for (unsigned cycle = 0; cycle < PIPE_CYCLES; ++cycle)
    {
        const size_t full = fill(fds[1]);
        struct pollfd ready = {fds[1], POLLOUT, 0};
        CHECK(poll(&ready, 1, 0) == 0);
        unsigned before = f.calls;
        CHECK(cpw_step(&t, descriptor, &f) == CPW_PENDING &&
              f.calls == before + 1 && f.last.kind == CPW_RETRY &&
              f.last.cause == EAGAIN && t.accepted == cycle * 3);
        drain(fds[0], full);
        CHECK(poll(&ready, 1, POLL_READY_TIMEOUT_MS) == 1 &&
              (ready.revents & POLLOUT));
        before = f.calls;
        CHECK(cpw_step(&t, descriptor, &f) ==
                  (cycle == 2 ? CPW_DONE : CPW_PENDING) &&
              f.calls == before + 1 && t.accepted == (cycle + 1) * 3);
        CHECK(read(fds[0], got + cycle * 3, 3) == 3);
    }
    CHECK(!memcmp(got, bytes, PAYLOAD_BYTES) && f.calls == 6);
    CHECK(close(fds[1]) == 0);
    f.fd = -1;
    CHECK(cpw_begin(&t, bytes, PAYLOAD_BYTES, 100) == CPW_PENDING &&
          cpw_step(&t, descriptor, &f) == CPW_FAILED &&
          t.error.primary == EBADF && !t.accepted);
    CHECK(close(fds[0]) == 0);
    printf("CWF_OS raw=%d cycles=3 callbacks=7 accepted=9 cause=%d\n",
           use_raw_syscall, EBADF);
}

/* Number of times the optional reporter ran (always 0 without CWF_LOG). */
static unsigned reports;
#ifdef CWF_LOG
static struct cwf_error saved;

static void report(const struct cpw_transfer *t)
{
    CHECK(cwf_error(t, &saved) == 0);
    ++reports;
}
#else
static void report(const struct cpw_transfer *t)
{
    (void)t;
}
#endif

/*
 * Results 3, EINTR (-4), 2, 4 for nine bytes: the retry moves nothing, and the
 * remaining length and offset seen by the writer are 9/0, 6/3, 6/3, 4/5.
 */
static void check_scripted_progress(const unsigned char *bytes)
{
    struct cpw_transfer t;
    struct cwf_script s = {.results = {3, -4, 2, 4}, .base = bytes};
    CHECK(cpw_begin(&t, bytes, 9, 71) == CPW_PENDING);
    for (unsigned i = 0; i < 4; ++i)
    {
        CHECK(cpw_step(&t, scripted, &s) == (i == 3 ? CPW_DONE : CPW_PENDING));
        CHECK(t.accepted == (i < 2 ? 3 : i == 2 ? 5 : 9));
    }
    const size_t remaining[] = {9, 6, 6, 4};
    const size_t offset[] = {0, 3, 3, 5};
    for (unsigned i = 0; i < 4; ++i)
    {
        CHECK(s.remaining[i] == remaining[i] && s.offset[i] == offset[i]);
    }
    CHECK(s.used == 9 && !memcmp(s.out, bytes, 9));
}

/*
 * A zero-progress turn, then a terminal error (EIO, -5) after 3 bytes. The
 * first cause and first cleanup cause are retained, and the failed transfer
 * performs no further callbacks.
 */
static void check_failure_record(const unsigned char *bytes)
{
    struct cpw_transfer t;
    struct cwf_script s = {.results = {3, 0, -5}, .base = bytes};
    CHECK(cpw_begin(&t, bytes, 9, 72) == CPW_PENDING);
    CHECK(cpw_step(&t, scripted, &s) == CPW_PENDING && t.accepted == 3);
    CHECK(cpw_step(&t, scripted, &s) == CPW_PENDING && t.accepted == 3 &&
          s.at == 2);
    CHECK(cpw_step(&t, scripted, &s) == CPW_FAILED && t.accepted == 3 &&
          t.error.primary == 5);
    cpw_cleanup(&t, 9);
    cpw_cleanup(&t, 10);
    report(&t);
    struct cwf_error error;
    CHECK(cwf_error(&t, &error) == 0 && error.category == CWF_IO &&
          error.operation == 72 && error.accepted == 3 && error.primary == 5 &&
          error.cleanup == 9 && s.at == 3);
    CHECK(cpw_step(&t, scripted, &s) == CPW_FAILED && s.at == 3);
}

/*
 * The same integer means different things at each boundary: raw Linux
 * returns -errno, libc returns -1 with errno captured separately, and the
 * teaching callback uses -2 for "retry".
 */
static void check_result_conventions(void)
{
    struct cpw_result raw_error = cpw_raw(-2);
    const struct cpw_result script_retry = cpw_script(-2);
    const struct cpw_result libc_retry = cpw_libc(-1, EINTR);
    CHECK(raw_error.kind == CPW_ERROR && raw_error.cause == 2 &&
          script_retry.kind == CPW_RETRY && script_retry.cause == 1002 &&
          libc_retry.kind == CPW_RETRY && libc_retry.cause == EINTR);
    raw_error = cpw_raw(-EINTR);
    CHECK(raw_error.kind == CPW_RETRY && !raw_error.accepted);
    CHECK(cpw_raw(3).accepted == 3 && cpw_libc(3, 99).accepted == 3 &&
          cpw_script(-1).cause == 1001);
}

/*
 * Runs the core transfer and the adapter product side by side on identical
 * scripts (3, EINTR, 2, EIO). They must agree on offset, first cause and
 * callback count after every step.
 */
static void check_adapter_product(const unsigned char *bytes)
{
    struct cpw_transfer t;
    struct cwf_script left = {.results = {3, -4, 2, -5}, .base = bytes};
    struct cwf_script right = left;
    struct cpx_progress product = {{0, ER_AGAIN}, {77, 0, 0, 0}, 0};
    CHECK(cpw_begin(&t, bytes, 9, 77) == CPW_PENDING);
    for (unsigned i = 0; i < 4; ++i)
    {
        (void)cpx_progress_step(&product, bytes, 9, scripted, &left);
        (void)cpw_step(&t, scripted, &right);
        CHECK(product.progress.next == t.accepted &&
              product.error.primary == t.error.primary && left.at == right.at);
    }
    cpx_progress_cleanup(&product, 9);
    cpw_cleanup(&t, 9);
    CHECK(product.progress.next == 5 && product.progress.code == ER_IO &&
          product.error.primary == 5 && product.error.cleanup == 9 &&
          product.callbacks == 4);
    CHECK(cpx_progress_step(&product, bytes, 9, scripted, &left) == ER_IO &&
          left.at == 4);
    printf(
        "CWF_COST product=%zu result=%zu adapter=%zu core=%zu callbacks=4 adapter_payload_copies=0 log_records=%u\n",
        sizeof(er_progress), sizeof(struct cpw_result),
        sizeof(struct cpx_progress), sizeof(struct cpw_transfer), reports);
}

int main(void)
{
    const unsigned char bytes[] = "ABCDEFGHI";
    check_scripted_progress(bytes);
    check_failure_record(bytes);
    check_result_conventions();
    check_adapter_product(bytes);
    check_pipe_backpressure(1);
    check_pipe_backpressure(0);
    puts("CWF_PROGRESS_PASS");
    return 0;
}
