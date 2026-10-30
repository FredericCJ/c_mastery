#include "context.h"
#include <errno.h>

/* Learner scaffold: every function compiles, ignores its inputs and returns a
 * sentinel, so the supplied tests fail until you implement the contracts in
 * context.h and CONTRACTS.md. */

int pc_launch(const char *path, char *const argv[], char *const env[], int cwd,
              int output, struct pc_launch *result)
{
    (void)path;
    (void)argv;
    (void)env;
    (void)cwd;
    (void)output;
    (void)result;
    return ENOSYS;
}

int pc_spawn(const char *path, char *const argv[], char *const env[], int cwd,
             int output, struct pc_launch *result)
{
    (void)path;
    (void)argv;
    (void)env;
    (void)cwd;
    (void)output;
    (void)result;
    return ENOSYS;
}

int pc_reap(struct pc_ticket *ticket, pc_wait_fn wait_fn, void *context,
            struct pc_launch *result)
{
    (void)ticket;
    (void)wait_fn;
    (void)context;
    (void)result;
    return ENOSYS;
}

int pc_replace(int directory, int lock, const unsigned char *data,
               size_t length, unsigned fail_stage, pc_step_fn step,
               void *context, struct pc_replace *result)
{
    (void)directory;
    (void)lock;
    (void)data;
    (void)length;
    (void)fail_stage;
    (void)step;
    (void)context;
    (void)result;
    return ENOSYS;
}

int pc_deadline(uint64_t now_ns, uint64_t budget_ns, uint64_t *deadline_ns)
{
    (void)now_ns;
    (void)budget_ns;
    (void)deadline_ns;
    return ENOSYS;
}

uint64_t pc_remaining(uint64_t deadline_ns, uint64_t now_ns)
{
    (void)deadline_ns;
    (void)now_ns;
    return 0;
}

int pc_choose(struct pc_capability *capability, unsigned epoch, int probe_error,
              int equivalent_poll, int operation_in_progress)
{
    (void)capability;
    (void)epoch;
    (void)probe_error;
    (void)equivalent_poll;
    (void)operation_in_progress;
    return ENOSYS;
}

int pc_operation_status(int error, size_t completed)
{
    (void)error;
    (void)completed;
    return 0;
}

int pc_ready(int backend, int fd)
{
    (void)backend;
    (void)fd;
    return -1;
}

int pc_request(unsigned available, unsigned requested, int injected_error,
               unsigned *accepted)
{
    (void)available;
    (void)requested;
    (void)injected_error;
    (void)accepted;
    return ENOSYS;
}
