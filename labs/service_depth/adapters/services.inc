#include "kernel_abi.h"

/* These hooks run while the retained kernel metadata lock is held.
 * Implement the published ownership/state transitions; never trust user token
 * bits as authority. The public guest probes exercise the missing mechanisms. */
static void sd_process_cleanup(unsigned pid)
{
    (void)pid;
}

static void sd_fork_files(unsigned parent, unsigned child)
{
    (void)parent;
    (void)child;
}

static void sd_thread_cleanup(unsigned tid)
{
    (void)tid;
}

static int64_t sd_syscall(struct frame *f)
{
    (void)f;
    return SDK_INVALID;
}
