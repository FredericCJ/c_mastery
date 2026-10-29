#include "percpu.h"
#include <inttypes.h>
#include <stdio.h>

int main(void)
{
    unsigned kind, cpu, align;
    uint64_t a, b, c, d;
    while (scanf("%u %" SCNu64 " %" SCNu64 " %u %" SCNu64 " %u %" SCNu64, &kind,
                 &a, &b, &cpu, &c, &align, &d) == 7)
    {
        if (kind == 0)
        {
            struct sp_layout l = {0};
            int r = sp_layout(a, b, cpu, c, align, &l);
            printf("SP %d %" PRIu64, r, l.end);
            if (r == SP_OK)
            {
                for (unsigned i = 0; i < cpu * SP_PARTS; ++i)
                {
                    printf(" %" PRIu64 ":%" PRIu64, l.spans[i].low,
                           l.spans[i].high);
                }
            }
            puts("");
        }
        else
        {
            uint64_t f[] = {a, b, c}, out = UINT64_C(0x123456);
            int r = sp_budget(f, cpu, d, 120, align, 512, &out);
            printf("SP %d %" PRIu64 "\n", r, out);
        }
    }
    return ferror(stdin) ? 1 : 0;
}
