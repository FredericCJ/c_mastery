#define _POSIX_C_SOURCE 200809L
#include "parsing.h"
#include <inttypes.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

static uint64_t now(void)
{
    struct timespec t;
    if (clock_gettime(CLOCK_MONOTONIC, &t))
    {
        return 0;
    }
    return (uint64_t)t.tv_sec * UINT64_C(1000000000) + (uint64_t)t.tv_nsec;
}

int main(void)
{
    const char *patterns[] = {"a*", "(a|b)*abb", "(~|a)*"};
    const unsigned lengths[] = {16, 64, 256};
    puts(
        "pattern,length,round,queries,nfa_ns,dfa_ns,checksum,nfa_states,dfa_"
        "states,nfa_bytes,dfa_bytes,lr_table_bytes,pratt_tree_bytes,peg_result_"
        "bytes");
    for (unsigned p = 0; p < 3; ++p)
    {
        for (unsigned l = 0; l < 3; ++l)
        {
            unsigned char text[256];
            for (unsigned i = 0; i < lengths[l]; ++i)
            {
                text[i] = (unsigned char)(p == 1 ? 'a' + (i & 1) : 'a');
            }
            if (p == 1)
            {
                text[lengths[l] - 3] = 'a';
                text[lengths[l] - 2] = text[lengths[l] - 1] = 'b';
            }
            struct rx_nfa n;
            struct rx_dfa d;
            struct pd_diagnostic diag;
            if (rx_compile((const unsigned char *)patterns[p],
                           strlen(patterns[p]), &n, &diag) ||
                rx_determinize(&n, 16, &d))
            {
                return 2;
            }
            for (unsigned round = 0; round < 9; ++round)
            {
                uint64_t times[2] = {0};
                unsigned sums[2] = {0};
                for (unsigned order = 0; order < 2; ++order)
                {
                    unsigned which = (round + order) & 1;
                    uint64_t begin = now();
                    for (unsigned q = 0; q < 1000; ++q)
                    {
                        unsigned value;
                        int rc =
                            which ? rx_dfa_match(&d, text, lengths[l], &value)
                                  : rx_match(&n, text, lengths[l], 0, &value);
                        if (rc)
                        {
                            return 3;
                        }
                        sums[which] += value;
                    }
                    times[which] = now() - begin;
                }
                if (sums[0] != sums[1] || !times[0] || !times[1])
                {
                    return 4;
                }
                printf("%u,%u,%u,1000,%" PRIu64 ",%" PRIu64
                       ",%u,%u,%u,%zu,%zu,%zu,%zu,%zu\n",
                       p, lengths[l], round, times[0], times[1], sums[0],
                       n.count, d.count, sizeof n, sizeof d,
                       sizeof(struct lr_table), sizeof(struct pd_tree),
                       sizeof(struct ps_result));
            }
        }
    }
    return 0;
}
