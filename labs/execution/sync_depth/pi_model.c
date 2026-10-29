#include "pi.h"
#include <stdio.h>
#include <stdlib.h>
#define N 5u
#define CHECK(x)                                                 \
    do                                                           \
    {                                                            \
        if (!(x))                                                \
        {                                                        \
            fprintf(stderr, "CDPI_CHECK:%d:%s\n", __LINE__, #x); \
            exit(1);                                             \
        }                                                        \
    } while (0)

/* This oracle walks supplied acyclic owner paths; it does not inspect the
 * extension's reachability matrix or call CMX to derive expected priorities. */
static void oracle(const unsigned *base, const unsigned *owner, unsigned *out)
{
    for (unsigned i = 0; i < N; ++i)
    {
        out[i] = base[i];
    }
    for (unsigned i = 0; i < N; ++i)
    {
        for (unsigned j = owner[i]; j < N; j = owner[j])
        {
            if (out[j] > base[i])
            {
                out[j] = base[i];
            }
        }
    }
}

int main(void)
{
    unsigned graphs = 0, values = 0;
    for (unsigned code = 0; code < 120; ++code)
    {
        unsigned remainder = code, owner[N];
        for (unsigned i = 0; i < N; ++i)
        {
            unsigned choices = N - i;
            owner[i] = i + 1 + remainder % choices;
            remainder /= choices;
        }
        for (unsigned b = 0; b < 243; ++b)
        {
            cmx_kernel k;
            cmx_mutex mutexes[N];
            unsigned base[N], expected[N];
            unsigned radix = b;
            cmx_init(&k, CMX_FIXED);
            for (unsigned i = 0; i < N; ++i)
            {
                base[i] = 3 + 4 * (radix % 3);
                radix /= 3;
                CHECK(cmx_task_add(&k, base[i], 0, 4096 + i * 512, 256) ==
                      (int)i);
                CHECK(cmx_mutex_init(&k, &mutexes[i], 0, 0) == CMX_OK);
                CHECK(cmx_mutex_lock(&k, &mutexes[i], i, cmx_forever()) ==
                      CMX_OK);
            }
            for (unsigned i = 0; i < N; ++i)
            {
                if (owner[i] < N)
                {
                    CHECK(cmx_mutex_lock(&k, &mutexes[owner[i]], i,
                                         cmx_forever()) == CMX_BLOCKED);
                }
            }
            oracle(base, owner, expected);
            for (unsigned i = 0; i < N; ++i)
            {
                CHECK(k.task[i].priority == expected[i]);
                ++values;
            }
            base[0] = 1;
            struct cdpi_stats stats;
            CHECK(cdpi_reprioritize(&k, 0, 1, &stats) == CMX_OK);
            oracle(base, owner, expected);
            for (unsigned i = 0; i < N; ++i)
            {
                CHECK(k.task[i].priority == expected[i] &&
                      k.task[i].base_priority == base[i]);
                ++values;
            }
            ++graphs;
        }
    }
    CHECK(graphs == 29160 && values == 291600);
    printf(
        "CDPI_MODEL:%u acyclic graph/base combinations;%u observed priorities;owner-path oracle\n",
        graphs, values);
    return 0;
}
