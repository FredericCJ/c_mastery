#include "object_runtime_rc.h"
#ifndef ORR_MUTANT
#define ORR_MUTANT 0
#endif
static uint16_t bit(unsigned i) { return (uint16_t)(1u<<i); }
static int valid(const ORRHeap *h, ORRHandle a) {
  return a.slot>=1 && a.slot<=16 && a.generation>=1 && a.generation<=3 &&
    h->generation[a.slot-1]==a.generation && (h->counts.live & bit(a.slot-1));
}
static int target(const ORRHeap *h, ORRHandle a) {
  return (!a.slot && !a.generation) || valid(h,a);
}
static ORStatus admission(const ORRHeap *h) {
  if(h->closed) return OR_STATE;
  return h->revision==65535 ? OR_FULL : OR_OK;
}
static void candidate(ORRHeap *h,unsigned i) {
  if(ORR_MUTANT!=4) h->candidates |= bit(i);
}
static void retire(ORRHeap *h,unsigned i) {
  h->counts.live &= (uint16_t)~bit(i);
  h->candidates &= (uint16_t)~bit(i);
  h->counts.refs[i]=0; h->roots[i]=0; h->generation[i]++;
  h->counts.edge[i][0]=0; h->counts.edge[i][1]=0;
}
static uint16_t decrement(ORRHeap *h,unsigned i) {
  uint16_t dead=0; unsigned j,k;
  h->counts.refs[i]--;
  if(h->counts.refs[i]) candidate(h,i);
  for(;;) {
    for(j=0;j<16;j++) if((h->counts.live & bit(j)) && !h->counts.refs[j]) break;
    if(j==16) break;
    for(k=0;k<2;k++) {
      unsigned t=h->counts.edge[j][k];
      if(t && ORR_MUTANT!=1) {
        h->counts.refs[t-1]--;
        if(h->counts.refs[t-1]) candidate(h,t-1);
      }
    }
    dead |= bit(j); retire(h,j);
  }
  return dead;
}
static int queue_full(const ORRHeap *h) { return h->length==8 && ORR_MUTANT!=3; }
static void enqueue(ORRHeap *h,unsigned slot) {
  /* Mutant3 silently drops the oldest obligation rather than writing out of bounds. */
  if(h->length==8) { h->head=(uint8_t)((h->head+1)%8); h->length--; }
  h->pending[(h->head+h->length)%8]=(uint8_t)slot; h->length++;
}
void orr_init(ORRHeap *h) {
  unsigned i; *h=(ORRHeap){0};h->revision=1;
  for(i=0;i<16;i++) h->generation[i]=1;
}
ORStatus orr_new(ORRHeap *h,ORRHandle *out) {
  unsigned i;ORStatus s;
  if(!h || !out) return OR_INVALID;
  s=admission(h);if(s!=OR_OK) return s;
  for(i=0;i<16;i++) if(!(h->counts.live&bit(i)) && h->generation[i]<=3) {
    h->counts.live |= bit(i);h->counts.refs[i]=1;h->roots[i]=1;h->revision++;
    *out=(ORRHandle){(uint8_t)(i+1),h->generation[i]};return OR_OK;
  }
  return OR_FULL;
}
ORStatus orr_retain(ORRHeap *h,ORRHandle a) {
  ORStatus s;if(!h) return OR_INVALID;s=admission(h);if(s!=OR_OK)return s;
  if(!valid(h,a)) return OR_INVALID;
  if(h->roots[a.slot-1]==255) return OR_FULL;
  h->roots[a.slot-1]++;h->counts.refs[a.slot-1]++;h->revision++;return OR_OK;
}
ORStatus orr_release(ORRHeap *h,ORRHandle a,int defer,uint16_t *out) {
  ORStatus s;uint16_t dead=0;
  if(!h || !out || (defer!=0 && defer!=1)) return OR_INVALID;
  s=admission(h);if(s!=OR_OK)return s;
  if(!valid(h,a)) return OR_INVALID;
  if(!h->roots[a.slot-1] && ORR_MUTANT!=2) return OR_STATE;
  if(defer && queue_full(h)) return OR_FULL;
  h->roots[a.slot-1]--;
  if(defer) enqueue(h,a.slot); else dead=decrement(h,a.slot-1);
  h->revision++;*out=dead;return OR_OK;
}
ORStatus orr_store(ORRHeap *h,ORRHandle a,unsigned field,ORRHandle b,int defer,uint16_t *out) {
  ORStatus s;unsigned old;uint16_t dead=0;
  if(!h || !out || field>1 || (defer!=0 && defer!=1)) return OR_INVALID;
  s=admission(h);if(s!=OR_OK)return s;
  if(!valid(h,a) || !target(h,b)) return OR_INVALID;
  old=h->counts.edge[a.slot-1][field];
  if(old!=b.slot) {
    if(old && defer && queue_full(h)) return OR_FULL;
    if(b.slot) h->counts.refs[b.slot-1]++;
    h->counts.edge[a.slot-1][field]=b.slot;
    if(old) { if(defer) enqueue(h,old);else dead=decrement(h,old-1); }
  }
  h->revision++;*out=dead;return OR_OK;
}
ORStatus orr_flush(ORRHeap *h,unsigned budget,unsigned *processed,uint16_t *out) {
  ORStatus s;unsigned n=0;uint16_t dead=0;
  if(!h || !processed || !out)return OR_INVALID;
  s=admission(h);if(s!=OR_OK)return s;
  while(n<budget && h->length) {
    unsigned slot=h->pending[h->head];h->pending[h->head]=0;
    h->head=(uint8_t)((h->head+1)%8);h->length--;n++;
    dead |= decrement(h,slot-1);
  }
  if(n)h->revision++;
  *processed=n;*out=dead;return h->length?OR_OK:OR_DONE;
}
ORStatus orr_prepare(ORRHeap *h,ORRTrial *out) {
  ORRTrial t;uint16_t closure,old;unsigned i,j;ORStatus s;
  if(!h || !out)return OR_INVALID;
  if(h->closed || (h->length && ORR_MUTANT!=6))return OR_STATE;
  closure=h->candidates;
  do {
    old=closure;
    for(i=0;i<16;i++) if(closure&bit(i)) for(j=0;j<2;j++) {
      unsigned child=h->counts.edge[i][j];if(child)closure |= bit(child-1);
    }
  } while(closure!=old);
  t=(ORRTrial){0};t.owner=h;t.result=h->counts;t.revision=h->revision;
  s=or_trial_delete(&t.result,closure,&t.dead);if(s!=OR_OK)return s;
  *out=t;return OR_OK;
}
ORStatus orr_commit(ORRHeap *h,const ORRTrial *t,uint16_t *out) {
  ORStatus s;unsigned i;
  if(!h || !t || !out)return OR_INVALID;
  s=admission(h);if(s!=OR_OK)return s;
  if(t->owner!=h || h->length || (t->revision!=h->revision && ORR_MUTANT!=5))return OR_STATE;
  h->counts=t->result;
  for(i=0;i<16;i++)if(t->dead&bit(i))retire(h,i);
  h->candidates=0;h->revision++;*out=t->dead;return OR_OK;
}
ORStatus orr_close(ORRHeap *h) {
  if(!h)return OR_INVALID;
  if(h->closed)return OR_STATE;
  h->counts=(ORCounts){0};h->length=0;h->candidates=0;h->closed=1;return OR_OK;
}
int orr_invariant(const ORRHeap *h) {
  unsigned sums[16]={0},i,j;
  if(!h || h->closed || !h->revision || h->length>8 || h->head>7 ||
     (h->candidates & ~h->counts.live))return 0;
  for(i=0;i<16;i++) {
    if(h->generation[i]<1 || h->generation[i]>4)return 0;
    if(h->counts.live&bit(i)) {
      if(h->generation[i]>3 || !h->counts.refs[i])return 0;
      sums[i]+=h->roots[i];
      for(j=0;j<2;j++) {
        unsigned t=h->counts.edge[i][j];
        if(t>16 || (t && !(h->counts.live&bit(t-1))))return 0;
        if(t)sums[t-1]++;
      }
    } else if(h->roots[i] || h->counts.refs[i] || h->counts.edge[i][0] || h->counts.edge[i][1])return 0;
  }
  for(i=0;i<h->length;i++) {
    unsigned t=h->pending[(h->head+i)%8];
    if(!t || t>16 || !(h->counts.live&bit(t-1)))return 0;
    sums[t-1]++;
  }
  for(i=0;i<16;i++)if(sums[i]!=h->counts.refs[i])return 0;
  return 1;
}
