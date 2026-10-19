#include "partial_optics.h"
#ifndef ADL_MUTATION
#define ADL_MUTATION 0
#endif
static AdlStatus path(AdlConfig s){
    if(s.present>1||s.inner.present>1)return ADL_INVALID;
    return s.present&&s.inner.present?ADL_OK:ADL_ABSENT;
}
AdlStatus adl_get(AdlConfig s,uint32_t *out){
    if(!out)return ADL_INVALID;
    AdlStatus status=path(s);if(status!=ADL_OK)return status;
    *out=s.inner.endpoint.x;return ADL_OK;
}
AdlStatus adl_put(AdlConfig s,uint32_t replacement,AdlConfig *out){
    if(!out)return ADL_INVALID;
    AdlStatus status=path(s);
    if(status!=ADL_OK){
        if(ADL_MUTATION!=1||status!=ADL_ABSENT)return status;
        s.present=1;s.inner.present=1;
    }
    s.inner.endpoint=ad_lens_x(s.inner.endpoint,replacement);
    if(ADL_MUTATION==2)s.inner.audit=replacement;
    *out=s;return ADL_OK;
}
AdlStatus adl_update(AdlConfig s,const AdlStep *steps,size_t n,AdlConfig *out){
    if(!out||n>2||(n&&!steps))return ADL_INVALID;
    for(size_t i=0;i<n;i++)if(!steps[i].change)return ADL_INVALID;
    AdlStatus status=path(s);if(status!=ADL_OK)return status;
    for(size_t i=0;i<n;i++){
        uint32_t value=0;
        if(steps[i].change(steps[i].context,s.inner.endpoint.x,&value)){
            if(ADL_MUTATION==6)*out=s;
            return ADL_CALLBACK;
        }
        if(ADL_MUTATION==5&&(steps[i].change(steps[i].context,s.inner.endpoint.x,&value)))return ADL_CALLBACK;
        s.inner.endpoint=ad_lens_x(s.inner.endpoint,value);
    }
    *out=s;return ADL_OK;
}
static AdlStatus io(AdlIO s){
    if(s.version!=1&&s.version!=2)return ADL_VERSION;
    if(!s.code||s.code>(s.version==1?255u:65535u)||(s.version==1&&s.context>255u))return ADL_RANGE;
    return ADL_OK;
}
AdlStatus adl_review(AdlIO s,AdlError *out){
    if(!out)return ADL_INVALID;
    AdlStatus status=io(s);if(status!=ADL_OK)return status;
    AdlError result={s.version,ADL_IO,{{s.code,s.context}}};
    if(ADL_MUTATION==3)result.version=1;
    *out=result;return ADL_OK;
}
AdlStatus adl_preview(AdlError s,AdlIO *out){
    if(!out)return ADL_INVALID;
    if(s.version!=1&&s.version!=2)return ADL_VERSION;
    if(s.tag==ADL_SUCCESS)return ADL_ABSENT;
    if(s.tag==ADL_TIMEOUT){
        if(s.version==1)return ADL_INVALID;
        if(ADL_MUTATION==4){*out=(AdlIO){2,1,s.data.timeout};return ADL_OK;}
        return ADL_ABSENT;
    }
    if(s.tag!=ADL_IO)return ADL_INVALID;
    AdlIO result={s.version,s.data.io.x,s.data.io.y};AdlStatus status=io(result);
    if(status!=ADL_OK)return status;
    *out=result;return ADL_OK;
}
