/* Isolated original teaching specimens; select one defect per short process. */
#include <stdlib.h>
#include <stdio.h>
#include <limits.h>
#ifndef DIAG_REPAIRED
#define DIAG_REPAIRED 0
#endif
volatile int diagnostic_sink;

void diagnostic_access(volatile int *p, unsigned mode)
{
    if (mode == 0)
    {
        p[DIAG_REPAIRED ? 3 : 4] = 9;
    }
    else if (mode == 1)
    {
        diagnostic_sink = p[0];
    }
    else if (mode == 2)
    {
        if (p[1])
        {
            diagnostic_sink = 7;
        }
        else
        {
            diagnostic_sink = 3;
        }
    }
    else
    {
        volatile int x = INT_MAX;
        diagnostic_sink = DIAG_REPAIRED ? x : x + 1;
    }
}

int main(int argc, char **argv)
{
    if (argc != 2 || argv[1][0] < '0' || argv[1][0] > '3' || argv[1][1])
    {
        return 2;
    }
    unsigned mode = (unsigned)(argv[1][0] - '0');
    int *p = malloc(4 * sizeof *p);
    if (!p)
    {
        return 2;
    }
    if (mode != 2 || DIAG_REPAIRED)
    {
        for (unsigned i = 0; i < 4; i++)
        {
            p[i] = (int)i;
        }
    }
    if (mode == 1 && !DIAG_REPAIRED)
    {
        free(p);
        diagnostic_access(p, mode);
    }
    else
    {
        diagnostic_access(p, mode);
        free(p);
    }
    puts("DIAG_TERMINATED");
    return 0;
}
