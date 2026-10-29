#include "root_checks.h"
#include <stdio.h>
#include <string.h>

int main(int argc, char **argv)
{
    if (argc == 2 && !strcmp(argv[1], "stream"))
    {
        unsigned profile, capacity, n;
        while (scanf("%u%u%u", &profile, &capacity, &n) == 3)
        {
            if (n > 65536)
            {
                return 2;
            }
            unsigned char p[65536];
            for (unsigned i = 0; i < n; ++i)
            {
                unsigned b;
                if (scanf("%2x", &b) != 1)
                {
                    return 2;
                }
                p[i] = (unsigned char)b;
            }
            struct sa_topology t = {0};
            int r = sa_parse(p, n, capacity, profile, &t);
            printf("SA %d %u %u %u %u %u %u", r, t.lapic, t.flags, t.count,
                   t.disabled, t.online_capable, t.unknown);
            for (unsigned i = 0; i < t.count; ++i)
            {
                printf(" %u %u %u", t.cpu[i].uid, t.cpu[i].apic, t.cpu[i].kind);
            }
            putchar('\n');
        }
        return 0;
    }
    if (argc != 1)
    {
        return 2;
    }
    int r = sa_checks();
    if (!r)
    {
        r = sa_root_checks();
    }
    if (r)
    {
        fprintf(stderr, "SA_CHECK:%d\n", r);
        return 1;
    }
    puts("SA_PASS:transactional-MADT-and-wide-identities");
    return 0;
}
