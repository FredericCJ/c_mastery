#ifndef CMASTERY_HOSTED_H
#define CMASTERY_HOSTED_H
#include <pthread.h>
#include <stddef.h>
#include <sys/types.h>
int hs_dup_owned(int borrowed);
int hs_copy(int input, int output, size_t limit, size_t *moved);
/* deadline is an absolute CLOCK_MONOTONIC nanosecond instant; cancellation wins
 * ties. */
ssize_t hs_read_until(int fd, void *buffer, size_t capacity, int cancel_fd,
                      long long deadline_ns);
int hs_spawn_exit(unsigned code, int *observed);
int hs_shared_roundtrip(unsigned char *mapping, size_t capacity,
                        unsigned offset, const char *text, size_t length);
/* 0 success; -1 syscall failure; -2 truncated frame; -3 capacity violation. */
int hs_read_frame(int fd, void *buffer, size_t capacity, size_t *length);
int hs_signal_roundtrip(int signal_number);
ssize_t hs_local_exchange(const void *input, size_t length, void *output,
                          size_t capacity);
int hs_reactor_sum(const int *fds, size_t count, unsigned *sum);
int hs_pool_sum(const int *values, size_t count, size_t workers, long *sum);
struct hs_queue {
  pthread_mutex_t mutex;
  pthread_cond_t readable, writable;
  int data[4];
  size_t head, count;
  int closed;
};
int hs_queue_init(struct hs_queue *);
int hs_queue_push(struct hs_queue *, int);
/* 1 item; 0 closed and empty; -1 failure. */
int hs_queue_pop(struct hs_queue *, int *);
int hs_queue_close(struct hs_queue *);
int hs_queue_destroy(struct hs_queue *);
struct hs_clocks {
  long long wall_ns, cpu_ns, resolution_ns;
  unsigned long fd_soft;
};
int hs_observe_clocks(struct hs_clocks *);
/* Change RLIMIT_FSIZE only in a child. Return 0 when actual writes stop at
 * limit. */
int hs_limit_file(int fd, size_t limit);
#endif
