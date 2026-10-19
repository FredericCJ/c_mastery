#include "frontier.h"
#include <limits.h>
_Static_assert(INT_MAX>=524279,"encoded bounded distance and vertex fit prior priority");
void ag_bfs_snapshot(AGTrace*trace,const ADTraversal*t,unsigned seen,const CDRing*ring){
 AGStep*s=&trace->bfs[trace->bfs_count++];s->done=0;for(unsigned i=0;i<t->count;++i)s->done|=1u<<t->order[i];s->seen=seen;s->count=(unsigned)ring->count;
 for(unsigned i=0;i<8;++i)s->distance[i]=t->distance[i]==UINT32_MAX?UINT64_MAX:t->distance[i];
 CDSpans spans;if(cd_ring_read(ring,&spans)!=CD_OK)return;unsigned at=0;for(size_t i=0;i<spans.first_count;++i)s->frontier[at++]=(unsigned)spans.first[i];for(size_t i=0;i<spans.second_count;++i)s->frontier[at++]=(unsigned)spans.second[i];s->selected=s->frontier[0];
}
void ag_weight_snapshot(AGTrace*trace,const uint64_t distance[8],unsigned done,const CDHeap*heap){
 AGStep*s=&trace->weighted[trace->weighted_count++];s->done=done;s->count=heap->count;s->seen=done;
 for(unsigned i=0;i<8;++i)s->distance[i]=distance[i];
 for(unsigned i=0;i<heap->count;++i){s->frontier[i]=heap->data[i].identity;s->priority[i]=heap->data[i].priority;s->seen|=1u<<s->frontier[i];}s->selected=s->frontier[0];
}
void ag_relax_snapshot(AGTrace*trace,unsigned edge,unsigned from,unsigned to,uint64_t before,uint64_t after){trace->relaxation[trace->relaxations++]=(AGRelax){edge,from,to,before,after};}
static int graph(const CDCSR*g){
 if(!g||!g->vertices||g->vertices>8||g->count>24||g->offset[0]||g->offset[g->vertices]!=g->count)return 0;
 for(unsigned v=0;v<g->vertices;++v){if(g->offset[v]>g->offset[v+1]||g->offset[v+1]>g->count)return 0;for(unsigned e=g->offset[v];e<g->offset[v+1];++e)if(g->edges[e].from!=v||g->edges[e].to>=g->vertices)return 0;}return 1;
}
enum AGStatus ag_run(const CDEdge*edges,size_t n,unsigned vertices,unsigned source,unsigned parallel,AGWorkspace*workspace,size_t queue_capacity,AGResult*out,AGTrace*trace){
 if(!workspace||!out||!trace||!vertices||vertices>8||source>=vertices||n>24||(n&&!edges)||parallel>1||queue_capacity>8)return AG_INVALID;
 if(queue_capacity<vertices)return AG_SPACE;
 CDCSR g;if(cd_csr_build(&g,vertices,edges,n,parallel)!=CD_OK)return AG_INVALID;
 for(unsigned i=0;i<g.count;++i)if(g.edges[i].weight<0)return AG_INVALID;
 AGTrace t={0};ADTraversal bfs;uint64_t distances[8];unsigned pred[8];enum ADStatus s=ag_refined_bfs(&g,source,0,workspace->queue,queue_capacity,&bfs,&t);if(s!=AD_OK)return s==AD_SPACE?AG_SPACE:AG_INVALID;
 s=ag_refined_dijkstra(&g,source,65534u,distances,pred,workspace,&t);if(s!=AD_OK)return s==AD_OVERFLOW?AG_RANGE:AG_INVALID;
 AGResult result={0};result.vertices=vertices;result.source=source;for(unsigned i=0;i<8;++i){result.previous[i]=8;if(distances[i]!=UINT64_MAX){result.reachable|=1u<<i;result.distance[i]=(uint32_t)distances[i];result.hops[i]=bfs.distance[i];if(i!=source)result.previous[i]=pred[i];}}
 *out=result;*trace=t;return AG_OK;
}
enum AGStatus ag_certificate(const CDCSR*g,unsigned source,const AGCertificate*c){
 if(!graph(g)||!c||source>=g->vertices||(c->reachable>>g->vertices))return AG_INVALID;
 if(!(c->reachable&(1u<<source))||c->distance[source]!=0||c->previous[source]!=source)return AG_BAD_CERTIFICATE;
 for(unsigned v=0;v<g->vertices;++v)if((c->reachable&(1u<<v))&&(c->distance[v]<-65534||c->distance[v]>65534))return AG_BAD_CERTIFICATE;
 for(unsigned v=0;v<g->vertices;++v)if(c->reachable&(1u<<v)){
  unsigned at=v,seen=0;while(at!=source){if(at>=g->vertices||(seen&(1u<<at))||!(c->reachable&(1u<<at)))return AG_BAD_CERTIFICATE;seen|=1u<<at;unsigned p=c->previous[at];if(p>=g->vertices||!(c->reachable&(1u<<p)))return AG_BAD_CERTIFICATE;int found=0;for(unsigned e=g->offset[p];e<g->offset[p+1];++e)if(g->edges[e].to==at&&c->distance[p]+(int64_t)g->edges[e].weight==c->distance[at])found=1;if(!found)return AG_BAD_CERTIFICATE;at=p;}
 }
#ifndef AG_MUTANT_INEQUALITY
 for(unsigned e=0;e<g->count;++e){CDEdge edge=g->edges[e];if(c->reachable&(1u<<edge.from))if(!(c->reachable&(1u<<edge.to))||c->distance[edge.to]>c->distance[edge.from]+(int64_t)edge.weight)return AG_BAD_CERTIFICATE;}
#endif
 return AG_OK;
}
