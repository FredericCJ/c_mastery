/* Original deliberately flawed diagnostic target; outputs are never language oracles. */
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
