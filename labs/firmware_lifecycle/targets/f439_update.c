#include "f439_update.h"
int f4_record(const F4Selection*s,unsigned char b[80]){(void)s;(void)b;return 0;}
int f4_select(EGRead r,void*c,const FBPolicy*p,const FBLayout l[2],F4Selection*s){(void)r;(void)c;(void)p;(void)l;(void)s;return 0;}
enum FHResult f4_install_begin(F4Writer*w,FMIO io,const F4Selection*o,const RTFrame*f,const FBPolicy*p,const FBLayout l[2]){(void)w;(void)io;(void)o;(void)f;(void)p;(void)l;return FH_REJECT;}
int f4_install_result(const F4Writer*w,EGRead r,void*c,const FBPolicy*p,const FBLayout l[2],FBImage*out){(void)w;(void)r;(void)c;(void)p;(void)l;(void)out;return 0;}
enum FHResult f4_trial_begin(F4Writer*w,FMIO io,const F4Selection*o,const FBImage*i){(void)w;(void)io;(void)o;(void)i;return FH_REJECT;}
int f4_trial_probe(EGRead r,void*c,const FBPolicy*p,const FBLayout l[2],const F4Selection*o,F4Selection*s){(void)r;(void)c;(void)p;(void)l;(void)o;(void)s;return 0;}
enum FHResult f4_commit_begin(F4Writer*w,FMIO io,const F4Selection*o,const F4Selection*t,uint32_t g,uint32_t h){(void)w;(void)io;(void)o;(void)t;(void)g;(void)h;return FH_REJECT;}
enum FHResult f4_poll(F4Writer*w){(void)w;return FH_REJECT;}
