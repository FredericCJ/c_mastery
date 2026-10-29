/* Controlled, deliberately faulty child programs. Never link into learner
 * library. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifndef REPAIRED
#define REPAIRED 0
#endif
static volatile int sink;

__attribute__((noinline)) static int overflow(void)
{
    char *p = malloc(8);
    if (!p)
    {
        return 2;
    }
    memset(p, 0, 8);
    volatile unsigned index = REPAIRED ? 7u : 8u;
    p[index] = 'x';
    sink = p[0];
    free(p);
    return 0;
}

__attribute__((noinline)) static int use_after_free(void)
{
    int *p = malloc(sizeof *p);
    if (!p)
    {
        return 2;
    }
    *p = 7;
    if (REPAIRED)
    {
        sink = *p;
        free(p);
    }
    else
    {
        free(p);
        sink = *p;
    }
    return 0;
}

__attribute__((noinline)) static int leak(void)
{
    char *volatile p = malloc(55);
    if (!p)
    {
        return 2;
    }
    p[0] = 42;
    sink = p[0];
    if (REPAIRED)
    {
        free(p);
    }
    /* Deliberately lose the only owner. Clear the local slot so conservative
       stale-stack reachability does not hide this particular leak experiment. */
    p = NULL;
    return 0;
}

__attribute__((noinline)) static int stack_buffer(void)
{
    volatile unsigned index = REPAIRED ? 3u : 4u;
    char values[4] = {0};
    values[index] = 7;
    sink = values[0];
    return 0;
}

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        return 2;
    }
    if (!strcmp(argv[1], "overflow"))
    {
        return overflow();
    }
    if (!strcmp(argv[1], "uaf"))
    {
        return use_after_free();
    }
    if (!strcmp(argv[1], "leak"))
    {
        return leak();
    }
    if (!strcmp(argv[1], "stack-buffer"))
    {
        return stack_buffer();
    }
    fprintf(stderr, "unknown case\n");
    return 2;
}
