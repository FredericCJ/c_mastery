#include "references.h"
#ifndef ORC_MUTATION
#define ORC_MUTATION 0
#endif
typedef struct Node Node;
typedef struct {Node *node;unsigned weak;} Edge;
typedef struct {OrcMemory memory;V4Allocator cells;Node *node[2];Edge pending[2];unsigned pending_count;OrcCycleReport report;} World;
struct Node {OwShared refs;World *world;int32_t *payload;Edge edge;unsigned index;};
static void destroy(void *p){Node *n=p;World *w=n->world;int32_t *payload=n->payload;Edge edge=n->edge;n->payload=NULL;n->edge=(Edge){0};++w->report.destructions;w->cells.release(w->cells.context,payload);if(edge.node){w->pending[w->pending_count++]=edge;++w->report.queued_releases;}}
static void reclaim(void *p){Node *n=p;World *w=n->world;OrcMemory memory=w->memory;w->node[n->index]=NULL;++w->report.reclamations;memory.release(memory.context,n);}
static OwStatus create(World *w,unsigned i){Node *n=w->memory.take(w->memory.context,sizeof(*n));if(!n)return OW_NOMEM;int32_t *payload=w->cells.take(w->cells.context,1);if(!payload){w->memory.release(w->memory.context,n);return OW_NOMEM;}*payload=(int32_t)(11+i);*n=(Node){0};n->world=w;n->payload=payload;n->index=i;n->refs=(OwShared){1,1,destroy,reclaim,n};w->node[i]=n;return OW_OK;}
static void drain(World *w){while(w->pending_count){Edge e=w->pending[--w->pending_count];if(e.weak)(void)ow_shared_weak_drop(&e.node->refs);else(void)ow_shared_drop(&e.node->refs);}}
OwStatus orc_cycle(OrcMemory m,V4Allocator a,int weak,OrcCycleReport *out){
 if(!out||!m.take||!m.release||!a.take||!a.release||(weak!=0&&weak!=1))return OW_INVALID;
 World w={0};w.memory=m;w.cells=a;
 for(unsigned i=0;i<2;++i){OwStatus s=create(&w,i);if(s!=OW_OK){for(unsigned j=0;j<i;++j)(void)ow_shared_drop(&w.node[j]->refs);return s;}}
 Node *first=w.node[0],*second=w.node[1];
 /* Each explicit observer weak retains its control until the final audit. */
 (void)ow_shared_downgrade(&first->refs);(void)ow_shared_downgrade(&second->refs);
 (void)ow_shared_retain(&second->refs);first->edge=(Edge){second,0};
 if(weak&&ORC_MUTATION!=1){(void)ow_shared_downgrade(&first->refs);second->edge=(Edge){first,1};}
 else{(void)ow_shared_retain(&first->refs);second->edge=(Edge){first,0};}
 (void)ow_shared_drop(&first->refs);(void)ow_shared_drop(&second->refs);drain(&w);
 w.report.retained_values=2-w.report.destructions;w.report.retained_controls=2-w.report.reclamations;
 if(w.report.retained_values){
  /* Exclusive graph maintenance consumes the existing first->second strong edge. */
  (void)ow_shared_downgrade(&second->refs);first->edge.weak=1;++w.report.repair_edges;
  (void)ow_shared_drop(&second->refs);drain(&w);
 }
 /* Nodes remain real live control objects because of the two observer weak refs. */
 (void)ow_shared_weak_drop(&first->refs);(void)ow_shared_weak_drop(&second->refs);
 *out=w.report;return OW_OK;
}
