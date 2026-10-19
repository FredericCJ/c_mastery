#include "dod_rope_stats.h"
#ifndef D_STATS_MUTANT
#define D_STATS_MUTANT 0
#endif
static unsigned fibonacci(unsigned depth){unsigned a=0,b=1;for(unsigned i=0;i<depth+2;++i){unsigned next=a+b;a=b;b=next>129?129:next;}return a;}
static int visit(const DRope *r,unsigned root,unsigned *seen,unsigned *depths,DRopeStats *out){
 if(!root)return D_OK;
 if(seen[root])return D_OK;
 seen[root]=1;const DRopeNode *n=&r->nodes[root];++out->reachable_nodes;
 if(!n->left){if(n->right||n->length<1||n->length>8)return D_INVALID;++out->unique_leaves;out->stored_bytes+=n->length;out->leaf_slack+=8-n->length;depths[root]=0;}
 else{if(!n->right||n->left>=root||n->right>=root)return D_INVALID;int s=visit(r,n->left,seen,depths,out);if(s)return s;s=visit(r,n->right,seen,depths,out);if(s)return s;if(n->length>128||n->length!=r->nodes[n->left].length+r->nodes[n->right].length)return D_INVALID;unsigned a=depths[n->left],b=depths[n->right];depths[root]=1+(a>b?a:b);}
 return D_OK;
}
int d_rope_stats(const DRope *r,unsigned root,DRopeStats *out){
 if(!r||!out||r->used>64||root>r->used)return D_INVALID;
 unsigned seen[65]={0},depths[65]={0};DRopeStats result={0};int s=visit(r,root,seen,depths,&result);if(s)return s;
 result.depth=depths[root];result.logical_bytes=root?r->nodes[root].length:0;result.retained_unreachable=r->used-result.reachable_nodes;result.fibonacci_min=root?fibonacci(result.depth):0;result.balanced=result.logical_bytes>=result.fibonacci_min;
#if D_STATS_MUTANT == 1
 result.balanced=1;
#elif D_STATS_MUTANT == 2
 result.stored_bytes=result.logical_bytes;
#endif
 *out=result;return D_OK;
}
