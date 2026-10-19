#ifndef COV_H
#define COV_H
#include <stddef.h>
typedef struct {unsigned rejection_count,live;int queue[4];size_t count;} Cv;
int cv_allow(Cv *,size_t length,int authorized);
int cv_push(Cv *,int);
void cv_cleanup(Cv *);
int cv_decision(int,int);
int cv_changed(int,int,int);
#endif
