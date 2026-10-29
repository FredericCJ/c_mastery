#ifndef LW_WRAPPERS_H
#define LW_WRAPPERS_H

struct lw_observation
{
    unsigned left, right, calls;
    int value;
    char text[32];
};

int lw_trace(unsigned, struct lw_observation *);
int lw_min(unsigned, struct lw_observation *);
int lw_statement(unsigned, struct lw_observation *);
#endif
