#include "async.h"
#include <stdio.h>
#include <stdlib.h>
static unsigned allocations, frees, calls, outcome;

static void *alloc(void *ctx, size_t n, size_t align)
{
    (void)ctx;
    if (align > _Alignof(max_align_t))
        return 0;
    void *p = malloc(n);
    if (p)
        ++allocations;
    return p;
}

static void drop(void *ctx, void *p)
{
    (void)ctx;
    ++frees;
    free(p);
}

static void terminal(void *ctx, enum ac_async_status status, struct ac_box *box)
{
    (void)ctx;
    ++calls;
    outcome = (unsigned)status;
    ac_box_drop(box);
}

int main(void)
{
    char trace[8];
    while (scanf("%7s", trace) == 1)
    {
        struct ac_operation op;
        ac_operation_init(&op);
        uint32_t ticket;
        allocations = frees = calls = 0;
        outcome = 99;
        if (ac_submit_v2(&op, 0, 0, (struct ac_allocate){0, alloc, drop},
                         terminal, 0, 0, &ticket) != ACX_OK)
            return 2;
        for (size_t i = 0; trace[i]; ++i)
        {
            enum ac_async_status status;
            switch (trace[i])
            {
                case 'C':
                case 'T':
                    status = ac_cancel_v2(&op, ticket);
                    break;
                case 'K':
                    status = ac_commit_v2(&op, ticket);
                    break;
                case 'S':
                    status = ac_finish_v2(&op, ticket, 0);
                    break;
                case 'A':
                    status = ac_finish_v2(&op, ticket, 1);
                    break;
                case '_':
                    continue;
                default:
                    return 3;
            }
            printf("%u,", (unsigned)status);
        }
        printf("|%u,%u,%u,%u,%u,%u,%u\n", (unsigned)op.core.state, op.committed,
               op.core.releases, calls, outcome, allocations, frees);
        if (op.core.state != F_DONE && ac_finish_v2(&op, ticket, 0) != ACX_OK)
            return 4;
        if (allocations != frees)
            return 5;
    }
    return 0;
}
