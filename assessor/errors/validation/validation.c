#include "validation.h"
#ifndef EVV_MUTATION
#define EVV_MUTATION 0
#endif
static void emit(struct evv_summary *r,struct evv_diagnostic *out,size_t cap,unsigned root,unsigned item,unsigned field){
    ++r->total;
    if(r->stored<cap){struct evv_diagnostic e={field,3,{root,EVV_MUTATION==5?0:item,field}};out[r->stored++]=e;}
}
struct evv_summary evv_validate(const struct evv_input *input,unsigned root,struct evv_input *output,struct evv_diagnostic *errors,size_t capacity){
    struct evv_summary r={0};
    if(!input||!output||!root||input->count>3||(capacity>9&&EVV_MUTATION!=7)||(capacity&&!errors)){r.total=SIZE_MAX;return r;}
    unsigned quota[3]={0},denominator[3]={0};
    if(EVV_MUTATION==1)*output=*input;
    for(size_t i=0;i<input->count;++i){
        er_config c=input->item[i];unsigned good[3]={c.age>=0&&c.age<=150,c.quota>=0&&c.quota<=1024,c.denominator>=1&&c.denominator<=4096};
        quota[i]=good[1];denominator[i]=good[2];
        for(unsigned field=0;field<3;++field)if(!good[field]&&(EVV_MUTATION!=2||r.total==0))emit(&r,errors,capacity,root,(unsigned)i,field+1);
    }
    size_t independent=r.total;
    for(size_t i=0;i<input->count;++i)if(quota[i]&&denominator[i]&&(EVV_MUTATION!=4||independent==0)){
        ++r.ratio_checks;uint32_t q=(uint32_t)input->item[i].quota,d=(uint32_t)input->item[i].denominator;
        unsigned bad=q/d>input->limit||(EVV_MUTATION!=3&&q/d==input->limit&&q%d!=0);
        if(bad)emit(&r,errors,capacity,root,(unsigned)i,4);
    }
    r.truncated=EVV_MUTATION==6?0:r.stored<r.total;
    if(!r.total)*output=*input;
    return r;
}
