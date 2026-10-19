#ifndef CMASTERY_DISPATCH_PLUGIN_H
#define CMASTERY_DISPATCH_PLUGIN_H
#include "dispatch.h"
#include "plugin_base.h"
typedef int (*DSDescribe)(unsigned char *,size_t,size_t *);
typedef struct { DSPluginBase base;DSDescribe describe; } DSPluginOptional;
typedef struct { DSLoaded *owner;const DSPluginBase *base;DSDescribe describe;unsigned features,active; } DSFeature;
int ds_feature_acquire(DSLoaded *,unsigned,DSFeature *);
int ds_feature_apply(const DSFeature *,int,int,int *);
int ds_feature_describe(const DSFeature *,unsigned char *,size_t,size_t *);
int ds_feature_release(DSFeature *);
#endif
