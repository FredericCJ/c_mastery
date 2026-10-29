#include "signals.h"
#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#define CHECK(c)                                                    \
    do                                                              \
    {                                                               \
        if (!(c))                                                   \
        {                                                           \
            fprintf(stderr, "SIGNALS-CHECK:%s:%d\n", #c, __LINE__); \
            return 1;                                               \
        }                                                           \
    } while (0)

struct rd_script
{
    unsigned call;
    int error;
};

static int now(void *arg, struct timespec *t)
{
    struct rd_script *s = arg;
    static const long ticks[] = {0, 400000000, 800000000, 1000000000};
    if (s->call > 3)
    {
        s->error = 1;
        errno = EIO;
        return -1;
    }
    *t = (struct timespec){ticks[s->call] / 1000000000,
                           ticks[s->call] % 1000000000};
    return 0;
}

static int wait_step(void *arg, const struct timespec *t)
{
    struct rd_script *s = arg;
    static const long nanos[] = {1000000000, 600000000, 200000000};
    if (s->call >= 3 || t->tv_sec * 1000000000 + t->tv_nsec != nanos[s->call])
        s->error = 1;
    ++s->call;
    errno = EINTR;
    return -1;
}

int main(void)
{
    unsigned histories = 0, power = 1;
    for (unsigned length = 0; length <= 8; ++length)
    {
        for (unsigned encoded = 0; encoded < power; ++encoded)
        {
            unsigned word = encoded, model[2] = {0}, used = 0;
            struct es_counted q = {0};
            for (unsigned i = 0; i < length; ++i)
            {
                unsigned op = word % 3;
                word /= 3;
                if (op < 2)
                {
                    int expected = used < 2;
                    CHECK(es_counted_push(&q, op + 1) == expected);
                    if (expected)
                        model[used++] = op + 1;
                }
                else
                {
                    unsigned out = 99;
                    int expected = used > 0;
                    CHECK(es_counted_pop(&q, &out) == expected);
                    if (expected)
                    {
                        CHECK(out == model[0]);
                        model[0] = model[1];
                        --used;
                    }
                    else
                        CHECK(out == 99);
                }
                CHECK(q.count == used);
            }
            ++histories;
        }
        power *= 3;
    }
    CHECK(histories == 9841);
    unsigned cases = 0;
    for (long n = 0; n <= 2000; n += 5)
        for (long d = 0; d <= 2000; d += 5)
        {
            struct timespec r = {-1, -1};
            int got = es_remaining(
                (struct timespec){n / 1000, (n % 1000) * 1000000},
                (struct timespec){d / 1000, (d % 1000) * 1000000}, &r);
            long expected = d > n ? d - n : 0;
            CHECK(got == (expected > 0));
            CHECK(r.tv_sec * 1000 + r.tv_nsec / 1000000 == expected);
            ++cases;
        }
    struct rd_script s = {0, 0};
    CHECK(es_wait_until((struct es_wait){now, wait_step, &s},
                        (struct timespec){1, 0}) == 0);
    CHECK(!s.error && s.call == 3);
    struct es_signals port = {0};
    sigset_t before, after;
    CHECK(pthread_sigmask(SIG_SETMASK, 0, &before) == 0);
    CHECK(es_open(&port) == 0);
    CHECK(es_open(&port) == -1 && errno == EBUSY);
    unsigned char bytes[1024] = {0};
    unsigned capacity = 0;
    ssize_t n;
    while ((n = write(port.write_fd, bytes, sizeof bytes)) > 0)
        capacity += (unsigned)n;
    CHECK(errno == EAGAIN && capacity > 0);
    CHECK(pthread_sigmask(SIG_SETMASK, &port.waiting_mask, 0) == 0);
    errno = EDOM;
    CHECK(raise(SIGUSR1) == 0);
    CHECK(errno == EDOM);
    CHECK(raise(SIGUSR2) == 0);
    CHECK(pthread_sigmask(SIG_BLOCK, &port.managed, 0) == 0);
    CHECK(es_take(&port, ES_STOP) == ES_STOP);
    CHECK(es_take(&port, ES_OTHER) == ES_OTHER);
    CHECK(es_take(&port, 7) == 0);
    CHECK(kill(getpid(), SIGUSR1) == 0);
    CHECK(es_wait(&port, &(struct timespec){1, 0}) == 1);
    CHECK(es_take(&port, ES_STOP) == ES_STOP);
    CHECK(es_close(&port) == 0);
    CHECK(pthread_sigmask(SIG_SETMASK, 0, &after) == 0);
    for (int i = 1; i < NSIG; ++i)
        CHECK(sigismember(&before, i) == sigismember(&after, i));
    printf(
        "SIGNALS-PASS:deadline_cases=%u;interruptions=3;full_pipe=%u;"
        "predicates=2;atomic_wait=1;mask_restored=1;counted_histories=9841;\n",
        cases, capacity);
    return 0;
}
