#include "views.h"
FvStatus fv_reset(FvRecord *p){(void)p;return FV_UNFINISHED;}
FvView fv_initialize(FvRecord *p){(void)p;return (FvView){0};}
FvStatus fv_update(FvRecord *p,size_t i,int v){(void)p;(void)i;(void)v;return FV_UNFINISHED;}
FvView fv_view(const FvRecord *p){(void)p;return (FvView){0};}
FvStatus fv_copy(FvDestination d,FvView s){(void)d;(void)s;return FV_UNFINISHED;}
FvStatus fv_move(int *b,size_t e,size_t d,size_t s,size_t n){(void)b;(void)e;(void)d;(void)s;(void)n;return FV_UNFINISHED;}
FvOwner *fv_owner(void){return NULL;}
FvView fv_owner_view(const FvOwner *p){(void)p;return (FvView){0};}
int *fv_owner_mutable(FvOwner *p){(void)p;return NULL;}
unsigned fv_device_twice(volatile unsigned *p){(void)p;return 1;}
