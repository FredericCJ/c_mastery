#include "checkpoint_channel.h"
void cpw_channel_init(CPWChannel *c){(void)c;}
CPMStatus cpw_offer(CPWChannel *c,const int *v,size_t n){(void)c;(void)v;(void)n;return CPM_INVALID;}
CPWStatus cpw_channel_step(CPWChannel *c,CPWWrite w,void *p){(void)c;(void)w;(void)p;return CPW_INVALID;}
