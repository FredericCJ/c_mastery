/* Original bounded UB target: invoke only as disposable ASan subprocess. */
#include <stdlib.h>
#include <string.h>

int main(int argc, char **argv)
{
    volatile unsigned char *p;
    volatile unsigned char observed;
    if (argc != 2)
    {
        return 2;
    }
    p = malloc(8);
    if (!p)
    {
        return 3;
    }
    p[0] = 65;
    if (strcmp(argv[1], "bounds") == 0)
    {
        p[8] = 66;
    }
    else if (strcmp(argv[1], "lifetime") == 0)
    {
        free((void *)p);
        observed = p[0];
        (void)observed;
        return 0;
    }
    else if (strcmp(argv[1], "double") == 0)
    {
        free((void *)p);
        free((void *)p);
        return 0;
    }
    else if (strcmp(argv[1], "repaired") != 0)
    {
        free((void *)p);
        return 2;
    }
    free((void *)p);
    return 0;
}
