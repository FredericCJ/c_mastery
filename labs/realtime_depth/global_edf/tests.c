#include "checks.h"
#include <stdio.h>
#include <string.h>

static int stream(void)
{
    unsigned n;
    struct ge_config c;
    while (scanf("%u%u%u%u%u%u", &n, &c.cpus, &c.horizon, &c.queue_cost,
                 &c.migration_cost, &c.partitioned) == 6)
    {
        if (!n || n > GE_JOBS)
        {
            return 2;
        }
        for (unsigned i = 0; i < GE_TASKS; i++)
        {
            if (scanf("%u", &c.home[i]) != 1)
            {
                return 2;
            }
        }
        struct ge_job jobs[GE_JOBS] = {0};
        for (unsigned i = 0; i < n; i++)
        {
            struct ge_job *j = &jobs[i];
            if (scanf("%u%u%u%u%u%u%u%u", &j->task, &j->generation,
                      &j->affinity, &j->strands, &j->nominal, &j->release,
                      &j->jitter, &j->deadline) != 8 ||
                !j->strands || j->strands > GE_STRANDS)
            {
                return 2;
            }
            for (unsigned v = 0; v < j->strands; v++)
            {
                if (scanf("%u%u", &j->cost[v], &j->predecessors[v]) != 2)
                {
                    return 2;
                }
            }
        }
        if (ge_simulate(jobs, n, &c, &ge_result) != GE_OK)
        {
            return 3;
        }
        const struct ge_result *r = &ge_result;
        printf("G %u %u %u %u %u %u %u %u %u\n", r->work, r->idle, r->queue,
               r->queue_wait, r->migration_overhead, r->migrations, r->misses,
               r->rejected, r->completed);
        for (unsigned i = 0; i < n; i++)
        {
            const struct ge_job_result *v = &r->jobs[i];
            printf("J %u %u %u %u %u %u", i, v->state, v->start, v->finish,
                   v->complete_mask, v->missed);
            for (unsigned s = 0; s < jobs[i].strands; s++)
            {
                printf(" %u %u", v->remaining[s], v->executed[s]);
            }
            puts("");
        }
        for (unsigned t = 0; t < c.horizon; t++)
        {
            for (unsigned cpu = 0; cpu < c.cpus; cpu++)
            {
                const struct ge_tick *v = &r->ticks[t][cpu];
                printf("T %u %u %u %u %u %u %u\n", t, cpu, v->kind, v->job,
                       v->strand, v->generation, v->remaining);
            }
        }
    }
    return ferror(stdin) ? 2 : 0;
}

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        return 2;
    }
    if (!strcmp(argv[1], "stream"))
    {
        return stream();
    }
    int result;
    if (!strcmp(argv[1], "dhall"))
    {
        result = dhall();
    }
    else if (!strcmp(argv[1], "dag"))
    {
        result = dag();
    }
    else if (!strcmp(argv[1], "overlap"))
    {
        result = overlap();
    }
    else if (!strcmp(argv[1], "costs"))
    {
        result = costs();
    }
    else if (!strcmp(argv[1], "affinity"))
    {
        result = affinity();
    }
    else if (!strcmp(argv[1], "comparison"))
    {
        result = comparison();
    }
    else
    {
        return 2;
    }
    if (result)
    {
        printf("GE_CHECK:%d\n", result);
        return 1;
    }
    printf("GE_PASS:%s\n", argv[1]);
    return 0;
}
