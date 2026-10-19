#ifndef CMASTERY_DOD_ORDERED_H
#define CMASTERY_DOD_ORDERED_H
#include "dod.h"
typedef struct {DSparse pool;unsigned head,tail,next[4],previous[4],version,link_writes,iteration_steps;} DOrdered;
typedef struct {const DOrdered *owner;unsigned version,next;} DOrderedIter;
void d_order_init(DOrdered *);
int d_order_add(DOrdered *,int,DHandle *);
int d_order_remove(DOrdered *,DHandle);
int d_order_begin(const DOrdered *,DOrderedIter *);
int d_order_next(DOrdered *,DOrderedIter *,DHandle *,const int **,unsigned *has_value);
#endif
