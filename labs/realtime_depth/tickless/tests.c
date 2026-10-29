#include "checks.h"
#include <inttypes.h>
#include <stdio.h>
#include <string.h>

static int stream(void)
{
    char kind;
    while (scanf(" %c", &kind) == 1)
    {
        if (kind == 'R')
        {
            unsigned a, b, width, inf;
            if (scanf("%u%u%u%u", &a, &b, &width, &inf) != 4)
            {
                return 2;
            }
            enum tq_relation r = TQ_AMBIGUOUS;
            enum tq_status s = tq_relation(a, b, width, inf, &r);
            printf("R %u %u\n", s, r);
        }
        else if (kind == 'T' || kind == 'B')
        {
            uint64_t now = 0, ms, n = 71;
            unsigned num, den;
            if (kind == 'B' && scanf("%" SCNu64, &now) != 1)
            {
                return 2;
            }
            if (scanf("%" SCNu64 "%u%u", &ms, &num, &den) != 3)
            {
                return 2;
            }
            enum tq_status s = kind == 'T'
                                   ? tq_ticks(ms, num, den, &n)
                                   : tq_not_before(now, ms, num, den, &n);
            printf("%c %u %" PRIu64 "\n", kind, s, n);
        }
        else if (kind == 'Q')
        {
            unsigned start, d0, d1, d2, before, after, irq;
            struct tq_timer t;
            struct tq_plan p;
            if (scanf("%u%u%u%u%u%u%u", &start, &d0, &d1, &d2, &before, &after,
                      &irq) != 7)
            {
                return 2;
            }
            if (tq_init(&t, start) != TQ_OK || tq_wait(&t, 0, d0, 0) != TQ_OK ||
                tq_wait(&t, 1, d1, 0) != TQ_OK ||
                tq_plan(&t, start + before, &p) != TQ_OK ||
                tq_wait(&t, 2, d2, 0) != TQ_OK)
            {
                return 1;
            }
            enum tq_status s = tq_commit(&t, p, start + after, irq);
            printf("Q %u %u %u %u %u %u %u %u %u %u\n", s, t.events, t.armed,
                   t.programmed, t.kernel.now, p.at, p.armed, t.dispatch[0],
                   t.dispatch[1], t.dispatch[2]);
        }
        else if (kind == 'S')
        {
            unsigned start, aon, num, den, count;
            struct tq_timer t;
            if (scanf("%u%u%u%u%u", &start, &aon, &num, &den, &count) != 5 ||
                count > 64 || tq_init(&t, start) != TQ_OK)
            {
                return 2;
            }
            struct tq_resume r = {aon, num, den, 0};
            for (unsigned i = 0; i < count; i++)
            {
                unsigned next;
                if (scanf("%u", &next) != 1)
                {
                    return 2;
                }
                enum tq_status s = tq_resume(&t, &r, next);
                printf("S %u %u %u %" PRIu64 "\n", s, t.kernel.now, r.last,
                       r.remainder);
            }
        }
        else
        {
            return 2;
        }
    }
    return 0;
}

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        return 2;
    }
    if (!strcmp(argv[1], "stream"))
    {
        return stream();
    }
    const char *names[] = {"relation",  "conversion", "wrap",
                           "reprogram", "resume",     "guards"};
    int (*checks[])(void) = {tq_check_relation, tq_check_conversion,
                             tq_check_wrap,     tq_check_reprogram,
                             tq_check_resume,   tq_check_guards};
    for (unsigned i = 0; i < 6; i++)
    {
        if (!strcmp(argv[1], names[i]))
        {
            int line = checks[i]();
            if (line)
            {
                fprintf(stderr, "TQ_CHECK:%s:%d\n", names[i], line);
                return 1;
            }
            printf("TQ_PASS:%s\n", names[i]);
            return 0;
        }
    }
    return 2;
}
