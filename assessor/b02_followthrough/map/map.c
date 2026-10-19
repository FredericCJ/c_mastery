#include "map.h"
#ifndef BF_MUTANT
#define BF_MUTANT 0
#endif
int bf_map_rehash(B2Map*m,B2Entry*s,size_t capacity,unsigned fail_after){
 if(!m||!s||capacity<7||fail_after>7)return B2_INVALID;
 for(size_t i=0;i<7;++i){s[i].state=0;s[i].key=0;s[i].value=0;}
 unsigned staged=0;
 for(size_t i=0;i<7;++i)if(m->slots[i].state==1){
  size_t at=m->slots[i].key%7u;
  for(unsigned visit=0;visit<7&&s[at].state;++visit)at=(at+1)%7;
  if(BF_MUTANT!=3||staged!=0)s[at]=m->slots[i];
  ++staged;
  if(staged==fail_after){
   if(BF_MUTANT==2){for(size_t j=0;j<7;++j)m->slots[j]=s[j];m->count=staged;}
   return B2_FAILED;
  }
 }
 for(size_t i=0;i<7;++i)m->slots[i]=s[i];
 return B2_OK;
}
static unsigned hash(const unsigned char*p,size_t n){unsigned h=0;for(size_t i=0;i<n;++i)h=(h*33u+p[i])%4;return h;}
static int equal(const BfBorrowMap*m,unsigned at,const unsigned char*p,size_t n){if(m->length[at]!=n)return 0;for(size_t i=0;i<n;++i)if(m->bytes[at][i]!=p[i])return 0;return 1;}
int bf_borrow_put(BfBorrowMap*m,const unsigned char*p,size_t n,int value){
 if(!m||(!p&&n)||n>8)return B2_INVALID;
 unsigned at=hash(p,n);
 for(unsigned i=0;i<4;++i){if(!m->live[at]||equal(m,at,p,n)){m->bytes[at]=p;m->length[at]=n;m->value[at]=value;m->live[at]=1;return B2_OK;}at=(at+1)%4;}
 return B2_FULL;
}
int bf_borrow_get(const BfBorrowMap*m,const unsigned char*p,size_t n,int*out){
 if(!m||!out||(!p&&n)||n>8)return B2_INVALID;
 unsigned at=hash(p,n);
 for(unsigned i=0;i<4;++i){if(!m->live[at])return B2_ABSENT;if(equal(m,at,p,n)){*out=m->value[at];return B2_OK;}at=(at+1)%4;}
 return B2_ABSENT;
}
