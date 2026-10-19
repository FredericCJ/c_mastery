#include "h745_cache.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define C(x) do { if(!(x)) {fprintf(stderr,"B06_H745_CHECK:%s:%d\n",#x,__LINE__);exit(1);} }while(0)
typedef struct {enum FWCacheOp ops[131];uint32_t addresses[131];unsigned n;} Trace;
static void emit(void *p,enum FWCacheOp op,uint32_t a) {Trace *t=p;C(t->n<131);t->ops[t->n]=op;t->addresses[t->n++]=a;}
static void reject(FWPeer *p,unsigned c,uint32_t g,enum FWPeerOp op,enum FWPeerResult want)
{ FWPeer old=*p;C(fw_peer_step(p,c,g,op)==want);C(!memcmp(&old,p,sizeof old)); }
int main(void)
{
    for(unsigned mode=FW_CLEAN;mode<=H7_INVALIDATE;++mode)
      for(uint32_t n=32;n<=4096;n+=32) {
        Trace t={0};C(fw_cache_range(0x30040000u,n,(enum FWCacheOp)mode,emit,&t));
        C(t.n==n/32+3);C(t.ops[0]==FW_DSB && t.ops[t.n-2]==FW_DSB && t.ops[t.n-1]==FW_ISB);
        for(unsigned i=1;i<t.n-2;++i)C(t.ops[i]==(enum FWCacheOp)mode && t.addresses[i]==0x30040000u+32u*(i-1));
      }
    Trace t={0};C(fw_cache_range(UINT32_MAX,0,FW_CLEAN,emit,&t));C(!t.n);
    C(fw_cache_range(0xffffffe0u,32,FW_CLEAN,emit,&t));C(t.addresses[1]==0xffffffe0u);t.n=0;
    C(!fw_cache_range(0xffffffe0u,64,FW_CLEAN,emit,&t));
    C(!fw_cache_range(1,32,H7_INVALIDATE,emit,&t));C(!fw_cache_range(32,33,FW_CLEAN,emit,&t));C(!t.n);
    C(!fw_cache_range(32,4128,FW_CLEAN,emit,&t));C(!fw_cache_range(32,32,FW_DSB,emit,&t));
    for(unsigned first=0;first<2;++first) {
      FWPeer p={.generation=9};reject(&p,first,8,FW_BEGIN,H7_STALE);
      C(fw_peer_step(&p,first,9,FW_BEGIN)==H7_OK);reject(&p,first,9,FW_HEALTHY,H7_BUSY);
      C(fw_peer_step(&p,first,9,FW_END)==H7_OK);C(fw_peer_step(&p,first,9,FW_HEALTHY)==H7_OK);
      reject(&p,first,9,FW_CONFIRM,H7_BUSY);C(fw_peer_step(&p,first,9,FW_FAIL_PEER)==H7_OK);
      reject(&p,1-first,9,FW_HEALTHY,H7_FAILED);reject(&p,first,9,FW_CONFIRM,H7_FAILED);C(!p.confirmed);
      p=(FWPeer){.generation=10,.health=3,.outstanding={1,0}};
      reject(&p,first,10,FW_CONFIRM,H7_BUSY);C(fw_peer_step(&p,0,10,FW_END)==H7_OK);
      C(fw_peer_step(&p,first,10,FW_CONFIRM)==H7_OK);C(p.confirmed);
      reject(&p,first,10,FW_BEGIN,H7_BUSY);C(fw_peer_step(&p,first,10,FW_FAIL_PEER)==H7_OK);C(!p.confirmed);
      p=(FWPeer){.generation=11,.outstanding={UINT32_MAX,0}};reject(&p,0,11,FW_BEGIN,H7_BUSY);
      p=(FWPeer){.generation=11,.outstanding={1,0}};C(fw_peer_step(&p,1,11,FW_FAIL_PEER)==H7_OK);
      C(fw_peer_step(&p,0,11,FW_END)==H7_OK);reject(&p,0,11,FW_BEGIN,H7_FAILED);
    }
    /* Independent physical-line counterexample: invalidating a rounded payload
       line loses an unrelated dirty byte. The range contract rejects that view. */
    unsigned char ram[32]={0},cache[32]={0};cache[31]=91;memcpy(cache,ram,32);C(cache[31]!=91);
    puts("B06_H745_PASS:256 ranges, boundaries, peer failure and dirty-neighbour witness");
}
