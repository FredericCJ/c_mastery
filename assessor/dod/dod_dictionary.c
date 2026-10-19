#include "dod_dictionary.h"
#ifndef D_MAP_MUTANT
#define D_MAP_MUTANT 0
#endif
int d_map_init(DHMap *m,DHDictionary d){if(!m||!d.hash||!d.equal)return D_INVALID;m->used=0;m->fail_at=0;m->attempts=0;m->dictionary=d;return D_OK;}
static unsigned rank(unsigned bitmap,unsigned slot){unsigned n=0;for(unsigned i=0;i<slot;++i)n+=(bitmap>>i)&1u;return n;}
static int equal(const DHMap *m,DHKey a,DHKey b,DHMetrics *cost){++cost->comparisons;
#if D_MAP_MUTANT == 1
 (void)m;(void)a;(void)b;return 1;
#else
 return m->dictionary.equal(m->dictionary.context,a,b)!=0;
#endif
}
static int allocate(DHMap *m,DHMapNode n,unsigned *out,DHMetrics *cost){
 ++m->attempts;if(m->used==512||(m->fail_at&&m->attempts==m->fail_at))return D_FAILED;
 m->nodes[++m->used]=n;*out=m->used;++cost->clones;return D_OK;
}
static int edit(DHMap *m,unsigned root,unsigned depth,uint32_t hash,DHKey key,int value,int remove,unsigned *out,DHMetrics *cost){
 DHMapNode n={0,0,{0},{{{0}}},{0}};if(root){n=m->nodes[root];++cost->visited;}
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
static int change(DHMap *m,unsigned root,DHKey key,int value,int remove,unsigned *out,DHMetrics *metrics){
 if(!m||!out||!m->dictionary.hash||!m->dictionary.equal||root>m->used)return D_INVALID;
 unsigned before=m->used,result=0;DHMetrics cost={0};m->attempts=0;
 uint32_t hash=m->dictionary.hash(m->dictionary.context,key);
 int status=edit(m,root,0,hash,key,value,remove,&result,&cost);
 if(status==D_OK)*out=result;else m->used=before;
 if(metrics)*metrics=cost;
 return status;
}
int d_map_update(DHMap *m,unsigned root,DHKey key,int value,unsigned *out,DHMetrics *metrics){return change(m,root,key,value,0,out,metrics);}
int d_map_delete(DHMap *m,unsigned root,DHKey key,unsigned *out,DHMetrics *metrics){return change(m,root,key,0,1,out,metrics);}
int d_map_get(const DHMap *m,unsigned root,DHKey key,int *out,DHMetrics *metrics){
 if(!m||!out||!m->dictionary.hash||!m->dictionary.equal||root>m->used)return D_INVALID;
 DHMetrics cost={0};uint32_t hash=m->dictionary.hash(m->dictionary.context,key);
 for(unsigned depth=0;depth<16&&root;++depth){const DHMapNode *n=&m->nodes[root];++cost.visited;unsigned slot=(hash>>(2*depth))&3u;root=n->bitmap&(1u<<slot)?n->children[rank(n->bitmap,slot)]:0;}
 int status=D_STALE;if(root){const DHMapNode *n=&m->nodes[root];++cost.visited;for(unsigned i=0;i<n->count;++i)if(equal(m,n->keys[i],key,&cost)){*out=n->values[i];status=D_OK;break;}}
 if(metrics)*metrics=cost;
 return status;
}
