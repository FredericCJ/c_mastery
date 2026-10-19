#include "dispatch_resolution.h"
void ds_resolution_init(DSResolution *m){*m=(DSResolution){0};}
int ds_resolution_set(DSResolution *m,unsigned l,unsigned r,DSCombine f,DSLoaded *p){(void)m;(void)l;(void)r;(void)f;(void)p;return DS_FAILED;}
void ds_resolution_clear(DSResolution *m){(void)m;}
int ds_resolution_uncached(const DSResolution *m,unsigned l,unsigned r,int a,int b,int *out){(void)m;(void)l;(void)r;(void)a;(void)b;(void)out;return DS_FAILED;}
void ds_resolution_cache_init(DSResolutionCache *c){*c=(DSResolutionCache){0};}
void ds_resolution_cache_clear(DSResolutionCache *c){(void)c;}
int ds_resolution_call(const DSResolution *m,DSResolutionCache *c,unsigned l,unsigned r,int a,int b,int *out){(void)m;(void)c;(void)l;(void)r;(void)a;(void)b;(void)out;return DS_FAILED;}
