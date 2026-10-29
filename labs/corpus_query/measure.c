#include "query.h"
#include <stdio.h>
#include <time.h>

int main(void)
{
    struct qi_row rows[32];
    struct qi_index index;
    uint32_t out;
    struct qi_cost cost;
    printf("index_bytes=%zu row_bytes=%zu posting_bytes=%zu zone_bytes=%zu\n",
           sizeof index, sizeof(struct qi_row), sizeof(struct qi_posting),
           sizeof(struct qi_zone));
    for (unsigned order = 0; order < 2; order++)
    {
        for (unsigned i = 0; i < 32; i++)
            rows[i] = (struct qi_row){
                1, 1, 2, {0, 1}, (int16_t)(order ? (i * 13) % 32 : i)};
        if (qi_build(rows, 32, &index))
            return 1;
        clock_t start = clock();
        unsigned groups = 0, examined = 0, checksum = 0;
        for (unsigned k = 0; k < 10000; k++)
        {
            int16_t low = (int16_t)(k % 32);
            uint8_t phrase[] = {0, 1};
            if (qi_search(&index, 1, 0, low, low, phrase, 2, &out, &cost))
                return 1;
            groups += cost.groups;
            examined += cost.rows;
            checksum ^= out;
        }
        clock_t stop = clock();
        if (start == (clock_t)-1 || stop == (clock_t)-1)
            return 2;
        printf(
            "order=%s queries=10000 groups=%u rows=%u cpu_ticks=%.0f ticks_per_second=%ld checksum=%u\n",
            order ? "permuted" : "sorted", groups, examined,
            (double)(stop - start), (long)CLOCKS_PER_SEC, checksum);
    }
    return 0;
}
