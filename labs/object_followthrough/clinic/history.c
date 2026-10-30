/*
 * Exhaustive enumeration of every length-4 sequence over three owner
 * operations (3^4 = 81 histories) against a real allocator. One CSV line per
 * step:
 *   history,step,event,status,live,releases,out
 * where event 0 is mc_acquire(7), 1 is mc_release, 2 is mc_read; live is 1
 * while the owner holds a block; out is 99 unless the read succeeded.
 */
#include "clinic.h"
#include <stdio.h>
#include <stdlib.h>

enum
{
    EVENT_KINDS = 3,
    STEPS_PER_HISTORY = 4,
    HISTORY_COUNT = EVENT_KINDS * EVENT_KINDS * EVENT_KINDS * EVENT_KINDS
};

static void *allocate(void *c, size_t n)
{
    (void)c;
    return malloc(n);
}

static void release(void *c, void *p)
{
    (void)c;
    free(p);
}

int main(void)
{
    struct mc_allocator a = {NULL, allocate, release};
    for (unsigned history = 0; history < HISTORY_COUNT; ++history)
    {
        struct mc_owner owner = {0};
        unsigned code = history; /* base-3 digits select the events */
        for (unsigned step = 0; step < STEPS_PER_HISTORY; ++step)
        {
            unsigned event = code % EVENT_KINDS;
            int status;
            int out = 99;
            code /= EVENT_KINDS;
            status = event == 0   ? mc_acquire(&owner, a, 7)
                     : event == 1 ? mc_release(&owner)
                                  : mc_read(&owner, &out);
            printf("%u,%u,%u,%d,%u,%u,%d\n", history, step, event, status,
                   owner.value != NULL, owner.releases, out);
        }
        if (owner.value && mc_release(&owner))
        {
            return 2;
        }
    }
    return 0;
}
