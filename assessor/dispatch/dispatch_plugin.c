#include "dispatch_plugin.h"
#include <dlfcn.h>
#ifndef DS_FEATURE_MUTANT
#define DS_FEATURE_MUTANT 0
#endif
int ds_feature_acquire(DSLoaded *owner,unsigned known,DSFeature *out) {
  if(!owner || !out || out->active || known>1)return DS_INVALID;
  if(!owner->handle)return DS_EMPTY;
  dlerror();const DSPluginBase *base=dlsym(owner->handle,"dispatch_plugin_extended");const char *error=dlerror();
  if(error || !base || base->major!=2 || base->size<sizeof(DSPluginBase) || !base->apply)return DS_INVALID;
  if((base->required_features & ~base->provided_features) || (DS_FEATURE_MUTANT!=1 && (base->required_features & ~known)))return DS_UNSUPPORTED;
  unsigned features=base->provided_features & known;DSDescribe describe=NULL;
  if(features & 1){
    if(base->size<sizeof(DSPluginOptional)) {
      if(DS_FEATURE_MUTANT!=2)return DS_INVALID;
      /* Deliberately accept a short descriptor without reading a missing tail. */
      features=0;
    } else {
      const DSPluginOptional *optional=(const DSPluginOptional *)base;
      describe=optional->describe;if(!describe)return DS_INVALID;
    }
  }
  const DSPlugin *borrow=NULL;int rc=ds_loaded_acquire(owner,&borrow);if(rc!=DS_OK)return rc;
  (void)borrow;*out=(DSFeature){owner,base,describe,features,1};return DS_OK;
}
int ds_feature_apply(const DSFeature *feature,int state,int amount,int *out) {
  if(!feature || !feature->active || !out)return DS_INVALID;
  int staged=0;int rc=feature->base->apply(state,amount,&staged);if(rc==DS_OK)*out=staged;return rc;
}
int ds_feature_describe(const DSFeature *feature,unsigned char *out,size_t cap,size_t *length) {
  if(!feature || !feature->active || !length || (!out && cap) || cap>32)return DS_INVALID;
  if(!(feature->features & 1))return DS_UNSUPPORTED;
  unsigned char staged[32]={0};size_t n=0;int rc=feature->describe(staged,cap,&n);
  if(rc!=DS_OK)return rc;
  if(n>cap)return DS_INVALID;
  for(size_t i=0;i<n;++i)out[i]=staged[i];
  *length=n;return DS_OK;
}
int ds_feature_release(DSFeature *feature) {
  if(!feature || !feature->active)return DS_INVALID;
  DSLoaded *owner=feature->owner;*feature=(DSFeature){0};return ds_loaded_release(owner);
}
