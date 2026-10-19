#define _GNU_SOURCE
#include "hosted.h"
#include "timer.h"
#include <errno.h>
#include <fcntl.h>
#include <inttypes.h>
#include <limits.h>
#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/eventfd.h>
#include <sys/timerfd.h>
#include <time.h>
#include <unistd.h>
#define CHECK(x)                                                               \
  do {                                                                         \
    if (!(x)) {                                                                \
      fprintf(stderr, "TIMER-NATIVE-CHECK:%d:%s errno=%d\n", __LINE__, #x,     \
              errno);                                                          \
      exit(1);                                                                 \
    }                                                                          \
  } while (0)
_Static_assert(sizeof(uint64_t) == 8, "Linux counter width");
_Static_assert(sizeof(time_t) >= 8, "Selected hosted time ABI");
static uint64_t now(void) {
  struct timespec t;
  CHECK(!clock_gettime(CLOCK_MONOTONIC, &t));
  CHECK(t.tv_sec >= 0 &&
        (uint64_t)t.tv_sec <= (UINT64_MAX - 999999999) / 1000000000);
  return (uint64_t)t.tv_sec * 1000000000 + (uint64_t)t.tv_nsec;
}
static struct timespec stamp(uint64_t n) {
  return (struct timespec){(time_t)(n / 1000000000), (long)(n % 1000000000)};
}
static void arm(int fd, uint64_t deadline, uint64_t interval) {
  struct itimerspec spec = {.it_value = stamp(deadline),
                            .it_interval = stamp(interval)};
  CHECK(!timerfd_settime(fd, TFD_TIMER_ABSTIME, &spec, NULL));
}
static void sleep_until(uint64_t n) {
  struct timespec t = stamp(n);
  int result;
  do
    result = clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &t, NULL);
  while (result == EINTR);
  CHECK(!result);
}
static uint64_t read_count(int fd) {
  uint64_t count = 0;
  ssize_t n;
  do
    n = read(fd, &count, sizeof count);
  while (n < 0 && errno == EINTR);
  CHECK(n == (ssize_t)sizeof count);
  return count;
}
static void event_counters(void) {
  int fd = eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
  CHECK(fd >= 0);
  uint64_t n = 2, value = 99;
  CHECK(write(fd, &n, 8) == 8);
  n = 3;
  CHECK(write(fd, &n, 8) == 8);
  CHECK(read(fd, &value, 4) == -1 && errno == EINVAL && value == 99);
  CHECK(read_count(fd) == 5);
  CHECK(read(fd, &value, 8) == -1 && errno == EAGAIN && value == 99);
  n = UINT64_MAX - 1;
  CHECK(write(fd, &n, 8) == 8);
  n = 1;
  CHECK(write(fd, &n, 8) == -1 && errno == EAGAIN);
  CHECK(read_count(fd) == UINT64_MAX - 1);
  n = UINT64_MAX;
  CHECK(write(fd, &n, 8) == -1 && errno == EINVAL);
  CHECK(!close(fd));
  fd = eventfd(3, EFD_NONBLOCK | EFD_CLOEXEC | EFD_SEMAPHORE);
  CHECK(fd >= 0);
  for (unsigned i = 0; i < 3; ++i)
    CHECK(read_count(fd) == 1);
  CHECK(read(fd, &value, 8) == -1 && errno == EAGAIN);
  CHECK(!close(fd));
  puts("TIMER-EVENT:sum=5;short_rejected=1;saturation_preserved=1;semaphore_"
       "ones=3;");
}
static void periodic(void) {
  int fd = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
  CHECK(fd >= 0);
  uint64_t value = 99;
  CHECK(read(fd, &value, 8) == -1 && errno == EAGAIN && value == 99);
  uint64_t period = 2000000, first = now() + period;
  arm(fd, first, period);
  sleep_until(first + 5 * period + period / 2);
  CHECK(read(fd, &value, 4) == -1 && errno == EINVAL);
  uint64_t before = now(), count = read_count(fd), dispatch = now();
  /* A read linearizes somewhere between the surrounding clock samples. */
  uint64_t low = (before - first) / period + 1,
           high = (dispatch - first) / period + 1;
  CHECK(count >= low && count <= high && count >= 6 && dispatch > first);
  TdTimer t;
  TdDispatch catchup, skip;
  unsigned g;
  CHECK(td_init(&t, 2) == 0 && td_arm(&t, first, period, 0, &g) == 1);
  CHECK(td_begin(&t, g, first + (count - 1) * period, &catchup) == 1);
  CHECK(td_finish(&t, g) == 1 && td_arm(&t, first, period, 1, &g) == 1);
  CHECK(td_begin(&t, g, first + (count - 1) * period, &skip) == 1);
  CHECK(catchup.effects == count && skip.effects == 1 &&
        catchup.next == skip.next);
  arm(fd, 0, 0);
  CHECK(read(fd, &value, 8) == -1 && errno == EAGAIN);
  CHECK(!close(fd));
  printf("TIMER-PERIODIC:first=%" PRIu64 ";period=%" PRIu64
         ";read_before=%" PRIu64 ";dispatch=%" PRIu64 ";count=%" PRIu64
         ";catchup=%" PRIu64 ";skip=1;next=%" PRIu64 ";\n",
         first, period, before, dispatch, count, catchup.effects, catchup.next);
}
static void rearm(void) {
  int fd = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC),
      cancel = eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
  CHECK(fd >= 0 && cancel >= 0);
  TdTimer timers[3];
  unsigned token[3];
  size_t index;
  uint64_t deadline;
  for (unsigned i = 0; i < 3; ++i)
    CHECK(td_init(&timers[i], 4) == 0);
  uint64_t start = now(), late = start + 5000000000ULL, early = start + 5000000;
  CHECK(td_arm(&timers[0], late, 0, 0, &token[0]) == 1);
  CHECK(td_earliest(timers, 3, &index, &deadline) == 1 && deadline == late);
  arm(fd, deadline, 0);
  CHECK(td_arm(&timers[1], early, 0, 0, &token[1]) == 1);
  CHECK(td_earliest(timers, 3, &index, &deadline) == 1 && index == 1 &&
        deadline == early);
  arm(fd, deadline, 0);
  uint64_t count = 0;
  CHECK(now() < LLONG_MAX - 2000000000ULL);
  CHECK(hs_read_until(fd, &count, sizeof count, cancel,
                      (long long)(now() + 2000000000)) == 8 &&
        count == 1);
  uint64_t dispatched = now();
  CHECK(dispatched >= early);
  TdDispatch d;
  CHECK(td_begin(&timers[1], token[1], dispatched, &d) == 1 &&
        td_finish(&timers[1], token[1]) == 1);
  CHECK(td_earliest(timers, 3, &index, &deadline) == 1 && index == 0 &&
        deadline == late);
  /* Select original late instant again, even if the host was stalled past it.
   */
  arm(fd, deadline, 0);
  CHECK(td_cancel(&timers[0]) == 1);
  CHECK(td_earliest(timers, 3, &index, &deadline) == 0);
  arm(fd, 0, 0);
  uint64_t one = 1;
  CHECK(write(cancel, &one, 8) == 8);
  CHECK(hs_read_until(fd, &count, 8, cancel, (long long)(now() + 2000000000)) ==
        -3);
  CHECK(!close(fd) && !close(cancel));
  printf("TIMER-REARM:earlier_selected=1;restored_late=1;empty_disarmed=1;"
         "provider_cancel=1;early=%" PRIu64 ";dispatch=%" PRIu64 ";\n",
         early, dispatched);
}
static void cached(void) {
  int fd = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC),
      ep = epoll_create1(EPOLL_CLOEXEC);
  CHECK(fd >= 0 && ep >= 0);
  TdTimer t;
  unsigned token;
  TdDispatch d;
  CHECK(td_init(&t, 3) == 0);
  uint64_t first = now() + 1000000;
  CHECK(td_arm(&t, first, 0, 0, &token) == 1);
  struct epoll_event registration = {.events = EPOLLIN, .data.u64 = token},
                     saved;
  CHECK(!epoll_ctl(ep, EPOLL_CTL_ADD, fd, &registration));
  arm(fd, first, 0);
  CHECK(epoll_wait(ep, &saved, 1, 2000) == 1 && saved.data.u64 == 1);
  CHECK(td_cancel(&t) == 1 && td_cancel(&t) == 0);
  arm(fd, 0, 0);
  CHECK(td_arm(&t, now() + 1000000, 0, 0, &token) == 1 && token == 2);
  registration.data.u64 = token;
  CHECK(!epoll_ctl(ep, EPOLL_CTL_MOD, fd, &registration));
  arm(fd, t.deadline, 0);
  CHECK(td_begin(&t, (unsigned)saved.data.u64, UINT64_MAX, &d) == 0 &&
        t.state == TD_ARMED);
  struct epoll_event fresh;
  CHECK(epoll_wait(ep, &fresh, 1, 2000) == 1 && fresh.data.u64 == 2);
  CHECK(read_count(fd) == 1 &&
        td_begin(&t, (unsigned)fresh.data.u64, now(), &d) == 1 &&
        td_finish(&t, token) == 1);
  CHECK(!epoll_ctl(ep, EPOLL_CTL_DEL, fd, NULL) && !close(ep) && !close(fd));
  puts("TIMER-CACHED:old=1;new=2;old_rejected=1;new_fired=1;repeat_cancel=0;");
}
static void failures(void) {
  unsigned closed = 0;
  for (unsigned cut = 0; cut <= 4; ++cut) {
    int owned[3] = {-1, -1, -1};
    unsigned acquired = 0;
    if (cut == 0)
      goto cleanup;
    owned[0] = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
    CHECK(owned[0] >= 0);
    ++acquired;
    if (cut == 1)
      goto cleanup;
    owned[1] = eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
    CHECK(owned[1] >= 0);
    ++acquired;
    if (cut == 2)
      goto cleanup;
    owned[2] = epoll_create1(EPOLL_CLOEXEC);
    CHECK(owned[2] >= 0);
    ++acquired;
    struct epoll_event e = {.events = EPOLLIN, .data.u64 = 1};
    CHECK(!epoll_ctl(owned[2], EPOLL_CTL_ADD, owned[0], &e));
    if (cut == 3) {
      struct itimerspec bad = {.it_value = {1, 1000000000}};
      CHECK(timerfd_settime(owned[0], 0, &bad, NULL) == -1 && errno == EINVAL);
    } else
      arm(owned[0], now() + 1000000, 0);
  cleanup:
    while (acquired) {
      int fd = owned[--acquired];
      CHECK(!close(fd));
      ++closed;
      CHECK(fcntl(fd, F_GETFD) == -1 && errno == EBADF);
      owned[acquired] = -1;
    }
    for (unsigned i = 0; i < 3; ++i)
      CHECK(owned[i] == -1);
  }
  CHECK(closed == 9);
  puts("TIMER-FAILURE:acquisition_cuts=3;invalid_arm=1;success=1;closed=9;");
}
int main(int argc, char **argv) {
  CHECK(argc == 2);
  if (!strcmp(argv[1], "event"))
    event_counters();
  else if (!strcmp(argv[1], "periodic"))
    periodic();
  else if (!strcmp(argv[1], "rearm"))
    rearm();
  else if (!strcmp(argv[1], "cached"))
    cached();
  else if (!strcmp(argv[1], "failure"))
    failures();
  else
    CHECK(0);
}
