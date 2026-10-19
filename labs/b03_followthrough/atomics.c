#include "follow.h"
void bf_box_init(BfBox*m){atomic_init(&m->state,0);m->payload=0;}
int bf_put(BfBox*m,int v){(void)m;(void)v;return B3_FAILED;}
int bf_get(BfBox*m,int*v){(void)m;(void)v;return B3_FAILED;}
