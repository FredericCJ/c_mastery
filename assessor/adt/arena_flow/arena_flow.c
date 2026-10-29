#include "arena_flow.h"
#ifndef ADF_MUTATION
#define ADF_MUTATION 0
#endif
enum adf_status adf_arena(const struct ad_node *nodes,size_t n,struct adf_arena *out){
 if(!nodes||!out||!n||n>32)return ADF_INVALID;
 struct adf_arena next={0};next.count=n;for(size_t i=0;i<n;++i)next.nodes[i]=nodes[i];
 uint32_t ignored;if(!ad_eval(next.nodes,ADF_MUTATION==1?1:n,(struct ad_node_index){0},&ignored))return ADF_INVALID;
 for(size_t i=0;i<n;++i){struct ad_node node=next.nodes[i];next.values[i]=node.kind==0?node.value:next.values[node.left.index]+next.values[node.right.index];}
 *out=next;return ADF_OK;
}
enum adf_status adf_root(const struct adf_arena *a,struct ad_node_index root,uint32_t *out){if(!a||!out||root.index>=a->count)return ADF_INVALID;*out=ADF_MUTATION==2?a->nodes[root.index].value:a->values[root.index];return ADF_OK;}
enum adf_status adf_start(const struct adf_node *nodes,unsigned n,struct adf_session *out){
 if(!nodes||!out||!n||n>8)return ADF_INVALID;
 unsigned allowed=(1u<<n)-1u;struct adf_session next={0};next.count=n;next.pending=allowed;
 for(unsigned i=0;i<n;++i){if(nodes[i].predecessors>allowed||nodes[i].generate>255||nodes[i].keep>255)return ADF_INVALID;next.nodes[i]=nodes[i];}
 *out=next;return ADF_OK;
}
enum adf_status adf_step(struct adf_session *s,unsigned budget,struct adf_result *out){
 if(!s||!out)return ADF_INVALID;
 unsigned used=0;
 while(s->pending&&used<budget){unsigned i=0;while(!(s->pending&(1u<<i)))++i;s->pending&=~(1u<<i);unsigned incoming=ADF_MUTATION==4?255u:0u;
  for(unsigned j=0;j<s->count;++j)if(s->nodes[i].predecessors&(1u<<j)){if(ADF_MUTATION==4)incoming&=s->values[j];else incoming|=s->values[j];}
  unsigned value=s->nodes[i].generate|(incoming&s->nodes[i].keep);
  if(value!=s->values[i]){s->values[i]=value;if(ADF_MUTATION!=3)for(unsigned j=0;j<s->count;++j)if(s->nodes[j].predecessors&(1u<<i))s->pending|=1u<<j;}
  ++s->evaluations;++used;
 }
 if(s->pending&&ADF_MUTATION!=5)return ADF_BUDGET;
 struct adf_result next={0};for(unsigned i=0;i<s->count;++i)next.values[i]=s->values[i];next.evaluations=s->evaluations;*out=next;return ADF_OK;
}
enum adf_status adf_boolean(const struct adf_boolean *nodes,unsigned n,unsigned budget,unsigned *out){
 if(!nodes||!out||!n||n>4)return ADF_INVALID;
 for(unsigned i=0;i<n;++i)if(nodes[i].left>=n||nodes[i].right>=n||nodes[i].table>15)return ADF_INVALID;
 for(unsigned candidate=0;candidate<(1u<<n);++candidate){if(candidate==budget)return ADF_MUTATION==6?ADF_NO_FIXED_POINT:ADF_BUDGET;unsigned computed=0;for(unsigned i=0;i<n;++i){unsigned a=(candidate>>nodes[i].left)&1u,b=(candidate>>nodes[i].right)&1u;computed|=((nodes[i].table>>(a|(b<<1)))&1u)<<i;}if(computed==candidate){*out=candidate;return ADF_OK;}}
 return ADF_NO_FIXED_POINT;
}
