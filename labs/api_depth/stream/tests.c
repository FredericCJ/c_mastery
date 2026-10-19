#include "stream.h"
#include <stdio.h>
#include <string.h>
#define C(x) do{if(!(x)){fprintf(stderr,"SF_CHECK:%d:%s\n",__LINE__,#x);return 1;}}while(0)
typedef struct{unsigned char data[4096];size_t used,quota;unsigned calls;ptrdiff_t special;} Sink;
static ptrdiff_t write_sink(void*v,const unsigned char*b,size_t n){Sink*s=v;s->calls++;if(s->special)return s->special;if(n>s->quota)n=s->quota;if(s->used+n>sizeof s->data)return -1;memcpy(s->data+s->used,b,n);s->used+=n;return (ptrdiff_t)n;}
int main(void){
 for(size_t n=0;n<=64;n++)for(size_t quota=1;quota<=7;quota++){
  unsigned char input[64];unsigned sum=0;for(size_t i=0;i<n;i++){input[i]=(unsigned char)(3*i+1);sum=(sum+input[i])&255u;}
  SFStream s;sf_init(&s);SFHandle h;C(sf_start(&s,input,n,&h)==0);Sink sink={.quota=quota};size_t accepted=91;
  C(sf_pump(h,write_sink,&sink,0,&accepted)==SF_PENDING&&accepted==0&&!sink.calls);
  for(unsigned calls=0;calls<67;calls++){int r=sf_pump(h,write_sink,&sink,1,&accepted);C(r==SF_PENDING||r==SF_DONE);if(r==SF_DONE)break;}
  C(sink.used==n+2&&sink.data[0]==n&&sink.data[n+1]==sum&&memcmp(input,sink.data+1,n)==0&&accepted==n+2);
  unsigned calls=sink.calls;C(sf_pump(h,write_sink,&sink,1,&accepted)==SF_DONE&&sink.calls==calls);
 }
 SFStream s;sf_init(&s);SFHandle old,h;const unsigned char input[]={1,2,3};C(sf_start(&s,input,3,&old)==0);Sink sink={.quota=2};size_t accepted=91;
 C(sf_pump(old,write_sink,&sink,1,&accepted)==SF_PENDING&&accepted==2);sink.quota=0;C(sf_pump(old,write_sink,&sink,99,&accepted)==SF_PENDING&&sink.calls==2&&accepted==2);
 sink.special=-2;C(sf_pump(old,write_sink,&sink,3,&accepted)==SF_PENDING&&sink.calls==5&&accepted==2);
 C(sf_cancel(old)==0);C(sf_pump(old,write_sink,&sink,99,&accepted)==SF_CANCELLED&&sink.calls==5&&accepted==2);
 C(sf_start(&s,input,3,&h)==0&&h.generation!=old.generation);accepted=91;C(sf_pump(old,write_sink,&sink,1,&accepted)==SF_INVALID&&accepted==91);
 sink=(Sink){.quota=2};C(sf_pump(h,write_sink,&sink,1,&accepted)==SF_PENDING&&accepted==2);sink.special=-1;
 C(sf_pump(h,write_sink,&sink,1,&accepted)==SF_IO&&accepted==2&&s.offset==2);C(sf_pump(h,write_sink,&sink,1,&accepted)==SF_STATE);
 s.generation=UINT32_MAX;h=(SFHandle){0};C(sf_start(&s,input,3,&h)==SF_RANGE&&!h.owner);
 puts("SF_STREAM:455 bounded payload/quota pairs;zero/retry/error/cancel/retirement traces");return 0;
}
