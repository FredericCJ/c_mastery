#include "dispatch.h"
#include <stdio.h>
#include <string.h>
#define CHECK(x) do{if(!(x)){fprintf(stderr,"DISPATCH-CHECK:%d:%s\n",__LINE__,#x);return 1;}}while(0)
int main(void){unsigned cases=0;
 for(unsigned form=0;form<3;++form)for(unsigned kind=0;kind<(form==2?1u:3u);++kind)for(unsigned n=0;n<=64;++n){
  unsigned char data[64],gold[66];unsigned sum=0;for(unsigned i=0;i<n;++i){data[i]=(unsigned char)(i*17+3);sum=(sum+data[i])%256;}gold[0]=(unsigned char)n;memcpy(gold+1,data,n);gold[n+1]=(unsigned char)sum;
  struct ad_direct d;struct ad_erased e;struct ad_sink fixed={0};CHECK(ad_direct_init(&d,kind)==SF_OK&&ad_erased_init(&e,ad_operations(kind))==SF_OK);sf_write_fn write=form==0?ad_direct_write:form==1?ad_erased_write:ad_fixed_write;void *ctx=form==0?(void*)&d:form==1?(void*)&e:(void*)&fixed;struct ad_sink *sink=form==0?&d.sink:form==1?&e.sink:&fixed;
  struct sf_stream frame;sf_init(&frame);struct sf_handle h;CHECK(sf_start(&frame,data,n,&h)==SF_OK);size_t accepted=999;CHECK(sf_pump(h,write,ctx,100,&accepted)==SF_DONE&&accepted==n+2);CHECK(sink->length==n+2&&memcmp(gold,sink->bytes,n+2)==0);
  size_t quota=kind==0?1:kind==1?4:66,calls=(n+2+quota-1)/quota,offered=calls*(n+2)-quota*calls*(calls-1)/2;CHECK(sink->calls==calls&&sink->offered==offered);CHECK(sf_pump(h,write,ctx,100,&accepted)==SF_DONE&&sink->calls==calls);++cases;
 }
 for(unsigned form=0;form<3;++form)for(unsigned mode=1;mode<=4;++mode){struct ad_direct d;struct ad_erased e;CHECK(ad_direct_init(&d,1)==SF_OK&&ad_erased_init(&e,ad_operations(1))==SF_OK);struct ad_sink fixed={0};struct ad_sink *sink=form==0?&d.sink:form==1?&e.sink:&fixed;sink->mode=mode;sf_write_fn write=form==0?ad_direct_write:form==1?ad_erased_write:ad_fixed_write;void *ctx=form==0?(void*)&d:form==1?(void*)&e:(void*)&fixed;struct sf_stream frame;sf_init(&frame);struct sf_handle h;unsigned char data[3]={1,2,3};CHECK(sf_start(&frame,data,3,&h)==SF_OK);size_t accepted=999;int status=sf_pump(h,write,ctx,1,&accepted);CHECK(status==(mode==1||mode==2?SF_PENDING:mode==3?SF_IO:SF_INVALID)&&accepted==0&&sink->length==0);if(mode<=2){sink->mode=0;CHECK(sf_pump(h,write,ctx,5,&accepted)==SF_DONE&&accepted==5);}else CHECK(sf_pump(h,write,ctx,1,&accepted)==SF_STATE);}
 struct ad_direct d={.kind=99};struct ad_erased e={0};struct ad_ops bad={2,0,ad_one};CHECK(ad_direct_init(&d,3)==SF_INVALID&&d.kind==99);CHECK(ad_erased_init(&e,&bad)==SF_INVALID&&!e.ops);CHECK(ad_direct_write(&d,0,0)==-1&&ad_erased_write(&e,0,0)==-1);
 struct ad_sink sink={0};CHECK(ad_one(&sink,0,1)==-1&&sink.calls==0);sink.length=66;unsigned char byte=1;CHECK(ad_four(&sink,&byte,1)==-1&&sink.calls==0);
 printf("DISPATCH-PASS:conformance=%u;fault_paths=12;third_backend=all;\n",cases);return 0;
}
