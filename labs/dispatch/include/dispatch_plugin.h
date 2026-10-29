#ifndef CMASTERY_DISPATCH_PLUGIN_H
#define CMASTERY_DISPATCH_PLUGIN_H
#include "dispatch.h"
#include "plugin_base.h"
typedef int (*ds_describe_fn)(unsigned char *,size_t,size_t *);
struct ds_plugin_optional { struct ds_plugin_base base;ds_describe_fn describe; };
struct ds_feature { struct ds_loaded *owner;const struct ds_plugin_base *base;ds_describe_fn describe;unsigned features,active; };
int ds_feature_acquire(struct ds_loaded *,unsigned,struct ds_feature *);
int ds_feature_apply(const struct ds_feature *,int,int,int *);
int ds_feature_describe(const struct ds_feature *,unsigned char *,size_t,size_t *);
int ds_feature_release(struct ds_feature *);
#endif
