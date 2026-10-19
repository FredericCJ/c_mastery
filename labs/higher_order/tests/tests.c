#include "higher_order.h"
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
static const char *test_name="arguments";
#define CHECK(x) do{if(!(x)){fprintf(stderr,"HO_CHECK:%s:%d:%s\n",test_name,__LINE__,#x);exit(1);}}while(0)
static int twice(HOA *a,int32_t v,void *c,unsigned *out){(void)c;int64_t n=(int64_t)v*2;if(n<INT32_MIN||n>INT32_MAX)return HO_RANGE;unsigned p;int s=ho_pure(a,(int32_t)n,&p);return s?s:ho_emit(a,v,p,out);}
static int pure_cb(HOA *a,int32_t v,void *c,unsigned *out){(void)c;return ho_pure(a,v,out);}
static int plus_one(HOA *a,int32_t v,void *c,unsigned *out){(void)c;if(v==INT32_MAX)return HO_RANGE;unsigned p;int s=ho_pure(a,v+1,&p);return s?s:ho_emit(a,-7,p,out);}
static int compose(HOA *a,int32_t v,void *c,unsigned *out){unsigned p;int s=twice(a,v,c,&p);return s?s:ho_bind(a,p,plus_one,NULL,out);}
static void free_test(void){
 for(int l=-3;l<=3;++l)for(int r=-3;r<=3;++r){HOA a={0};unsigned left,right,choice,source,bound;CHECK(ho_pure(&a,l,&left)==HO_OK);CHECK(ho_pure(&a,r,&right)==HO_OK);CHECK(ho_choose(&a,left,right,&choice)==HO_OK);CHECK(ho_emit(&a,7,choice,&source)==HO_OK);unsigned mark=a.used;CHECK(ho_bind(&a,source,twice,NULL,&bound)==HO_OK);
  for(unsigned char bit=0;bit<2;++bit){HOResult out={0};CHECK(ho_run(&a,bound,&bit,1,&out)==HO_OK);int v=bit?r:l;CHECK(out.value==2*v&&out.consumed==1&&out.count==2&&out.trace[0]==7&&out.trace[1]==v);}
  HOSummary sum;CHECK(ho_summarize(&a,bound,&sum)==HO_OK&&sum.paths==2&&sum.emissions==4&&sum.sum==2*(l+r));
  unsigned identity;CHECK(ho_bind(&a,source,pure_cb,NULL,&identity)==HO_OK);for(unsigned char bit=0;bit<2;++bit){HOResult x,y;CHECK(ho_run(&a,source,&bit,1,&x)==HO_OK&&ho_run(&a,identity,&bit,1,&y)==HO_OK);CHECK(x.value==y.value&&x.count==y.count&&x.trace[0]==y.trace[0]);}
  for(size_t cut=1;cut<=6;++cut){a.used=mark;a.fail_at=a.attempts+cut;unsigned out=999;CHECK(ho_bind(&a,source,twice,NULL,&out)==HO_FAILED&&out==999&&a.used==mark);HOResult old;unsigned char bit=0;CHECK(ho_run(&a,source,&bit,1,&old)==HO_OK&&old.value==l);}
 }
 HOA a={0};unsigned p,c,root;CHECK(ho_pure(&a,1,&p)==HO_OK);CHECK(ho_choose(&a,p,p,&c)==HO_OK);CHECK(ho_emit(&a,9,c,&root)==HO_OK);HOResult sentinel={.value=91,.count=31,.consumed=77},out=sentinel;CHECK(ho_run(&a,root,NULL,0,&out)==HO_INPUT);CHECK(out.value==91&&out.count==31&&out.consumed==77);unsigned char invalid=2;CHECK(ho_run(&a,root,&invalid,1,&out)==HO_INVALID&&out.value==91);
 for(unsigned i=0;i<33;++i){unsigned next;CHECK(ho_emit(&a,1,p,&next)==HO_OK);p=next;}CHECK(ho_run(&a,p,NULL,0,&out)==HO_FULL&&out.value==91);
 for(int v=-3;v<=3;++v){HOA law={0};unsigned pure,first,left,right,identity,unit;
  CHECK(ho_pure(&law,v,&pure)==HO_OK&&ho_emit(&law,9,pure,&root)==HO_OK);
  CHECK(ho_bind(&law,root,twice,NULL,&first)==HO_OK&&ho_bind(&law,first,plus_one,NULL,&left)==HO_OK&&ho_bind(&law,root,compose,NULL,&right)==HO_OK);
  HOResult x,y;CHECK(ho_run(&law,left,NULL,0,&x)==HO_OK&&ho_run(&law,right,NULL,0,&y)==HO_OK&&x.value==y.value&&x.count==y.count);
  for(size_t i=0;i<x.count;++i)CHECK(x.trace[i]==y.trace[i]);
  CHECK(ho_bind(&law,pure,twice,NULL,&identity)==HO_OK&&twice(&law,v,NULL,&unit)==HO_OK);
  CHECK(ho_run(&law,identity,NULL,0,&x)==HO_OK&&ho_run(&law,unit,NULL,0,&y)==HO_OK&&x.value==y.value&&x.count==y.count&&x.trace[0]==y.trace[0]);
 }
 for(int value=0;value<=1;++value){HOA sharing={0};CHECK(ho_pure(&sharing,value,&root)==HO_OK);for(unsigned i=0;i<63;++i){unsigned next;CHECK(ho_choose(&sharing,root,root,&next)==HO_OK);root=next;}
  HOSummary s={91,92,93};int result=ho_summarize(&sharing,root,&s);if(value)CHECK(result==HO_RANGE&&s.paths==91&&s.sum==93);else CHECK(result==HO_OK&&s.paths==(UINT64_C(1)<<63)&&s.sum==0);
  unsigned unchanged=999;CHECK(ho_pure(&sharing,2,&unchanged)==HO_FULL&&unchanged==999&&sharing.used==64);
 }
}
#include "program.inc"
HO_PROGRAM(E,HEInt,HEFun,HEOps)
HO_PROGRAM(P,HPInt,HPFun,HPOps)
static int bad_body(void *c,HEInt x,HEInt *out){(void)c;(void)x;*out=123;return HO_FAILED;}
static void final_test(void){
 HEOps e=ho_eval_ops();HPOps p=ho_print_ops();CHECK(e.lit&&e.add&&e.lam&&e.app&&p.lit&&p.add&&p.lam&&p.app);
 for(int n=-10;n<=10;++n){EContext ec={e,n,0};PContext pc={p,n,0};HEFun ef;HPFun pf;HEInt ei;HPInt pi;
  CHECK(e.lam(Ebody,&ec,&ef)==HO_OK&&ec.calls==0);CHECK(p.lam(Pbody,&pc,&pf)==HO_OK&&pc.calls==1);
  CHECK(Ehigher(e,ef,&ei)==HO_OK&&ei==n+3&&ec.calls==1);CHECK(Phigher(p,pf,&pi)==HO_OK&&pc.calls==1);
  char expected[128];int count=snprintf(expected,sizeof expected,"(((lambda x.(x+%d)) 1)+2)",n);CHECK(count>0&&pi.length==(size_t)count&&!strcmp(pi.data,expected));
 }
 HEInt unchanged=91;CHECK(e.add(INT32_MAX,1,&unchanged)==HO_RANGE&&unchanged==91);HEFun fail;CHECK(e.lam(bad_body,NULL,&fail)==HO_OK&&e.app(fail,7,&unchanged)==HO_FAILED&&unchanged==91);
 HPInt text;CHECK(p.lit(INT32_MIN,&text)==HO_OK&&!strcmp(text.data,"-2147483648"));HPInt full={{0},127};for(unsigned i=0;i<127;++i)full.data[i]='a';HPInt old=text;CHECK(p.add(full,text,&text)==HO_FULL&&!strcmp(text.data,old.data));
}
static void oracle(void){
 for(int l=-3;l<=3;++l)for(int r=-3;r<=3;++r){HOA a={0};unsigned x,y,c,s,b;CHECK(ho_pure(&a,l,&x)==HO_OK&&ho_pure(&a,r,&y)==HO_OK&&ho_choose(&a,x,y,&c)==HO_OK&&ho_emit(&a,7,c,&s)==HO_OK&&ho_bind(&a,s,twice,NULL,&b)==HO_OK);for(unsigned char bit=0;bit<2;++bit){HOResult out;CHECK(ho_run(&a,b,&bit,1,&out)==HO_OK);printf("F %d %d %u %d %zu %d %d\n",l,r,bit,out.value,out.count,out.trace[0],out.trace[1]);}}
 HEOps e=ho_eval_ops();HPOps p=ho_print_ops();CHECK(e.lit&&p.lit);
 for(int n=-10;n<=10;++n){EContext ec={e,n,0};PContext pc={p,n,0};HEFun ef;HPFun pf;HEInt ei;HPInt pi;CHECK(e.lam(Ebody,&ec,&ef)==HO_OK&&p.lam(Pbody,&pc,&pf)==HO_OK&&Ehigher(e,ef,&ei)==HO_OK&&Phigher(p,pf,&pi)==HO_OK);printf("T %d %d %s\n",n,ei,pi.data);}
}
int main(int argc,char **argv){CHECK(argc==2);test_name=argv[1];if(!strcmp(argv[1],"free"))free_test();else if(!strcmp(argv[1],"final"))final_test();else if(!strcmp(argv[1],"oracle"))oracle();else CHECK(0);return 0;}
