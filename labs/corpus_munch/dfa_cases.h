#ifndef MM_CASES_H
#define MM_CASES_H
#include "munch.h"

/* Public union DFA to audit against independent regex definitions:
 * rule0=abc, rule1=(abc)*d, rule2=abc (equal-length lower priority).
 * This is a fixture, not the tokenization implementation. */
static struct mm_dfa mm_example(void)
{
    struct mm_dfa d = {0};
    d.count = 8;
    for (unsigned i = 0; i < 256; ++i)
    {
        d.map[i] = 4;
    }
    d.map['a'] = 0;
    d.map['b'] = 1;
    d.map['c'] = 2;
    d.map['d'] = 3;
    for (unsigned q = 0; q < 8; ++q)
    {
        for (unsigned c = 0; c < 4; ++c)
        {
            d.next[q][c] = MM_DEAD;
        }
    }
    d.next[0][0] = 1;
    d.next[1][1] = 2;
    d.next[2][2] = 3;
    d.next[3][0] = 4;
    d.next[4][1] = 5;
    d.next[5][2] = 6;
    d.next[6][0] = 4;
    d.next[0][3] = 7;
    d.next[3][3] = 7;
    d.next[6][3] = 7;
    d.accept[3] = 5;
    d.accept[7] = 2;
    return d;
}
#endif
