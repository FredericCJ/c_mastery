#include "transfer.h"
#include <limits.h>
static int push(const ATCertificate*c,CTWorkspace*w,unsigned*used,unsigned capacity,
                int node,int parent,int64_t low,int64_t high){
 if(node<0||(unsigned)node>=c->capacity||w->seen[node]||!c->nodes[node].live)return ALGO_BAD;
 if(*used==capacity)return CT_SPACE;
 w->seen[node]=1;w->frames[(*used)++]=(CTFrame){.node=node,.parent=parent,.low=low,.high=high,.left_black=1,.right_black=1};
 if(*used>w->high_water)w->high_water=*used;
 return ALGO_OK;
}
int ct_tree(const ATCertificate*c,unsigned policy,CTWorkspace*w,unsigned capacity,int*out){
 if(!c||!w||!out||!c->capacity||c->capacity>64||c->count>c->capacity||policy>2||capacity>64)return ALGO_BAD;
 for(unsigned i=0;i<64;++i)w->seen[i]=0;
 w->high_water=0;unsigned used=0;int height=0;
 if(c->root!=-1){int status=push(c,w,&used,capacity,c->root,-1,INT64_MIN,INT64_MAX);if(status!=ALGO_OK)return status;}
 while(used){CTFrame*f=&w->frames[used-1];ATNode n=c->nodes[f->node];
  if(f->phase==0){
   if(n.parent!=f->parent||n.red>1)return ALGO_BAD;
#ifndef CT_MUTANT_BOUNDS
   if(n.key<=f->low||n.key>=f->high)return ALGO_BAD;
#endif
   f->phase=1;
   if(n.left!=-1){int status=push(c,w,&used,capacity,n.left,f->node,f->low,n.key);if(status!=ALGO_OK)return status;continue;}
  }
  if(f->phase==1){f->phase=2;if(n.right!=-1){int status=push(c,w,&used,capacity,n.right,f->node,n.key,f->high);if(status!=ALGO_OK)return status;continue;}}
  int l=f->left_height,r=f->right_height,h=1+(l>r?l:r),black=0;
#ifndef CT_MUTANT_HEIGHT
  if(policy!=2&&n.height!=h)return ALGO_BAD;
#endif
  if(policy==1&&(l-r>1||r-l>1))return ALGO_BAD;
  if(policy==2){if(f->left_black!=f->right_black||(n.red&&((n.left>=0&&c->nodes[n.left].red)||(n.right>=0&&c->nodes[n.right].red))))return ALGO_BAD;black=f->left_black+!n.red;}
  --used;if(used){CTFrame*p=&w->frames[used-1];if(p->phase==1){p->left_height=h;p->left_black=black;}else{p->right_height=h;p->right_black=black;}}else height=h;
 }
 unsigned count=0;for(unsigned i=0;i<c->capacity;++i){if(w->seen[i]!=(c->nodes[i].live!=0))return ALGO_BAD;count+=w->seen[i];}
 if(count!=c->count||(policy==2&&c->root>=0&&c->nodes[c->root].red))return ALGO_BAD;
 *out=height;return ALGO_OK;
}
