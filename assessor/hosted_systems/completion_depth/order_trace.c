#include "completion.h"
#include <stdio.h>

static void visit(unsigned *order, unsigned used, unsigned at)
{
    if (at < 6)
    {
        for (unsigned i = 0; i < 6; ++i)
        {
            if (!(used & (1u << i)))
            {
                order[at] = i;
                visit(order, used | (1u << i), at + 1);
            }
        }
        return;
    }
    unsigned char storage[4][8], *buffers[4];
    for (unsigned i = 0; i < 4; ++i)
    {
        buffers[i] = storage[i];
    }
    struct co_pool p;
    uint64_t t[2], c[2];
    if (co_init(&p, buffers, 8, 2) != 0)
    {
        return;
    }
    for (unsigned i = 0; i < 2; ++i)
    {
        if (co_acquire(&p, i, 8, &t[i]) != 1 || co_cancel(&p, t[i], &c[i]) != 1)
        {
            return;
        }
    }
    for (unsigned i = 0; i < 6; ++i)
    {
        unsigned event = order[i], slot = event / 3, kind = event % 3;
        int result = kind == 0   ? co_complete(&p, t[slot], 4, 0)
                     : kind == 1 ? co_complete(&p, c[slot], -2, 0)
                                 : co_release(&p, t[slot]);
        for (unsigned j = 0; j < 6; ++j)
        {
            printf("%u", order[j]);
        }
        printf(",%u,%d,%d,%d,%d,%d,%d\n", i, result, p.slot[0].state,
               p.slot[1].state, p.slot[0].cancel_pending,
               p.slot[1].cancel_pending, co_quiescent(&p));
    }
}

int main(void)
{
    unsigned order[6];
    visit(order, 0, 0);
}
