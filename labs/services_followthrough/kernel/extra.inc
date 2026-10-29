#include "process.inc"

static void sf_process_cleanup(unsigned pid)
{
    (void)pid;
}

static void sf_thread_cleanup(unsigned tid)
{
    (void)tid;
}

static void sf_fork_maps(unsigned parent, unsigned child)
{
    (void)parent;
    (void)child;
}

static int64_t sf_syscall(struct frame *f)
{
    return sf_process_call(f);
}
