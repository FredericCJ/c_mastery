#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
int hh_unchecked(const int *, size_t);
int main(int argc, char **argv)
{
    int values[4] = {10,20,30,40};
    size_t i = argc == 2 ? (size_t)strtoul(argv[1], NULL, 10) : 0;
    printf("HH_VALUE:%d\n", hh_unchecked(values, i));
    return 0;
}
