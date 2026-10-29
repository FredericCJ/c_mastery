#include "dag.h"
#include <stdio.h>

int main(void)
{
    for (unsigned mask = 0; mask < 4096; mask++)
    {
        struct dg_spec spec = {0};
        spec.count = 4;
        uint64_t costs[4] = {2, 3, 5, 1};
        for (unsigned i = 0; i < 4; i++)
        {
            spec.costs[i] = costs[i];
            spec.payloads[i] = i + 1;
        }
        unsigned bit = 0;
        for (unsigned i = 0; i < 4; i++)
            for (unsigned j = 0; j < 4; j++)
                if (i != j)
                {
                    if (mask & (1u << bit))
                        spec.edges[spec.edge_count++] = (struct dg_edge){i, j};
                    bit++;
                }
        struct sa_dg graph;
        int status = dg_init(&graph, 1, &spec);
        printf("%u,%d", mask, status);
        if (status == DG_OK)
        {
            if (dg_seal(&graph) != DG_OK)
                return 2;
            unsigned count = 0;
            struct dg_claim claim;
            while (count < 4 && dg_claim(&graph, count % 4, &claim) == DG_OK)
            {
                if (dg_finish(&graph, claim, DG_DONE, claim.input) != DG_OK)
                    return 3;
                count++;
            }
            struct dg_snapshot s;
            if (dg_snapshot(&graph, &s) != DG_OK)
                return 4;
            printf(",%llu,%llu,%u", (unsigned long long)s.work,
                   (unsigned long long)s.span, count);
            for (unsigned i = 0; i < 4; i++)
                printf(",%llu,%u", (unsigned long long)s.results[i],
                       s.ready_events[i]);
        }
        puts("");
    }
    return 0;
}
