#include <stdio.h>
#include <stdlib.h>

void sr_note(const char *s)
{
    fputs(s, stdout);
}

int sr_test(unsigned);

int main(int n, char **a)
{
    return n == 2 ? sr_test((unsigned)strtoul(a[1], 0, 10)) : 41;
}
