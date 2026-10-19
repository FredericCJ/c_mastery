#include "inactive_install.h"
enum FHResult fu_begin(FUInstall*f,FMIO io,EGRead r,void*c,const RTFrame*t,const FBPolicy*p,const BSLayouts*l,const BSSelection*o){(void)f;(void)io;(void)r;(void)c;(void)t;(void)p;(void)l;(void)o;return FH_REJECT;}
enum FHResult fu_poll(FUInstall*f,FBImage*out){(void)f;(void)out;return FH_REJECT;}
