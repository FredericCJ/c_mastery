#include "float_contract.h"
#include <math.h>
#include <stdio.h>

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        return 2;
    }
    FILE *f = fopen(argv[1], "r");
    if (!f)
    {
        return 2;
    }
    size_t n, count = 0;
    double a[4096], b[4096];
    while (fscanf(f, "%zu", &n) == 1)
    {
        if (n > 4096)
        {
            return 3;
        }
        for (size_t i = 0; i < n; i++)
        {
            if (fscanf(f, "%la %la", &a[i], &b[i]) != 2)
            {
                return 3;
            }
        }
        double r[4];
        for (int i = 0; i < 3; i++)
        {
            if (ff_sum(a, n, (enum ff_method)i, &r[i]) != FF_OK)
            {
                return 4;
            }
        }
        if (ff_dot(a, b, n, &r[3]) != FF_OK)
        {
            return 4;
        }
        printf("%zu %a %a %a %a\n", count++, r[0], r[1], r[2], r[3]);
    }
    if (ferror(f) || fclose(f) != 0)
    {
        return 3;
    }
    fprintf(stderr, "FF_ORACLE_DRIVER_PASS cases=%zu\n", count);
    return 0;
}
