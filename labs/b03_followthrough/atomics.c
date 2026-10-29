#include "follow.h"
void bf_box_init(struct bf_box*m){atomic_init(&m->state,0);m->payload=0;}
int bf_put(struct bf_box*m,int v){(void)m;(void)v;return B3_FAILED;}
int bf_get(struct bf_box*m,int*v){(void)m;(void)v;return B3_FAILED;}
