#include "higher_order.h"
#include <limits.h>
#ifndef HO_MUTANT
#define HO_MUTANT 0
#endif
static int valid(const struct ho_hoa *a,unsigned n){return a&&n&&n<=a->used;}
static int node(struct ho_hoa *a,struct ho_node n,unsigned *out){
 if(!a||!out)return HO_INVALID;
 if(a->attempts==SIZE_MAX)return HO_FULL;
 ++a->attempts;
 if(a->fail_at==a->attempts)return HO_FAILED;
 if(a->used==64)return HO_FULL;
 a->nodes[++a->used]=n;*out=a->used;return HO_OK;
}
int ho_pure(struct ho_hoa *a,int32_t v,unsigned *out){return node(a,(struct ho_node){HO_PURE,v,0,0},out);}
int ho_emit(struct ho_hoa *a,int32_t v,unsigned next,unsigned *out){if(!valid(a,next))return HO_INVALID;return node(a,(struct ho_node){HO_EMIT,v,next,0},out);}
int ho_choose(struct ho_hoa *a,unsigned left,unsigned right,unsigned *out){if(!valid(a,left)||!valid(a,right))return HO_INVALID;return node(a,(struct ho_node){HO_CHOOSE,0,left,right},out);}
static int bind_rec(struct ho_hoa *a,unsigned root,ho_bind_fn f,void *context,unsigned *out){
 struct ho_node n=a->nodes[root];unsigned l,r;int status;
 if(n.kind==HO_PURE){status=f(a,n.value,context,&l);if(status!=HO_OK)return status;if(!valid(a,l))return HO_INVALID;*out=l;return HO_OK;}
 if(n.kind==HO_CHOOSE&&HO_MUTANT==1){*out=root;return HO_OK;}
 status=bind_rec(a,n.left,f,context,&l);if(status!=HO_OK)return status;
 if(n.kind==HO_EMIT)return ho_emit(a,n.value,l,out);
 status=bind_rec(a,n.right,f,context,&r);if(status!=HO_OK)return status;
 return ho_choose(a,l,r,out);
}
int ho_bind(struct ho_hoa *a,unsigned root,ho_bind_fn f,void *context,unsigned *out){
 if(!valid(a,root)||!f||!out)return HO_INVALID;
 unsigned mark=a->used,result;int status=bind_rec(a,root,f,context,&result);
 if(status!=HO_OK){a->used=mark;return status;}
 *out=result;return HO_OK;
}
int ho_run(const struct ho_hoa *a,unsigned root,const unsigned char *bits,size_t length,struct ho_result *out){
 if(!valid(a,root)||!out||(!bits&&length))return HO_INVALID;
 struct ho_result r={0};
 while(1){struct ho_node n=a->nodes[root];
  if(n.kind==HO_PURE){r.value=n.value;*out=r;return HO_OK;}
  if(n.kind==HO_EMIT){if(r.count==32)return HO_FULL;r.trace[r.count++]=n.value;root=n.left;}
  else{if(r.consumed==length){if(HO_MUTANT==2)*out=r;return HO_INPUT;}
   unsigned b=bits[r.consumed++];if(b>1)return HO_INVALID;root=b?n.right:n.left;}
 }
}
static int summary(const struct ho_hoa *a,unsigned root,struct ho_summary memo[65],unsigned char done[65],struct ho_summary *out){
 if(done[root]){*out=memo[root];return HO_OK;}
 struct ho_node n=a->nodes[root];struct ho_summary l,r;int status;
 if(n.kind==HO_PURE){*out=(struct ho_summary){1,0,n.value};memo[root]=*out;done[root]=1;return HO_OK;}
 status=summary(a,n.left,memo,done,&l);if(status!=HO_OK)return status;
 if(n.kind==HO_EMIT){if(UINT64_MAX-l.emissions<l.paths)return HO_RANGE;l.emissions+=l.paths;*out=l;memo[root]=l;done[root]=1;return HO_OK;}
 status=summary(a,n.right,memo,done,&r);if(status!=HO_OK)return status;
 if(UINT64_MAX-l.paths<r.paths||UINT64_MAX-l.emissions<r.emissions)return HO_RANGE;
 if((r.sum>0&&l.sum>INT64_MAX-r.sum)||(r.sum<0&&l.sum<INT64_MIN-r.sum))return HO_RANGE;
 *out=(struct ho_summary){l.paths+r.paths,l.emissions+r.emissions,l.sum+r.sum};memo[root]=*out;done[root]=1;return HO_OK;
}
int ho_summarize(const struct ho_hoa *a,unsigned root,struct ho_summary *out){if(!valid(a,root)||!out)return HO_INVALID;struct ho_summary r,memo[65]={{0}};unsigned char done[65]={0};int status=summary(a,root,memo,done,&r);if(status==HO_OK)*out=r;return status;}
