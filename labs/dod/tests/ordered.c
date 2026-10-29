#include "dod_ordered.h"
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
static const char *test_name="ordered";
#define CHECK(x) do{if(!(x)){fprintf(stderr,"DOD_CHECK:%s:%d:%s\n",test_name,__LINE__,#x);exit(1);}}while(0)
static unsigned collect(struct d_ordered *o,int *values){struct d_ordered_iter it;CHECK(d_order_begin(o,&it)==D_OK);struct d_handle h;const int *value;unsigned has,n=0;for(;;){CHECK(d_order_next(o,&it,&h,&value,&has)==D_OK);if(!has)break;CHECK(n<4&&h.owner==&o->pool);int v;CHECK(d_sparse_get(&o->pool,h,&v)==D_OK&&v==*value);values[n++]=v;}return n;}
static void checks(void){
 struct d_ordered o;d_order_init(&o);struct d_handle h[4];for(unsigned i=0;i<4;++i)CHECK(d_order_add(&o,(int)i,&h[i])==D_OK);int values[4];CHECK(collect(&o,values)==4);for(unsigned i=0;i<4;++i)CHECK(values[i]==(int)i);CHECK(o.link_writes==16&&o.iteration_steps==4);
 struct d_ordered_iter it;CHECK(d_order_begin(&o,&it)==D_OK);const int *stable=&o.pool.slots[h[2].slot].value;CHECK(d_order_remove(&o,h[1])==D_OK&&stable==&o.pool.slots[h[2].slot].value&&*stable==2);struct d_handle output={0};const int *borrow=NULL;unsigned has=99;CHECK(d_order_next(&o,&it,&output,&borrow,&has)==D_STALE&&has==99&&borrow==NULL&&output.owner==NULL);
 CHECK(d_order_add(&o,9,&output)==D_OK&&output.slot==h[1].slot&&output.generation!=h[1].generation);CHECK(collect(&o,values)==4&&values[0]==0&&values[1]==2&&values[2]==3&&values[3]==9);int v=77;CHECK(d_sparse_get(&o.pool,h[1],&v)==D_STALE&&v==77);CHECK(*stable==2);CHECK(o.link_writes==24);
 o.version=UINT_MAX;CHECK(d_order_remove(&o,h[0])==D_FULL&&d_order_add(&o,3,&output)==D_FULL&&o.pool.count==4);d_order_init(&o);
 for(unsigned i=0;i<12;++i){CHECK(d_order_add(&o,(int)i,&output)==D_OK&&output.slot==i/3&&d_order_remove(&o,output)==D_OK&&o.head==4&&o.tail==4);}CHECK(d_order_add(&o,1,&output)==D_FULL&&o.link_writes==96);CHECK(collect(&o,values)==0);
 o.iteration_steps=UINT_MAX;d_order_init(&o);CHECK(d_order_add(&o,3,&output)==D_OK);o.iteration_steps=UINT_MAX;CHECK(collect(&o,values)==1&&o.iteration_steps==UINT_MAX);
}
static void oracle(void){for(unsigned removed=0;removed<4;++removed){struct d_ordered o;d_order_init(&o);struct d_handle h[4],added;for(unsigned i=0;i<4;++i)CHECK(d_order_add(&o,(int)i,&h[i])==D_OK);CHECK(d_order_remove(&o,h[removed])==D_OK&&d_order_add(&o,9,&added)==D_OK);int values[4];CHECK(collect(&o,values)==4);printf("O %u %u %u",removed,o.link_writes,o.iteration_steps);for(unsigned i=0;i<4;++i)printf(" %d",values[i]);putchar('\n');}}
int main(int argc,char **argv){CHECK(argc==2);test_name=argv[1];if(!strcmp(argv[1],"ordered"))checks();else if(!strcmp(argv[1],"oracle"))oracle();else CHECK(0);return 0;}
