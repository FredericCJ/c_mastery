#include "match.h"
#define CHECK(x) do{if(!(x))return mt_failure(#x,__LINE__);}while(0)
typedef struct {uint64_t values[128];size_t count,attempts,stop;int result;} Sink;
static int publish(void*ctx,uint64_t offset){Sink*s=ctx;size_t at=s->attempts++;if(at==s->stop)return s->result;s->values[s->count++]=offset;return 0;}
static int retries(void){
 const unsigned char pattern[]={'a','a','a'},input[]={'a','a','a','a','a','a','a'};unsigned cases=0;
 for(unsigned split=0;split<=7;++split)for(unsigned stop=0;stop<5;++stop){MTStream s;MTReport r;Sink sink={.stop=stop,.result=1};CHECK(mt_init(&s,pattern,3)==AL_OK);int status=mt_feed(&s,input,split,publish,&sink,&r);
  if(status==AL_FAILED){uint64_t consumed=s.prefix.consumed;size_t saved=s.next;MTReport untouched={99,99};CHECK(mt_feed(&s,input+split,7-split,publish,&sink,&untouched)==MT_BUSY);CHECK(s.prefix.consumed==consumed&&s.next==saved&&untouched.accepted==99);CHECK(mt_drain(&s,publish,&sink,&r)==AL_OK);}
  else CHECK(status==AL_OK);
  status=mt_feed(&s,input+split,7-split,publish,&sink,&r);if(status==AL_FAILED)CHECK(mt_drain(&s,publish,&sink,&r)==AL_OK);else CHECK(status==AL_OK);
  CHECK(sink.count==5);for(unsigned i=0;i<5;++i)CHECK(sink.values[i]==i);CHECK(s.prefix.consumed==7&&s.state==0);CHECK(mt_drain(&s,publish,&sink,&r)==AL_OK&&r.attempted==0);++cases;
 }
 mt_observation("retry",cases,(unsigned)sizeof(MTStream));return 0;
}
static int cancellation(void){unsigned cases=0;const unsigned char p[]={'a'},input[]={'a','a','a'};
 for(size_t stop=0;stop<3;++stop){MTStream s;MTReport r;Sink sink={.stop=stop,.result=2};CHECK(mt_init(&s,p,1)==AL_OK);CHECK(mt_feed(&s,input,3,publish,&sink,&r)==AL_CANCELLED);CHECK(sink.count==stop&&sink.attempts==stop+1&&r.accepted==stop&&r.attempted==stop+1);CHECK(s.prefix.consumed==3&&s.state==2);r=(MTReport){99,99};CHECK(mt_drain(&s,publish,&sink,&r)==MT_CLOSED&&r.accepted==99);CHECK(mt_feed(&s,input,3,publish,&sink,&r)==MT_CLOSED);CHECK(mt_init(&s,p,1)==AL_OK);sink=(Sink){.stop=99};CHECK(mt_feed(&s,input,3,publish,&sink,&r)==AL_OK&&sink.count==3);++cases;}
 MTStream s;MTReport r;Sink sink={.stop=99};CHECK(mt_init(&s,NULL,0)==AL_OK);CHECK(mt_feed(&s,NULL,0,publish,&sink,&r)==AL_OK&&sink.count==1);CHECK(mt_feed(&s,NULL,0,publish,&sink,&r)==AL_OK&&sink.count==1);CHECK(mt_feed(&s,input,3,publish,&sink,&r)==AL_OK&&sink.count==4);for(unsigned i=0;i<4;++i)CHECK(sink.values[i]==i);++cases;
 unsigned char maximum[64];for(unsigned i=0;i<64;++i)maximum[i]='a';sink=(Sink){.stop=64,.result=1};CHECK(mt_init(&s,NULL,0)==AL_OK);CHECK(mt_feed(&s,maximum,64,publish,&sink,&r)==AL_FAILED&&sink.count==64);CHECK(mt_drain(&s,publish,&sink,&r)==AL_OK&&sink.count==65&&sink.values[64]==64);++cases;
 CHECK(mt_init(&s,p,1)==AL_OK);s.prefix.consumed=s.prefix.prefix.consumed=UINT64_MAX;sink=(Sink){.stop=99};r=(MTReport){99,99};CHECK(mt_feed(&s,input,1,publish,&sink,&r)==AL_OVERFLOW&&sink.attempts==0&&r.accepted==99);++cases;
 mt_observation("cancel",cases,(unsigned)sizeof s);return 0;
}
static int enumeration(void){unsigned cases=0;unsigned char text[5],pattern[3];
 for(unsigned n=0;n<=5;++n)for(unsigned bits=0;bits<(1u<<n);++bits){for(unsigned i=0;i<n;++i)text[i]=(unsigned char)((bits>>i)&1u);
  for(unsigned m=0;m<=3;++m)for(unsigned pb=0;pb<(1u<<m);++pb){for(unsigned i=0;i<m;++i)pattern[i]=(unsigned char)((pb>>i)&1u);uint64_t expected[6];unsigned count=0;if(m<=n)for(unsigned start=0;start<=n-m;++start){unsigned i=0;while(i<m&&text[start+i]==pattern[i])++i;if(i==m)expected[count++]=start;}
   for(unsigned split=0;split<=n;++split){MTStream s;MTReport r;Sink sink={.stop=99};CHECK(mt_init(&s,pattern,m)==AL_OK);CHECK(mt_feed(&s,text,split,publish,&sink,&r)==AL_OK);CHECK(mt_feed(&s,text+split,n-split,publish,&sink,&r)==AL_OK);CHECK(sink.count==count);for(unsigned i=0;i<count;++i)CHECK(sink.values[i]==expected[i]);++cases;}
  }
 }
 mt_observation("enumeration",cases,(unsigned)sizeof(MTStream));return 0;
}
int mt_tests(unsigned n){return n==0?retries():n==1?cancellation():enumeration();}
