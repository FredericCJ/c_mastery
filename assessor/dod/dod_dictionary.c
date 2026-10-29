#include "dod_dictionary.h"
#ifndef D_MAP_MUTANT
#define D_MAP_MUTANT 0
#endif
int d_map_init(struct dh_map *m,struct dh_dictionary d){if(!m||!d.hash||!d.equal)return D_INVALID;m->used=0;m->fail_at=0;m->attempts=0;m->dictionary=d;return D_OK;}
static unsigned rank(unsigned bitmap,unsigned slot){unsigned n=0;for(unsigned i=0;i<slot;++i)n+=(bitmap>>i)&1u;return n;}
static int equal(const struct dh_map *m,struct dh_key a,struct dh_key b,struct dh_metrics *cost){++cost->comparisons;
#if D_MAP_MUTANT == 1
 (void)m;(void)a;(void)b;return 1;
#else
 return m->dictionary.equal(m->dictionary.context,a,b)!=0;
#endif
}
static int allocate(struct dh_map *m,struct dh_map_node n,unsigned *out,struct dh_metrics *cost){
 ++m->attempts;if(m->used==512||(m->fail_at&&m->attempts==m->fail_at))return D_FAILED;
 m->nodes[++m->used]=n;*out=m->used;++cost->clones;return D_OK;
}
static int edit(struct dh_map *m,unsigned root,unsigned depth,uint32_t hash,struct dh_key key,int value,int remove,unsigned *out,struct dh_metrics *cost){
 struct dh_map_node n={0,0,{0},{{{0}}},{0}};if(root){n=m->nodes[root];++cost->visited;}
 if(depth==16){
  unsigned i=0;while(i<n.count&&!equal(m,n.keys[i],key,cost))++i;
  if(remove){if(i==n.count){*out=root;return D_OK;}for(unsigned j=i+1;j<n.count;++j){n.keys[j-1]=n.keys[j];n.values[j-1]=n.values[j];}--n.count;if(!n.count){*out=0;return D_OK;}}
  else{if(i==n.count){if(n.count==8)return D_FULL;n.keys[n.count++]=key;}n.values[i]=value;}
#if D_MAP_MUTANT == 2
  if(root)m->nodes[root]=n;
#endif
  return allocate(m,n,out,cost);
 }
 unsigned slot=(hash>>(2*depth))&3u,position=rank(n.bitmap,slot),old=(n.bitmap&(1u<<slot))?n.children[position]:0,child=0;
 int status=edit(m,old,depth+1,hash,key,value,remove,&child,cost);if(status!=D_OK)return status;
 if(child==old){*out=root;return D_OK;}
 if(!child){for(unsigned j=position+1;j<n.count;++j)n.children[j-1]=n.children[j];--n.count;n.bitmap&=~(1u<<slot);if(!n.count){*out=0;return D_OK;}}
 else{if(!old){for(unsigned j=n.count;j>position;--j)n.children[j]=n.children[j-1];++n.count;n.bitmap|=1u<<slot;}n.children[position]=child;}
 return allocate(m,n,out,cost);
}
static int change(struct dh_map *m,unsigned root,struct dh_key key,int value,int remove,unsigned *out,struct dh_metrics *metrics){
 if(!m||!out||!m->dictionary.hash||!m->dictionary.equal||root>m->used)return D_INVALID;
 unsigned before=m->used,result=0;struct dh_metrics cost={0};m->attempts=0;
 uint32_t hash=m->dictionary.hash(m->dictionary.context,key);
 int status=edit(m,root,0,hash,key,value,remove,&result,&cost);
 if(status==D_OK)*out=result;else m->used=before;
 if(metrics)*metrics=cost;
 return status;
}
int d_map_update(struct dh_map *m,unsigned root,struct dh_key key,int value,unsigned *out,struct dh_metrics *metrics){return change(m,root,key,value,0,out,metrics);}
int d_map_delete(struct dh_map *m,unsigned root,struct dh_key key,unsigned *out,struct dh_metrics *metrics){return change(m,root,key,0,1,out,metrics);}
int d_map_get(const struct dh_map *m,unsigned root,struct dh_key key,int *out,struct dh_metrics *metrics){
 if(!m||!out||!m->dictionary.hash||!m->dictionary.equal||root>m->used)return D_INVALID;
 struct dh_metrics cost={0};uint32_t hash=m->dictionary.hash(m->dictionary.context,key);
 for(unsigned depth=0;depth<16&&root;++depth){const struct dh_map_node *n=&m->nodes[root];++cost.visited;unsigned slot=(hash>>(2*depth))&3u;root=n->bitmap&(1u<<slot)?n->children[rank(n->bitmap,slot)]:0;}
 int status=D_STALE;if(root){const struct dh_map_node *n=&m->nodes[root];++cost.visited;for(unsigned i=0;i<n->count;++i)if(equal(m,n->keys[i],key,&cost)){*out=n->values[i];status=D_OK;break;}}
 if(metrics)*metrics=cost;
 return status;
}
