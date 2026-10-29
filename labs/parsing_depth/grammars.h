#ifndef PD_GRAMMARS_H
#define PD_GRAMMARS_H
#include "parsing.h"

static inline struct lr_grammar pd_grammar(unsigned mode)
{
    struct lr_grammar g = {0};
    if (mode == 0)
    {
        g = (struct lr_grammar){7,
                                {{12, 1, {8}},
                                 {8, 3, {8, 1, 9}},
                                 {8, 1, {9}},
                                 {9, 3, {9, 2, 10}},
                                 {9, 1, {10}},
                                 {10, 3, {3, 8, 4}},
                                 {10, 1, {0}}}};
    }
    else if (mode == 1)
    {
        g = (struct lr_grammar){3,
                                {{12, 1, {8}}, {8, 3, {8, 1, 8}}, {8, 1, {0}}}};
    }
    else
    {
        g = (struct lr_grammar){
            4, {{12, 1, {8}}, {8, 2, {9, 1}}, {9, 2, {0, 9}}, {9, 0, {0}}}};
    }
    return g;
}
#endif
