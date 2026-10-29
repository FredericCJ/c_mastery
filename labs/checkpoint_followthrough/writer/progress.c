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

struct cwf_script
{
    long results[8];
    size_t at, remaining[8], offset[8], used;
    unsigned char out[16];
    const unsigned char *base;
};

static struct cpw_result scripted(void *context, const unsigned char *p,
                                  size_t n)
{
    struct cwf_script *s = context;
    CHECK(s->at < 8);
    size_t i = s->at++;
    s->remaining[i] = n;
    s->offset[i] = (size_t)(p - s->base);
    struct cpw_result r = cpw_raw(s->results[i]);
    if (r.kind == CPW_PROGRESS)
    {
        CHECK(r.accepted <= n && r.accepted <= sizeof s->out - s->used);
        memcpy(s->out + s->used, p, r.accepted);
        s->used += r.accepted;
    }
    return r;
}

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

struct cwf_fd
{
    int fd, raw;
    unsigned calls;
    struct cpw_result last;
};

static struct cpw_result descriptor(void *context, const unsigned char *p,
                                    size_t n)
{
    struct cwf_fd *f = context;
    ++f->calls;
    if (n > 3)
    {
        n = 3;
    }
    if (f->raw)
    {
        f->last = cpw_raw(raw_write(f->fd, p, n));
    }
    else
    {
        ssize_t r = write(f->fd, p, n);
        int saved = r < 0 ? errno : 0;
        f->last = cpw_libc((long)r, saved);
    }
    return f->last;
}

static size_t fill(int fd)
{
    unsigned char bytes[4096];
    memset(bytes, 0x55, sizeof bytes);
    size_t total = 0;
    for (unsigned turn = 0; turn < 4096; ++turn)
    {
        ssize_t n = write(fd, bytes, sizeof bytes);
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

static void drain(int fd, size_t n)
{
    unsigned char bytes[4096];
    while (n)
    {
        size_t take = n < sizeof bytes ? n : sizeof bytes;
        ssize_t r = read(fd, bytes, take);
        CHECK(r > 0);
        for (ssize_t i = 0; i < r; ++i)
        {
            CHECK(bytes[i] == 0x55);
        }
        n -= (size_t)r;
    }
}

static void actual(int raw)
{
    int fds[2];
    CHECK(pipe(fds) == 0);
    CHECK(fcntl(fds[0], F_SETFL, O_NONBLOCK) == 0 &&
          fcntl(fds[1], F_SETFL, O_NONBLOCK) == 0);
    const unsigned char bytes[] = "ABCDEFGHI";
    unsigned char got[9];
    struct cpw_transfer t;
    struct cwf_fd f = {fds[1], raw, 0, {CPW_RETRY, 0, 0}};
    CHECK(cpw_begin(&t, bytes, 9, 99) == CPW_PENDING);
    for (unsigned cycle = 0; cycle < 3; ++cycle)
    {
        size_t full = fill(fds[1]);
        struct pollfd ready = {fds[1], POLLOUT, 0};
        CHECK(poll(&ready, 1, 0) == 0);
        unsigned before = f.calls;
        CHECK(cpw_step(&t, descriptor, &f) == CPW_PENDING &&
              f.calls == before + 1 && f.last.kind == CPW_RETRY &&
              f.last.cause == EAGAIN && t.accepted == cycle * 3);
        drain(fds[0], full);
        CHECK(poll(&ready, 1, 1000) == 1 && (ready.revents & POLLOUT));
        before = f.calls;
        CHECK(cpw_step(&t, descriptor, &f) ==
                  (cycle == 2 ? CPW_DONE : CPW_PENDING) &&
              f.calls == before + 1 && t.accepted == (cycle + 1) * 3);
        CHECK(read(fds[0], got + cycle * 3, 3) == 3);
    }
    CHECK(!memcmp(got, bytes, 9) && f.calls == 6);
    CHECK(close(fds[1]) == 0);
    f.fd = -1;
    CHECK(cpw_begin(&t, bytes, 9, 100) == CPW_PENDING &&
          cpw_step(&t, descriptor, &f) == CPW_FAILED &&
          t.error.primary == EBADF && !t.accepted);
    CHECK(close(fds[0]) == 0);
    printf("CWF_OS raw=%d cycles=3 callbacks=7 accepted=9 cause=%d\n", raw,
           EBADF);
}

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
int main(void)
{
    const unsigned char bytes[] = "ABCDEFGHI";
    struct cpw_transfer t;
    struct cwf_script s = {.results = {3, -4, 2, 4}, .base = bytes};
    CHECK(cpw_begin(&t, bytes, 9, 71) == CPW_PENDING);
    for (unsigned i = 0; i < 4; ++i)
    {
        CHECK(cpw_step(&t, scripted, &s) == (i == 3 ? CPW_DONE : CPW_PENDING));
        CHECK(t.accepted == (i < 2 ? 3 : i == 2 ? 5 : 9));
    }
    const size_t remaining[] = {9, 6, 6, 4}, offset[] = {0, 3, 3, 5};
    for (unsigned i = 0; i < 4; ++i)
    {
        CHECK(s.remaining[i] == remaining[i] && s.offset[i] == offset[i]);
    }
    CHECK(s.used == 9 && !memcmp(s.out, bytes, 9));
    s = (struct cwf_script){.results = {3, 0, -5}, .base = bytes};
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
    struct cpw_result a = cpw_raw(-2), b = cpw_script(-2),
                      c = cpw_libc(-1, EINTR);
    CHECK(a.kind == CPW_ERROR && a.cause == 2 && b.kind == CPW_RETRY &&
          b.cause == 1002 && c.kind == CPW_RETRY && c.cause == EINTR);
    a = cpw_raw(-EINTR);
    CHECK(a.kind == CPW_RETRY && !a.accepted);
    CHECK(cpw_raw(3).accepted == 3 && cpw_libc(3, 99).accepted == 3 &&
          cpw_script(-1).cause == 1001);
    struct cwf_script left = {.results = {3, -4, 2, -5}, .base = bytes},
                      right = left;
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
    actual(1);
    actual(0);
    puts("CWF_PROGRESS_PASS");
    return 0;
}
