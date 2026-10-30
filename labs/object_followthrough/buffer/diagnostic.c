/*
 * CM-SPECIMEN: defective by design; classify the defect before running a
 * tool on it (see CONTRACTS.md).
 * Original diagnostic target for tool experiments; its output is an
 * observation of a tool, never a language oracle.
 */
#include <stdlib.h>
volatile int ob_diagnostic_sink;

int main(int argc, char **argv)
{
    int *p = malloc(sizeof *p);
    int initialized = argc > 1 && argv[1][0] == '1';
    if (!p)
    {
        return 2;
    }
    if (initialized)
    {
        *p = 17;
    }
    if (*p == 17)
    {
        ob_diagnostic_sink = 1;
    }
    free(p);
    return 0;
}
