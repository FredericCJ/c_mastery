#include "policy.h"
#include <stdio.h>
#include <inttypes.h>
#include <string.h>
#define CHECK(x) do{if(!(x)){fprintf(stderr,"LC_CHECK:%u\n",__LINE__);return 1;}}while(0)
int main(int argc,char **argv){
 if(argc==2&&!strcmp(argv[1],"stream")){int64_t lag[3];uint64_t vd[3],q,d,p;while(scanf("%"SCNd64" %"SCNu64" %"SCNd64" %"SCNu64" %"SCNd64" %"SCNu64" %"SCNu64" %"SCNu64" %"SCNu64,&lag[0],&vd[0],&lag[1],&vd[1],&lag[2],&vd[2],&q,&d,&p)==9){struct lc_entity e[3]={{lag[0],vd[0]},{lag[1],vd[1]},{lag[2],vd[2]}};size_t out=77;int r=lc_select(e,3,&out);printf("M %d %zu %d\n",r,out,lc_parameters(q,d,p));}return 0;}
 if(argc!=1)return 2;
 struct lc_entity e[3]={{-1,1},{0,20},{1,10}};size_t out=77;CHECK(lc_select(e,3,&out)==1&&out==2);e[2].virtual_deadline=20;CHECK(lc_select(e,3,&out)==1&&out==1);e[1].lag=-1;e[2].lag=-3;out=77;CHECK(lc_select(e,3,&out)==0&&out==77);CHECK(lc_select(e,0,&out)==-1&&out==77);CHECK(lc_select(0,3,&out)==-1&&out==77);CHECK(lc_select(e,3,0)==-1);CHECK(lc_parameters(3,5,10));CHECK(!lc_parameters(3,2,10));CHECK(!lc_parameters(0,5,10));CHECK(!lc_parameters(3,5,4));CHECK(lc_parameters(UINT64_MAX,UINT64_MAX,UINT64_MAX));puts("LC_PASS:eligibility-virtual-deadline-parameter-boundaries");return 0;}
