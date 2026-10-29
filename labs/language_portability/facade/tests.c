#include "depth.h"
#include BF_B02_FACADE
#include "facade.h"
#include <stdio.h>
#include <string.h>
#define CHECK(x) do{if(!(x)){fprintf(stderr,"LP_CHECK:%s\n",#x);return 1;}}while(0)
struct lp_trace {char order[4];unsigned n,counts[2];int values[2];};
struct lp_get {struct lp_trace *trace;unsigned side;};
static int get(void *p){struct lp_get *g=p;struct lp_trace *t=g->trace;t->order[t->n++]=(char)('L'+(g->side?6:0));++t->counts[g->side];return t->values[g->side];}
int main(int argc,char **argv)
{
 if(argc!=2)return 2;
 if(!strcmp(argv[1],"borrow")){struct bf_record r={7};const struct bf_record *c=&r;CHECK(BF_BORROW(&r)==&r && BF_BORROW(c)==c);BF_BORROW(&r)->value=9;CHECK(c->value==9);CHECK(BF_BORROW((struct bf_record*)0)==0);puts("LP borrow validated");}
 else if(!strcmp(argv[1],"binary")){int i=7;unsigned u=9;CHECK(B2_IDENTITY(i++)==7 && i==8);CHECK(B2_IDENTITY(u++)==9 && u==10);CHECK(BF_BINARY(7,9)==709 && BF_BINARY(7u,9u)==709u);struct lp_trace t={{0},0,{0},{7,9}};struct lp_get l={&t,0},r={&t,1};CHECK(BF_BINARY(get(&l),get(&r))==709 && t.n==2 && t.counts[0]==1 && t.counts[1]==1);CHECK((t.order[0]=='L' && t.order[1]=='R') || (t.order[0]=='R' && t.order[1]=='L'));puts("LP binary validated");}
 else if(!strcmp(argv[1],"sequence")){struct lp_trace t={{0},0,{0},{7,9}};struct lp_get l={&t,0},r={&t,1};int out=77;CHECK(bf_sequence(get,&l,get,&r,&out)==0 && out==709 && t.n==2 && t.order[0]=='L' && t.order[1]=='R');t.n=0;t.values[0]=-1;out=77;CHECK(bf_sequence(get,&l,get,&r,&out)==-1 && out==77 && t.n==1);t.n=0;t.values[0]=7;t.values[1]=100;CHECK(bf_sequence(get,&l,get,&r,&out)==-1 && out==77 && t.n==2);t.n=0;CHECK(bf_sequence(0,&l,get,&r,&out)==-1 && out==77 && t.n==0);puts("LP sequence validated");}
 else if(!strcmp(argv[1],"oracle")){for(int a=0;a<100;++a)for(int b=0;b<100;++b)printf("%d %d %d %u\n",a,b,BF_BINARY(a,b),BF_BINARY((unsigned)a,(unsigned)b));}
 else if(!strcmp(argv[1],"metrics"))printf("{\"record_bytes\":%zu,\"callback_pointer_bytes\":%zu,\"allocations\":0}\n",sizeof(struct bf_record),sizeof(bf_get_fn));
 else return 2;
 return 0;
}
