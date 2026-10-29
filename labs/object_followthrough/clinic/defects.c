/* Original intentionally invalid local teaching programs. Run only under the named diagnostic harness. */
#include <stdlib.h>
#include <string.h>
static volatile int *escaped;
static void *volatile released;
static void *(*volatile copy_call)(void *, const void *, size_t) = memcpy;
volatile int mc_defect_sink;

__attribute__((noinline)) static void escape_source(void)
{
    volatile int local = 7;
    escaped = &local;
}

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        return 2;
    }
    if (!strcmp(argv[1], "escape"))
    {
        escape_source();
        mc_defect_sink = *escaped;
    }
    else if (!strcmp(argv[1], "release"))
    {
        void *p = malloc(16);
        if (!p)
        {
            return 2;
        }
        released = p;
        free(p);
        free(released);
    }
    else if (!strcmp(argv[1], "access"))
    {
        volatile int *p = malloc(sizeof *p);
        if (!p)
        {
            return 2;
        }
        *p = 7;
        released = (void *)p;
        free((void *)p);
        mc_defect_sink = *(volatile int *)released;
    }
    else if (!strcmp(argv[1], "overlap"))
    {
        unsigned char *p = malloc(16);
        if (!p)
        {
            return 2;
        }
        for (size_t i = 0; i < 16; ++i)
        {
            p[i] = (unsigned char)i;
        }
        memcpy(p + 1, p, 8);
        mc_defect_sink = p[2];
        free(p);
    }
    else if (!strcmp(argv[1], "overlap-indirect"))
    {
        unsigned char *p = malloc(16);
        if (!p)
        {
            return 2;
        }
        for (size_t i = 0; i < 16; ++i)
        {
            p[i] = (unsigned char)i;
        }
        copy_call(p + 1, p, 8);
        mc_defect_sink = p[2];
        free(p);
    }
    else
    {
        return 2;
    }
    return 0;
}
