#ifndef PC_CONTEXT_H
#define PC_CONTEXT_H
#include <stddef.h>
#include <stdint.h>
#include <sys/types.h>

/* Outcome of one launched child. kind: 1 exit (value is the exit status),
 * 2 launch error (value is the errno), 3 terminating signal (value is the
 * signal number). */
struct pc_launch
{
    int kind;
    int value;
    pid_t pid;
};

/* Launch the helper at `path` with `argv`/`env` (borrowed, null-terminated),
 * running in directory descriptor `cwd` with standard output sent to
 * `output`. Both descriptors are >= 3, distinct, open and caller-owned. See
 * CONTRACTS.md for the full contract.
 * Returns 0 and fills `result` once the child was collected, or a positive
 * errno value and leaves `result` untouched. Blocks until the child ends. */
int pc_launch(const char *path, char *const argv[], char *const env[], int cwd,
              int output, struct pc_launch *result);
/* Same contract and results through posix_spawn (GNU file actions). An
 * immediate spawn failure is reported as kind 2 with a 0 return. */
int pc_spawn(const char *path, char *const argv[], char *const env[], int cwd,
             int output, struct pc_launch *result);

/* Collection authority for one child. phase 1: the caller owns the child and
 * has not collected it; 2: collected; 3: collection authority lost. */
struct pc_ticket
{
    pid_t pid;
    unsigned phase;
};

/* Wait hook with waitpid() semantics, so tests can inject EINTR and errors. */
typedef pid_t (*pc_wait_fn)(pid_t pid, int *status, void *context);
/* Collect the child of an owned ticket, retrying EINTR. Returns 0 and fills
 * `result` (ticket becomes phase 2); EALREADY if the ticket is not owned;
 * ECHILD when another reaper took the child (ticket becomes phase 3, never
 * waited on again). Any other failure keeps the ticket owned and leaves
 * `result` untouched. Blocks like waitpid(). */
int pc_reap(struct pc_ticket *ticket, pc_wait_fn wait_fn, void *context,
            struct pc_launch *result);

/* Outcome of pc_replace(). stage: last completed step (0 none, 1 written,
 * 2 file synced, 3 renamed over the target, 4 directory synced). error: the
 * first errno seen. visible: 1 once this call's rename completed; a later
 * writer may still replace the name. */
struct pc_replace
{
    unsigned stage;
    int error;
    int visible;
};

/* Called after each completed stage; tests use it to cut the process there. */
typedef void (*pc_step_fn)(void *context, unsigned stage);
/* Atomically replace "data" in `directory` with `length` bytes (at most 4096)
 * while holding an exclusive flock on `lock`. `fail_stage` 1..4 injects EIO
 * at that stage; 0 injects nothing. Both descriptors are caller-owned and
 * remain open. Returns 0, EINVAL for bad arguments or the errno of a failed
 * lock (nothing touched), or the first errno otherwise; `result` is filled
 * once the lock is held. A name "temporary" already present makes the call
 * fail with EEXIST and is left untouched. Blocks while another holder has the lock. */
int pc_replace(int directory, int lock, const unsigned char *data,
               size_t length, unsigned fail_stage, pc_step_fn step,
               void *context, struct pc_replace *result);

/* All time values below are nonnegative integer nanoseconds in ONE domain.
 * pc_deadline returns ERANGE (and leaves *deadline alone) when now + budget
 * would wrap; pc_remaining returns the nonnegative interval length. */
int pc_deadline(uint64_t now_ns, uint64_t budget_ns, uint64_t *deadline_ns);
uint64_t pc_remaining(uint64_t deadline_ns, uint64_t now_ns);

/* Cached decision of which readiness mechanism to use. */
struct pc_capability
{
    unsigned epoch;
    int backend;
    int error;
};

/* backend: 0 unprobed, 1 epoll, 2 poll, 3 unsupported/error. A decision is
 * kept for its epoch; a new epoch re-probes unless an operation is in
 * progress (EBUSY, state unchanged). */
int pc_choose(struct pc_capability *capability, unsigned epoch, int probe_error,
              int equivalent_poll, int operation_in_progress);
/* No replay after progress: returns 0 success, 1 safely unsupported, 2 partial. */
int pc_operation_status(int error, size_t completed);
/* Zero-timeout readiness of `fd` through the chosen backend: 1 readable or at
 * end of file, 0 not ready, -1 with errno set (ENOTSUP for backend 3). */
int pc_ready(int backend, int fd);
/* Grant `requested` only if it is a subset of `available`; a positive
 * `injected_error` is returned unchanged (a negative one gives EINVAL). `accepted` is written only on 0. */
int pc_request(unsigned available, unsigned requested, int injected_error,
               unsigned *accepted);
#endif
