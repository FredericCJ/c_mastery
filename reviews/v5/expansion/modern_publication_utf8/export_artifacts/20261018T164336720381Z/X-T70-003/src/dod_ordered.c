#include "dod_ordered.h"
void d_order_init(DOrdered *o){(void)o;}
int d_order_add(DOrdered *o,int v,DHandle *h){(void)o;(void)v;(void)h;return D_FAILED;}
int d_order_remove(DOrdered *o,DHandle h){(void)o;(void)h;return D_FAILED;}
int d_order_begin(const DOrdered *o,DOrderedIter *it){(void)o;(void)it;return D_FAILED;}
int d_order_next(DOrdered *o,DOrderedIter *it,DHandle *h,const int **v,unsigned *has){(void)o;(void)it;(void)h;(void)v;(void)has;return D_FAILED;}
