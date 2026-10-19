#include "object_runtime_finalizers.h"
#ifndef ORF_MUTANT
#define ORF_MUTANT 0
#endif
static uint16_t bit(unsigned i) { return (uint16_t)(1u << i); }
static int valid(const ORFHeap *h, ORFHandle a) {
  return a.slot >= 1 && a.slot <= 16 && a.generation >= 1 &&
         a.generation <= 3 && h->generation[a.slot-1] == a.generation &&
         (h->weak.graph.live & bit(a.slot-1));
}
static int target(const ORFHeap *h, ORFHandle a) {
  return (!a.slot && !a.generation) || valid(h,a);
}
void orf_init(ORFHeap *h) {
  unsigned i;
  *h = (ORFHeap){0};
  for(i=0;i<16;i++) h->generation[i]=1;
}
ORStatus orf_new(ORFHeap *h, ORFFinalizer cb, void *ctx, int resource,
                 int fail, ORFHandle *out) {
  unsigned i;
  if(!h || !out || (resource!=0 && resource!=1) || (fail!=0 && fail!=1) ||
     (resource && !cb)) return OR_INVALID;
  if(h->closed) return OR_STATE;
  if(fail) return OR_FULL;
  for(i=0;i<16;i++) if(!(h->weak.graph.live & bit(i)) && h->generation[i]<=3) {
    h->weak.graph.live |= bit(i); h->weak.graph.roots |= bit(i);
    h->weak.graph.edge[i][0]=0; h->weak.graph.edge[i][1]=0;
    h->weak.graph.color[i]=0;
    h->weak.finalized &= (uint16_t)~bit(i);
    h->weak.finalizable &= (uint16_t)~bit(i);
    if(cb) h->weak.finalizable |= bit(i);
    h->callback[i]=cb; h->context[i]=ctx;
    if(resource) h->resources |= bit(i);
    *out=(ORFHandle){(uint8_t)(i+1),h->generation[i]}; return OR_OK;
  }
  return OR_FULL;
}
ORStatus orf_root(ORFHeap *h, ORFHandle a, int present) {
  if(!h || (present!=0 && present!=1)) return OR_INVALID;
  if(h->closed) return OR_STATE;
  if(!valid(h,a)) return OR_INVALID;
  return or_graph_root(&h->weak.graph,a.slot,present);
}
ORStatus orf_edge(ORFHeap *h, ORFHandle a, unsigned field, ORFHandle b) {
  if(!h || field>1) return OR_INVALID;
  if(h->closed) return OR_STATE;
  if(!valid(h,a) || !target(h,b)) return OR_INVALID;
  return or_graph_store(&h->weak.graph,a.slot,field,b.slot);
}
ORStatus orf_pair(ORFHeap *h, unsigned index, ORFHandle a, ORFHandle b) {
  if(!h || index>=4) return OR_INVALID;
  if(h->closed) return OR_STATE;
  if(!target(h,a) || !target(h,b) || (!a.slot != !b.slot)) return OR_INVALID;
  h->weak.key[index]=a.slot; h->weak.value[index]=b.slot; return OR_OK;
}
ORStatus orf_weak(ORFHeap *h, unsigned index, ORFHandle a) {
  if(!h || index>=4) return OR_INVALID;
  if(h->closed) return OR_STATE;
  if(!target(h,a)) return OR_INVALID;
  h->weak.weak[index]=a.slot; return OR_OK;
}
ORStatus orf_collect(ORFHeap *h, uint16_t *out) {
  ORWeak staged; uint16_t dead; unsigned i; ORStatus s;
  if(!h || !out) return OR_INVALID;
  if(h->closed || (h->busy && ORF_MUTANT!=1) || h->cleanup_pending)
    return OR_STATE;
  staged=h->weak;
  if(ORF_MUTANT==3) for(i=0;i<4;i++)
    if(staged.value[i]) staged.graph.roots |= bit(staged.value[i]-1);
  s=or_weak_collect(&staged,&dead);
  if(s!=OR_OK) return s;
  if(dead & h->resources) return OR_STATE;
  h->weak=staged;
  for(i=0;i<16;i++) if(dead & bit(i)) {
    h->generation[i]++; h->callback[i]=0; h->context[i]=0;
    h->weak.graph.edge[i][0]=0; h->weak.graph.edge[i][1]=0;
    h->weak.graph.color[i]=0;
  }
  *out=dead; return OR_OK;
}
ORStatus orf_run_one(ORFHeap *h, ORFHandle *out, ORStatus *status) {
  unsigned slot; ORStatus s; ORFHandle a;
  if(!h || !out || !status) return OR_INVALID;
  if(h->closed || h->busy) return OR_STATE;
  s=or_final_next(&h->weak,&slot); if(s!=OR_OK) return s;
  a=(ORFHandle){(uint8_t)slot,h->generation[slot-1]};
  h->busy=1; h->callbacks_run++;
  s=h->callback[slot-1](h,a,h->context[slot-1]);
  h->busy=0;
  if((h->resources & bit(slot-1)) && (ORF_MUTANT!=2 || s==OR_OK))
    h->cleanup_pending |= bit(slot-1);
  *out=a; *status=s; return OR_OK;
}
ORStatus orf_close_resource(ORFHeap *h, ORFHandle a) {
  uint16_t mask;
  if(!h) return OR_INVALID;
  if(h->closed) return OR_STATE;
  if(!valid(h,a)) return OR_INVALID;
  mask=bit(a.slot-1); if(!(h->resources & mask)) return OR_STATE;
  h->resources &= (uint16_t)~mask;
  h->cleanup_pending &= (uint16_t)~mask;
  h->resources_closed++; return OR_OK;
}
ORStatus orf_cleanup(ORFHeap *h, unsigned budget, unsigned *out) {
  unsigned i, n=0;
  if(!h || !out) return OR_INVALID;
  if(h->closed || h->busy) return OR_STATE;
  for(i=0;i<16 && n<budget;i++) if(h->cleanup_pending & bit(i)) {
    ORFHandle a={(uint8_t)(i+1),h->generation[i]};
    (void)orf_close_resource(h,a); n++;
  }
  *out=n; return h->cleanup_pending ? OR_OK : OR_DONE;
}
ORStatus orf_close(ORFHeap *h) {
  unsigned i;
  if(!h) return OR_INVALID;
  if(h->closed || h->busy) return OR_STATE;
  for(i=0;i<16;i++) if(h->resources & bit(i)) {
    ORFHandle a={(uint8_t)(i+1),h->generation[i]};
    (void)orf_close_resource(h,a);
  }
  h->weak=(ORWeak){0}; h->closed=1; return OR_OK;
}
