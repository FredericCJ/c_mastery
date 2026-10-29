#include "dod_ordered.h"
void d_order_init(struct d_ordered *o){(void)o;}
int d_order_add(struct d_ordered *o,int v,struct d_handle *h){(void)o;(void)v;(void)h;return D_FAILED;}
int d_order_remove(struct d_ordered *o,struct d_handle h){(void)o;(void)h;return D_FAILED;}
int d_order_begin(const struct d_ordered *o,struct d_ordered_iter *it){(void)o;(void)it;return D_FAILED;}
int d_order_next(struct d_ordered *o,struct d_ordered_iter *it,struct d_handle *h,const int **v,unsigned *has){(void)o;(void)it;(void)h;(void)v;(void)has;return D_FAILED;}
