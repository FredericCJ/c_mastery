#include "match.h"
int mt_init(MTStream*out,const unsigned char*p,size_t n){
 if(!out)return AL_INVALID;
 MTStream next={0};int status=al_stream_init(&next.prefix,p,n);if(status!=AL_OK)return status;*out=next;return AL_OK;
}
int mt_drain(MTStream*s,ALPublish cb,void*ctx,MTReport*out){
 if(!s||!cb||!out)return AL_INVALID;
 if(s->state==2)return MT_CLOSED;
 MTReport r={0};
 while(s->next<s->count){++r.attempted;int status=cb(ctx,s->pending[s->next]);
  if(status){
#ifdef MT_MUTANT_SKIP
   ++s->next;
#endif
   if(status==2){
#ifndef MT_MUTANT_CANCEL
    s->state=2;s->next=s->count;*out=r;return AL_CANCELLED;
#endif
   }else{*out=r;return AL_FAILED;}
  }else ++r.accepted;
  ++s->next;
 }
 s->state=0;s->next=s->count=0;*out=r;return AL_OK;
}
int mt_feed(MTStream*s,const unsigned char*p,size_t n,ALPublish cb,void*ctx,MTReport*out){
 if(!s||!cb||!out)return AL_INVALID;
 if(s->state==2)return MT_CLOSED;
#ifndef MT_MUTANT_REPLACE
 if(s->state==1)return MT_BUSY;
#endif
 size_t count=0;int status=al_stream_feed(&s->prefix,p,n,s->pending,65,&count);if(status!=AL_OK)return status;
 s->count=count;s->next=0;s->state=1;return mt_drain(s,cb,ctx,out);
}
