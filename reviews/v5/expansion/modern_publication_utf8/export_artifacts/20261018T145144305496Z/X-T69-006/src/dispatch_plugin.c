#include "dispatch_plugin.h"
int ds_feature_acquire(DSLoaded *m,unsigned k,DSFeature *o){(void)m;(void)k;(void)o;return DS_FAILED;}
int ds_feature_apply(const DSFeature *f,int s,int a,int *o){(void)f;(void)s;(void)a;(void)o;return DS_FAILED;}
int ds_feature_describe(const DSFeature *f,unsigned char *o,size_t n,size_t *l){(void)f;(void)o;(void)n;(void)l;return DS_FAILED;}
int ds_feature_release(DSFeature *f){(void)f;return DS_FAILED;}
