#include "owning_sum.h"
#ifndef ADO_MUTATION
#define ADO_MUTATION 0
#endif
void ado_init(AdoValue *v) { v->tag=ADO_EMPTY; v->data.number=0; }
void ado_drop(AdoValue *v) {
    if(v->tag==ADO_TEXT) {
        AdoText text=v->data.text;
        ado_init(v);
        if(ADO_MUTATION!=4)text.ops.drop(text.ops.context,text.bytes,text.length);
    } else ado_init(v);
}
static void publish(AdoValue *out,AdoValue staged) { ado_drop(out);*out=staged; }
AdoStatus ado_from_value(AdoValue *out,ADValue value) {
    uint64_t ignored;
    if(!out||!ad_measure(value,&ignored))return ADO_INVALID;
    AdoValue staged={ADO_EMPTY,{.number=0}};
    switch(value.tag) {
    case AD_EMPTY:break;
    case AD_NUMBER:staged.tag=ADO_NUMBER;staged.data.number=value.data.number;break;
    case AD_PAIR:staged.tag=ADO_PAIR;staged.data.pair=value.data.pair;break;
    default:return ADO_INVALID;
    }
    publish(out,staged);return ADO_OK;
}
AdoStatus ado_text(AdoValue *out,const unsigned char *bytes,size_t n,AdoOps ops) {
    if(!out||n>65535||(n&&!bytes)||!ops.clone||!ops.drop)return ADO_INVALID;
    AdoValue staged={ADO_TEXT,{.text={NULL,n,ops}}};
    if(ADO_MUTATION==2)ado_drop(out);
    if(!ops.clone(ops.context,bytes,n,&staged.data.text.bytes))return ADO_NOMEM;
    publish(out,staged);return ADO_OK;
}
AdoStatus ado_copy(AdoValue *out,const AdoValue *in) {
    if(!out||!in)return ADO_INVALID;
    if(out==in)return ADO_OK;
    if(in->tag==ADO_TEXT) {
        if(ADO_MUTATION==1) {publish(out,*in);return ADO_OK;}
        return ado_text(out,in->data.text.bytes,in->data.text.length,in->data.text.ops);
    }
    switch(in->tag) {
    case ADO_EMPTY:case ADO_NUMBER:case ADO_PAIR:publish(out,*in);return ADO_OK;
    case ADO_TEXT:break;
    }
    return ADO_INVALID;
}
AdoStatus ado_move(AdoValue *out,AdoValue *in) {
    if(!out||!in)return ADO_INVALID;
    if(out==in)return ADO_OK;
    AdoValue staged=*in;
    if(ADO_MUTATION!=5)ado_init(in);
    publish(out,staged);return ADO_OK;
}
AdoStatus ado_measure(const AdoValue *in,uint64_t *out) {
    if(!in||!out)return ADO_INVALID;
    ADValue value={AD_EMPTY,{.number=0}};
    switch(in->tag) {
    case ADO_EMPTY:break;
    case ADO_NUMBER:value.tag=AD_NUMBER;value.data.number=in->data.number;break;
    case ADO_PAIR:value.tag=AD_PAIR;value.data.pair=in->data.pair;break;
    case ADO_TEXT:*out=ADO_MUTATION==3?0:in->data.text.length;return ADO_OK;
    default:return ADO_INVALID;
    }
    return ad_measure(value,out)?ADO_OK:ADO_INVALID;
}
AdoStatus ado_number(const AdoValue *in,uint32_t *out) {
    if(!in||!out)return ADO_INVALID;
    if(ADO_MUTATION==6&&in->tag==ADO_TEXT){*out=0;return ADO_OK;}
    if(in->tag!=ADO_NUMBER)return ADO_INVALID;
    return ad_number((ADValue){AD_NUMBER,{.number=in->data.number}},out)?ADO_OK:ADO_INVALID;
}
