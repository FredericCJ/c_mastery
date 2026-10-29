#define _POSIX_C_SOURCE 200809L
#include "dispatch.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#define CHECK(x) do{if(!(x)){fprintf(stderr,"DISPATCH-COST-CHECK:%d\n",__LINE__);exit(1);}}while(0)
static unsigned long long tick(void){struct timespec t;CHECK(clock_gettime(CLOCK_MONOTONIC,&t)==0);return (unsigned long long)t.tv_sec*1000000000ull+(unsigned long long)t.tv_nsec;}
int main(int argc,char **argv){CHECK(argc==4);unsigned form=(unsigned)strtoul(argv[1],0,10),mix=(unsigned)strtoul(argv[2],0,10),n=(unsigned)strtoul(argv[3],0,10);CHECK(form<3&&mix<3&&n<=64&&!(form==2&&mix));unsigned char data[64];unsigned sum=0;for(unsigned i=0;i<n;++i){data[i]=(unsigned char)(i*17+3);sum=(sum+data[i])%256;}
 for(unsigned batch=0;batch<10;++batch){unsigned long long calls=0,offered=0,bytes=0,check=0,start=tick();for(unsigned r=0;r<3000;++r){unsigned kind=mix?r%(mix+1):0;struct ad_direct direct;struct ad_erased erased;struct ad_sink fixed={0};if(form==0){CHECK(ad_direct_init(&direct,kind)==SF_OK);}else if(form==1){CHECK(ad_erased_init(&erased,ad_operations(kind))==SF_OK);}struct ad_sink *sink=form==0?&direct.sink:form==1?&erased.sink:&fixed;void *ctx=form==0?(void*)&direct:form==1?(void*)&erased:(void*)&fixed;sf_write_fn write=form==0?ad_direct_write:form==1?ad_erased_write:ad_fixed_write;struct sf_stream frame;sf_init(&frame);struct sf_handle handle;CHECK(sf_start(&frame,data,n,&handle)==SF_OK);size_t accepted=0;CHECK(sf_pump(handle,write,ctx,100,&accepted)==SF_DONE&&accepted==n+2&&sink->length==n+2);CHECK(sink->bytes[0]==n&&sink->bytes[n+1]==sum);for(unsigned i=0;i<n;++i)CHECK(sink->bytes[i+1]==data[i]);calls+=sink->calls;offered+=sink->offered;bytes+=sink->length;check+=sink->bytes[n+1];}unsigned long long elapsed=tick()-start;
  printf("%u,%u,%u,%u,%llu,%llu,%llu,%llu,%llu,%zu,%zu,%zu,%zu\n",batch,form,mix,n,elapsed,calls,offered,bytes,check,sizeof(struct ad_direct),sizeof(struct ad_erased),sizeof(struct ad_sink),sizeof(struct ad_ops));}
 return 0;}
