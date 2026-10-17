#include "core.h"
#ifndef B06_MUTATION
#define B06_MUTATION 0
#endif

int li_advance(const size_t *a,size_t n,LIPosition *p,size_t bytes){size_t i,off,total=0,b=bytes;LIPosition q=*p;if(q.index>n||(q.index==n&&q.offset)|| (q.index<n&&q.offset>a[q.index]))return 0;for(i=q.index;i<n;++i){off=i==q.index?q.offset:0;if(a[i]-off>SIZE_MAX-total)return 0;total+=a[i]-off;}if(bytes>total)return 0;while(q.index<n){size_t rem=a[q.index]-q.offset;if(b<rem){q.offset+=b;b=0;break;}b-=rem;++q.index;q.offset=0;}if(B06_MUTATION==1&&bytes)q.offset=0;*p=q;return 1;}
int li_ready_after(int r,unsigned budget){if(budget>1)return 0;return r>0&&!(B06_MUTATION==2&&budget);}
int li_slot(uint32_t h,uint32_t t,uint32_t c,uint32_t *o){if(!c||c>(UINT32_C(1)<<30)||(c&(c-1)))return 0;if(t-h>=c&&B06_MUTATION!=3)return 0;*o=t&(c-1);return 1;}
int li_event(LIRequest *r,uint32_t g,unsigned e,int result){if(g!=r->generation&&B06_MUTATION!=4)return 0;if(e==0){if(r->state!=0)return 0;r->state=1;}else if(e==1){if(r->state!=1)return 0;r->state=2;}else if(e==2){if(r->state!=1&&r->state!=2)return 0;r->state=3;r->result=result;}else if(e==3){if(r->state!=3)return 0;r->state=4;}else return 0;return 1;}
int li_releasable(unsigned ack,unsigned done){if(ack>1||done>1)return 0;return done||(B06_MUTATION==5&&ack);}
int li_expired(uint32_t s,uint32_t n,uint32_t d,unsigned *o){if(!d||d>INT32_MAX)return 0;*o=B06_MUTATION==6?n>=s+d:n-s>=d;return 1;}
int li_durable(unsigned w,unsigned f,unsigned r,unsigned d){return w==1&&f==1&&r==1&&(d==1||B06_MUTATION==7);}
int li_buffer_release(unsigned s,unsigned t,unsigned b){return s<=1&&t<=1&&(!s||t)&&(b==0||B06_MUTATION==8);}
