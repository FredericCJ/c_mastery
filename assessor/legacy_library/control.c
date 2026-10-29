#include "control.h"
#include <setjmp.h>
#include <signal.h>
#include <stdlib.h>
static volatile sig_atomic_t observed;

static void mark(int sig)
{
    (void)sig;
#ifndef LG_MUTANT_SIGNAL
    observed = 1;
#endif
}

enum lg_status lg_raise(void)
{
    observed = 0;
    void (*previous)(int) = signal(SIGTERM, mark);
    if (previous == SIG_ERR)
    {
        return LG_FAILURE;
    }
    int result = raise(SIGTERM);
    int seen = observed;
    int restored = signal(SIGTERM, previous) != SIG_ERR;
    return result == 0 && seen == 1 && restored ? LG_OK : LG_FAILURE;
}

static void work(struct lg_owner *o, jmp_buf destination, unsigned mode)
{
    if (mode == 2)
    {
        return;
    }
    o->allocation = malloc(8);
    if (!o->allocation)
    {
        return;
    }
    ++o->acquired;
    if (mode == 1)
    {
        longjmp(destination, 2);
    }
    if (mode == 3)
    {
        longjmp(destination, 0);
    }
}

enum lg_status lg_jump(struct lg_owner *o, unsigned mode, int *out)
{
    if (!o || !out || o->allocation || mode > 3)
    {
        return LG_ARGUMENT;
    }
    jmp_buf destination;
    volatile int changed = 0;
    enum lg_status result = LG_FAILURE;
    switch (setjmp(destination))
    {
        case 0:
            changed = 7;
            work(o, destination, mode);
            result = o->allocation ? LG_OK : LG_FAILURE;
            break;
        case 1:
            result = LG_OK;
            break;
        case 2:
            result = LG_OK;
            break;
        default:
            result = LG_FAILURE;
            break;
    }
#ifndef LG_MUTANT_CLEANUP
    if (o->allocation)
    {
        free(o->allocation);
        o->allocation = NULL;
        ++o->released;
    }
#endif
    *out = changed;
    return result;
}

#include "control_atomic.c"
