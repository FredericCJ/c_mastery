#include "trial_intent.h"
int ft_intent(const BSSelection*o,const FBImage i[2],unsigned char b[128]){(void)o;(void)i;(void)b;return 0;}
int ft_attempt(const unsigned char i[128],unsigned char b[32]){(void)i;(void)b;return 0;}
int ft_probe(EGRead r,void*c,const FBPolicy*p,const BSLayouts*l,const BSSelection*o,BSSelection*out){(void)r;(void)c;(void)p;(void)l;(void)o;(void)out;return 0;}
enum FHResult ft_begin(FTWriter*w,FMIO io,const BSSelection*o,const FBImage i[2]){(void)w;(void)io;(void)o;(void)i;return FH_REJECT;}
enum FHResult ft_poll(FTWriter*w){(void)w;return FH_REJECT;}
