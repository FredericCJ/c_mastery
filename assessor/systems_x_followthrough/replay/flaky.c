#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv)
{
    const char *path = getenv("SX_REPLAY_STATE");
    unsigned n = 0;
    FILE *f;
    if (argc != 3 || !path)
    {
        return 2;
    }
    (void)argv;
    f = fopen(path, "r");
    if (f)
    {
        if (fscanf(f, "%u", &n) != 1)
        {
            fclose(f);
            return 2;
        }
        fclose(f);
    }
    f = fopen(path, "w");
    if (!f)
    {
        return 2;
    }
    if (fprintf(f, "%u\n", n + 1) < 0 || fclose(f))
    {
        return 2;
    }
    if (n % 2 == 0)
    {
        puts("TRANSITION:admission:seed=17:violations=1");
        return 1;
    }
    puts("violations=0");
    return 0;
}
