#ifndef PC_CONTEXT_H
#define PC_CONTEXT_H
#include <stddef.h>
#include <stdint.h>
#include <sys/types.h>
typedef struct { int kind; int value; pid_t pid; } PCLaunch;
/* See CONTRACTS.md. kind: 1 exit, 2 launch errno, 3 terminating signal. */
int pc_launch(const char *, char *const [], char *const [], int cwd, int output, PCLaunch *);
int pc_spawn(const char *, char *const [], char *const [], int cwd, int output, PCLaunch *);
typedef struct { pid_t pid; unsigned phase; } PCTicket;
typedef pid_t (*PCWait)(pid_t, int *, void *);
/* phase1 owned, phase2 collected, phase3 collection authority lost. */
int pc_reap(PCTicket *, PCWait, void *, PCLaunch *);
typedef struct { unsigned stage; int error; int visible; } PCReplace;
typedef void (*PCStep)(void *, unsigned);
int pc_replace(int directory, int lock, const unsigned char *, size_t,
               unsigned fail_stage, PCStep, void *, PCReplace *);
/* All time values below are nonnegative integer nanoseconds in ONE domain. */
int pc_deadline(uint64_t now, uint64_t budget, uint64_t *deadline);
uint64_t pc_remaining(uint64_t deadline, uint64_t now);
typedef struct { unsigned epoch; int backend; int error; } PCCapability;
/* backend: 0 unprobed, 1 epoll, 2 poll, 3 unsupported/error. */
int pc_choose(PCCapability *, unsigned epoch, int probe_error,
              int equivalent_poll, int operation_in_progress);
/* No replay after progress: returns 0 success, 1 safely unsupported, 2 partial. */
int pc_operation_status(int error, size_t completed);
int pc_ready(int backend, int fd);
int pc_request(unsigned available, unsigned requested, int injected_error,
               unsigned *accepted);
#endif
