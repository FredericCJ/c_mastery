#include "history.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define CK(x)                                                  \
    do                                                         \
    {                                                          \
        if (!(x))                                              \
        {                                                      \
            fprintf(stderr, "CH_CHECK:%d:%s\n", __LINE__, #x); \
            exit(41);                                          \
        }                                                      \
    } while (0)

static struct ch_op op(unsigned p, unsigned kind, unsigned value,
                       unsigned result, unsigned a, unsigned b)
{
    return (struct ch_op){a, b, a, b, p, kind, value, result, 0};
}

int main(int argc, char **argv)
{
    if (argc == 2 && !strcmp(argv[1], "driver"))
    {
        unsigned n, cap, spec, extend, events;
        uint32_t budget;
        while (scanf("%u%u%u%u%u%u", &n, &cap, &spec, &extend, &events,
                     &budget) == 6)
        {
            if (n > 10)
            {
                return 2;
            }
            struct ch_op o[10];
            for (unsigned i = 0; i < n; i++)
            {
                if (scanf("%u%u%u%u%u%u%u%u%u", &o[i].invoke, &o[i].respond,
                          &o[i].invoke_event, &o[i].respond_event,
                          &o[i].participant, &o[i].operation, &o[i].value,
                          &o[i].result, &o[i].pending) != 9)
                {
                    return 2;
                }
            }
            struct ch_witness w = {0};
            uint32_t visited = 0;
            int r =
                ch_check(o, n, cap, spec, extend, events, budget, &w, &visited);
            printf("%d %u %u %u", r, visited, w.count, w.selected_pending);
            if (r == CH_VALID)
            {
                for (unsigned i = 0; i < w.count; i++)
                {
                    printf(" %u,%u,%u", w.order[i], w.result[w.order[i]],
                           w.value[w.order[i]]);
                }
            }
            puts("");
        }
        return 0;
    }
    struct ch_witness w = {0}, saved;
    uint32_t visited = 0;
    struct ch_op h[10] = {op(0, CH_INSERT, 7, CH_VALUE, 0, 1),
                          op(1, CH_INSERT, 8, CH_VALUE, 2, 3),
                          op(2, CH_REMOVE, 8, CH_VALUE, 4, 5)};
    CK(ch_check(h, 3, 4, CH_FIFO, 0, 1, 10000, &w, &visited) == CH_NONLINEAR);
    CK(ch_check(h, 3, 4, CH_STACK, 0, 1, 10000, &w, &visited) == CH_VALID);
    h[2].value = 7;
    CK(ch_check(h, 3, 4, CH_FIFO, 0, 1, 10000, &w, &visited) == CH_VALID);
    CK(ch_check(h, 3, 4, CH_STACK, 0, 1, 10000, &w, &visited) == CH_NONLINEAR);
    h[0] = op(0, CH_INSERT, 7, CH_VALUE, 0, 1);
    h[1] = op(1, CH_INSERT, 8, CH_VALUE, 1, 2);
    h[2] = op(2, CH_REMOVE, 8, CH_VALUE, 3, 4);
    CK(ch_check(h, 3, 4, CH_FIFO, 0, 0, 10000, &w, &visited) == CH_VALID);
    for (unsigned i = 0; i < 3; i++)
    {
        h[i].invoke_event = 2 * i;
        h[i].respond_event = 2 * i + 1;
    }
    CK(ch_check(h, 3, 4, CH_FIFO, 0, 1, 10000, &w, &visited) == CH_NONLINEAR);
    h[1].participant = 0;
    h[1].invoke = 0;
    CK(ch_check(h, 3, 4, CH_FIFO, 0, 0, 10000, &w, &visited) == CH_INVALID);
    h[0] = op(0, CH_INSERT, 7, CH_VALUE, 0, 0);
    h[0].pending = 1;
    h[1] = op(1, CH_REMOVE, 7, CH_VALUE, 1, 2);
    saved = w;
    CK(ch_check(h, 2, 4, CH_FIFO, 0, 1, 10000, &w, &visited) == CH_PENDING &&
       !memcmp(&saved, &w, sizeof w));
    CK(ch_check(h, 2, 4, CH_FIFO, 1, 1, 10000, &w, &visited) == CH_VALID &&
       w.selected_pending == 1);
    h[1].value = 8;
    CK(ch_check(h, 2, 4, CH_FIFO, 1, 1, 10000, &w, &visited) == CH_NONLINEAR);
    h[1] = op(1, CH_REMOVE, 0, CH_EMPTY, 1, 2);
    CK(ch_check(h, 2, 4, CH_FIFO, 1, 1, 10000, &w, &visited) == CH_VALID);
    /* Nine complete calls: actual extension beyond original CMX's eight. */
    for (unsigned i = 0; i < 9; i++)
    {
        h[i] =
            op(i, i % 2 ? CH_REMOVE : CH_INSERT, 7, CH_VALUE, 2 * i, 2 * i + 1);
    }
    CK(ch_check(h, 9, 4, CH_FIFO, 0, 1, 10000, &w, &visited) == CH_VALID &&
       w.count == 9);
    saved = w;
    CK(ch_check(h, 9, 4, CH_FIFO, 0, 1, 1, &w, &visited) == CH_LIMIT &&
       visited == 1 && !memcmp(&saved, &w, sizeof w));
    CK(ch_check(h, 11, 4, CH_FIFO, 0, 1, 10000, &w, &visited) == CH_INVALID);
    puts("CH_PASS:sequential-spec exact-order timestamps pending-selection "
         "nine-calls bounded-search");
    return 0;
}
