#define _POSIX_C_SOURCE 200809L
#include "collection.h"
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define OPERATIONS 256
static uint64_t stamp(void) {
  struct timespec t;
  if(clock_gettime(CLOCK_MONOTONIC,&t)) exit(2);
  return (uint64_t)t.tv_sec*UINT64_C(1000000000)+(uint64_t)t.tv_nsec;
}
static uint64_t deadline(size_t slot, unsigned pattern) {
  if(pattern==1) return 1000+slot%3;
  if(pattern==2) return 1000+64*(slot%61);
  return 1+(slot*37)%8191;
}
static uint64_t fold(uint64_t hash,size_t slot,uint64_t d) {
  return (hash ^ ((uint64_t)slot+1))*UINT64_C(1099511628211) ^ d;
}
static uint64_t oracle(size_t n,unsigned pattern) {
  uint64_t *d=calloc(n,sizeof *d),hash=1;
  if(!d) exit(2);
  for(size_t i=0;i<n;++i) d[i]=deadline(i,pattern);
  for(unsigned step=0;step<OPERATIONS;++step) {
    size_t slot=((size_t)step*73)%n;
    if(step%4==0) d[slot]=0;
    else d[slot]=deadline((size_t)step*7,pattern);
    size_t best=n;
    for(size_t i=0;i<n;++i) if(d[i] && (best==n || d[i]<d[best])) best=i;
    hash=fold(hash,best,best<n?d[best]:0);
  }
  free(d);
  return hash;
}
static int trial(enum TcKind kind,size_t n,unsigned pattern,unsigned repeat,uint64_t expected,int emit) {
  TdTimer *timers=calloc(n,sizeof *timers);
  TcToken *tokens=calloc(n,sizeof *tokens);
  size_t *index=NULL,*pos=NULL,*next=NULL,*prev=NULL,*heads=NULL;
  size_t retained=n*(sizeof *timers+sizeof *tokens);
  if(kind==TC_HEAP) {
    index=calloc(n,sizeof *index); pos=calloc(n,sizeof *pos);
    retained+=n*2*sizeof(size_t);
  } else if(kind==TC_WHEEL) {
    pos=calloc(n,sizeof *pos); next=calloc(n,sizeof *next); prev=calloc(n,sizeof *prev);
    heads=calloc(64,sizeof *heads); retained+=(n*3+64)*sizeof(size_t);
  }
  if(!timers || !tokens || (kind==TC_HEAP && (!index || !pos)) ||
     (kind==TC_WHEEL && (!pos || !next || !prev || !heads))) exit(2);
  TcCollection q;
  uint64_t preparation=stamp();
  int valid=tc_init(&q,kind,timers,n,index,pos,next,prev,heads,64,16,65535)==0;
  for(size_t i=0;i<n;++i) valid &= tc_arm(&q,i,deadline(i,pattern),0,0,tokens+i)==1;
  preparation=stamp()-preparation;
  if(!valid) exit(1);
  uint64_t hash=1,start=stamp();
  for(unsigned step=0;step<OPERATIONS;++step) {
    size_t slot=((size_t)step*73)%n;
    if(step%4==0) (void)tc_cancel(&q,tokens[slot]);
    else (void)tc_arm(&q,slot,deadline((size_t)step*7,pattern),0,0,tokens+slot);
    TcToken found={n,0};
    uint64_t d=0;
    (void)tc_peek(&q,&found,&d);
    hash=fold(hash,found.slot,d);
  }
  uint64_t elapsed=stamp()-start;
  valid=hash==expected;
  if(emit) printf("{\"kind\":%d,\"n\":%zu,\"pattern\":%u,\"trial\":%u,\"operations\":%u,\"elapsed_ns\":%" PRIu64 ",\"preparation_ns\":%" PRIu64 ",\"retained_bytes\":%zu,\"checksum\":%" PRIu64 ",\"valid\":%s}\n",
      kind,n,pattern,repeat,OPERATIONS,elapsed,preparation,retained,hash,valid?"true":"false");
  free(timers);free(tokens);free(index);free(pos);free(next);free(prev);free(heads);
  return valid?0:1;
}
int main(void) {
  const size_t sizes[]={16,256,4096};
  for(size_t i=0;i<3;++i) for(unsigned pattern=0;pattern<3;++pattern) {
    uint64_t expected=oracle(sizes[i],pattern);
    for(unsigned k=0;k<3;++k) if(trial((enum TcKind)k,sizes[i],pattern,0,expected,0)) return 1;
    for(unsigned repeat=0;repeat<31;++repeat) for(unsigned offset=0;offset<3;++offset) {
      enum TcKind kind=(enum TcKind)((repeat+offset)%3);
      if(trial(kind,sizes[i],pattern,repeat,expected,1)) return 1;
    }
  }
  return 0;
}
