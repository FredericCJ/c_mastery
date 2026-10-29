#include "reactor.h"
#include <stdio.h>

static int callback(void *ctx, unsigned at)
{
    const unsigned *kind = ctx;
    return kind[at] == 0 ? 1 : kind[at] == 1 ? 0 : kind[at] == 2 ? -1 : -2;
}

int main(int argc, char **argv)
{
    if (argc != 2)
        return 2;
    FILE *f = fopen(argv[1], "r");
    if (!f)
        return 2;
    unsigned mask, start, budget, kind[3];
    while (fscanf(f, "%u %u %u %u %u %u", &mask, &start, &budget, &kind[0],
                  &kind[1], &kind[2]) == 6)
    {
        struct rd_scheduler s = {.ready = mask, .cursor = start};
        struct rd_turn t;
        if (rd_turn(&s, budget, callback, kind, &t))
            return 3;
        printf("%u %u %u %u", t.callbacks, t.successes, s.ready, s.cursor);
        for (unsigned i = 0; i < t.callbacks; ++i)
            printf(" %u", t.order[i]);
        putchar('\n');
    }
    int bad = ferror(f);
    return fclose(f) || bad ? 2 : 0;
}
