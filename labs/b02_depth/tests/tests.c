#include "depth.h"
#include "facade.h"
#include <assert.h>
#include <limits.h>
#include <string.h>
#include <stdio.h>
#undef assert
#define assert(x) do { if (!(x)) { fprintf(stderr,"B02_ASSERT:%s:%s\n",argc>1?argv[1]:"arguments",#x); return 1; } } while(0)
struct b2_trace {unsigned fail,trace[8],n;};
static int acquire(void *p,unsigned i){struct b2_trace *t=p;t->trace[t->n++]=10+i;return i==t->fail;}
static void release(void *p,unsigned i){struct b2_trace *t=p;t->trace[t->n++]=20+i;}
int main(int argc,char **argv){assert(argc==2);
 if(!strcmp(argv[1],"macro")){int i=7;unsigned u=9;
#define B2_A B2_B
#define B2_B 73
 assert(B2_IDENTITY(i++)==7&&i==8);assert(B2_IDENTITY(u++)==9&&u==10);assert(!strcmp(B2_STRING_RAW(B2_A),"B2_A"));assert(!strcmp(B2_STRING(B2_A),"73"));}
 else if(!strcmp(argv[1],"parse")){int32_t v=77;assert(b2_parse((const unsigned char *)"-2147483648",11,&v)==B2_OK&&v==INT32_MIN);assert(b2_parse((const unsigned char *)"2147483648",10,&v)==B2_RANGE&&v==INT32_MIN);assert(b2_parse((const unsigned char *)"01",2,&v)==B2_INVALID);assert(b2_parse((const unsigned char *)"12\0x",4,&v)==B2_INVALID);assert(b2_parse((const unsigned char *)"0",1,&v)==B2_OK&&v==0);}
 else if(!strcmp(argv[1],"map")){struct b2_map m;int v=90;unsigned i;b2_map_init(&m);assert(b2_map_put(&m,0,1)==B2_OK);assert(b2_map_put(&m,7,2)==B2_OK);assert(b2_map_remove(&m,0)==B2_OK);assert(b2_map_put(&m,7,3)==B2_OK&&m.count==1);assert(b2_map_get(&m,7,&v)==B2_OK&&v==3);b2_map_init(&m);for(i=0;i<7;i++)assert(b2_map_put(&m,(uint16_t)i,(int)i)==B2_OK);assert(b2_map_put(&m,70,2)==B2_FULL);for(i=0;i<7;i++)assert(b2_map_remove(&m,(uint16_t)i)==B2_OK);assert(b2_map_get(&m,77,&v)==B2_ABSENT);assert(b2_map_put(&m,77,4)==B2_OK);}
 else if(!strcmp(argv[1],"sort")){struct b2_item a[]={{2,0},{1,1},{2,2},{1,3}},s[4];assert(b2_sort(a,4,s,3)==B2_FULL&&a[0].serial==0);assert(b2_sort(a,4,s,4)==B2_OK);assert(a[0].serial==1&&a[1].serial==3&&a[2].serial==0&&a[3].serial==2);assert(b2_sort(0,0,0,0)==B2_OK);}
 else if(!strcmp(argv[1],"arena")){struct b2_arena a;struct b2_mark m,n;size_t o=99;unsigned i;b2_arena_init(&a);assert(b2_mark(&a,&m)==B2_OK);assert(b2_reserve(&a,3,1,&o)==B2_OK&&o==0);assert(b2_mark(&a,&n)==B2_OK);assert(b2_reserve(&a,5,8,&o)==B2_OK&&o==8);assert(b2_rollback(&a,m)==B2_STALE&&a.cursor==13);assert(b2_rollback(&a,n)==B2_OK&&a.cursor==3);assert(b2_rollback(&a,m)==B2_OK&&a.cursor==0);assert(b2_rollback(&a,m)==B2_STALE);for(i=2;i<15;i++){assert(b2_mark(&a,&n)==B2_OK);assert(b2_rollback(&a,n)==B2_OK);}assert(b2_mark(&a,&n)==B2_FULL);}
 else if(!strcmp(argv[1],"q7")){int8_t a[2]={1,1},b[2]={64,64},o=9;assert(b2_q7(a,b,2,&o)==B2_OK&&o==1);a[0]=-1;assert(b2_q7(a,b,1,&o)==B2_OK&&o==-1);a[0]=-128;b[0]=-128;assert(b2_q7(a,b,1,&o)==B2_OK&&o==127);assert(b2_q7(0,0,0,&o)==B2_OK&&o==0);}
 else if(!strcmp(argv[1],"transaction")){unsigned f;for(f=0;f<=3;f++){struct b2_trace t={f,{0},0};struct b2_backend b={acquire,release,&t};unsigned p=99;int r=b2_transaction(b,&p);if(f==3)assert(r==B2_OK&&p==3&&t.n==3);else{unsigned j;assert(r==B2_FAILED&&p==99&&t.n==2*f+1);for(j=0;j<f;j++)assert(t.trace[f+1+j]==20+f-1-j);}}}
 else if(!strcmp(argv[1],"variant")){struct b2_result r={0,{.number=-123}};unsigned char out[8]={88};size_t w=77;assert(b2_render(&r,out,3,&w)==B2_FULL&&out[0]==88&&w==77);assert(b2_render(&r,out,8,&w)==B2_OK&&w==4&&!memcmp(out,"-123",4));r.tag=9;assert(b2_render(&r,out,8,&w)==B2_INVALID&&w==4);r.tag=1;r.value.text.data=(const unsigned char *)"a\0b";r.value.text.length=3;assert(b2_render(&r,out,8,&w)==B2_OK&&w==3&&out[1]==0);}
 else return 2;
 return 0;
}
