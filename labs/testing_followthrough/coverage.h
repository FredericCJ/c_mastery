#ifndef COV_H
#define COV_H
#include <stddef.h>

struct cv_cv
{
    unsigned rejection_count, live;
    int queue[4];
    size_t count;
};

int cv_allow(struct cv_cv *, size_t length, int authorized);
int cv_push(struct cv_cv *, int);
void cv_cleanup(struct cv_cv *);
int cv_decision(int, int);
int cv_changed(int, int, int);
#endif
