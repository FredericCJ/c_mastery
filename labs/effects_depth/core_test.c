#include "effects.h"
#include "logger.h"
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CHECK(x) do {if(!(x)){fprintf(stderr,"EFFECTS_ASSERT:%d:%s\n",__LINE__,#x);return 1;}}while(0)
struct ef_alloc {unsigned requests,acquisitions,releases,live,fail;unsigned char storage[32];};
static unsigned char*take(void*v,size_t n){struct ef_alloc*a=v;++a->requests;if(n!=32||a->fail||a->live)return NULL;++a->acquisitions;a->live=32;return a->storage;}
static void release(void*v,unsigned char*p){struct ef_alloc*a=v;if(p!=a->storage||a->live!=32)abort();++a->releases;a->live=0;}
static int callback(void*v,unsigned char*p,size_t n){unsigned*a=v;if(n!=16)abort();p[0]=1;return ++*a==3?1:0;}
static int never_ready(void*v,unsigned char*p,size_t n){(void)v;(void)p;(void)n;return 0;}
static int compare(const void*a,const void*b){int x=*(const int*)a,y=*(const int*)b;return(x>y)-(x<y);}
int main(void){
 struct ef_alloc a={0};struct ef_cache c={{&a,take,release},NULL,1,0,0,0};uint32_t value=99;
 CHECK(ef_checksum(NULL,0,&value)==EF_OK&&value==0);
 CHECK(ef_checksum(NULL,1,&value)==EF_INVALID&&value==0);
 CHECK(ef_cached(&c,(const unsigned char*)"abc",3,&value)==EF_OK&&value==294);
 CHECK(a.requests==1&&a.acquisitions==1&&a.live==32);
 for(unsigned i=0;i<10;++i)CHECK(ef_cached(&c,(const unsigned char*)"abc",3,&value)==EF_OK);
 CHECK(a.requests==1);const unsigned char*borrow=NULL;
 CHECK(ef_cache_borrow(&c,&borrow)==EF_OK&&borrow==a.storage);
 CHECK(ef_cache_reset(&c)==EF_BUSY&&a.live==32&&c.generation==1);
 CHECK(ef_cached(&c,NULL,0,&value)==EF_BUSY);
 CHECK(ef_cache_return(&c)==EF_OK&&ef_cache_reset(&c)==EF_OK&&a.live==0&&c.generation==2);
 a.fail=1;value=77;CHECK(ef_cached(&c,NULL,0,&value)==EF_NOMEM&&value==77);
 a.fail=0;CHECK(ef_cached(&c,NULL,0,&value)==EF_OK&&a.requests==3);
 CHECK(ef_cache_reset(&c)==EF_OK);c.permanent=1;a.fail=1;
 CHECK(ef_cached(&c,NULL,0,&value)==EF_NOMEM);a.fail=0;
 CHECK(ef_cached(&c,NULL,0,&value)==EF_FAILED&&a.requests==4);
 CHECK(ef_cache_reset(&c)==EF_OK&&ef_cached(&c,NULL,0,&value)==EF_OK&&a.requests==5);
 CHECK(ef_cache_reset(&c)==EF_OK);c.generation=UINT32_MAX;
 CHECK(ef_cache_reset(&c)==EF_FULL&&c.generation==UINT32_MAX);
 struct ef_pool pool={0};unsigned char*p=NULL;
 for(unsigned i=0;i<16;++i)CHECK(ef_pool_take(&pool,&p)==EF_OK&&p==&pool.bytes[i]);
 CHECK(ef_pool_take(&pool,&p)==EF_FULL&&pool.requests==17);
 CHECK(ef_pool_return(&pool,7)==EF_OK&&ef_pool_take(&pool,&p)==EF_OK&&p==&pool.bytes[7]);
 CHECK(pool.requests==18&&pool.used==65535);CHECK(ef_pool_return(&pool,16)==EF_INVALID);
 struct ef_parser first={{0},0,7,1,0},second={{0},0,7,1,0};struct ef_view x={0},y={0};
 CHECK(ef_parse(&first,(const unsigned char*)"abc",3,&x)==EF_OK);
 CHECK(ef_parse(&second,(const unsigned char*)"def",3,&y)==EF_OK&&memcmp(x.data,"ABC",3)==0&&memcmp(y.data,"DEF",3)==0);
 struct ef_view unchanged=x;CHECK(ef_parse(&first,(const unsigned char*)"z",1,&unchanged)==EF_BUSY&&memcmp(x.data,"ABC",3)==0);
 CHECK(ef_parser_reset(&first,7)==EF_BUSY&&first.cursor==3);
 CHECK(ef_view_release(&x)==EF_OK&&ef_view_release(&x)==EF_INVALID);
 uint32_t random1=ef_random(&first);CHECK(random1==1025555898u);
 CHECK(ef_parser_reset(&first,7)==EF_OK&&first.cursor==0&&ef_random(&first)==random1);
 CHECK(ef_view_release(&y)==EF_OK);first.cursor=SIZE_MAX;
 CHECK(ef_parse(&first,(const unsigned char*)"a",1,&x)==EF_FULL);
 unsigned calls=0,used=99;CHECK(ef_attempt(callback,&calls,1,2,&used)==EF_INCOMPLETE&&used==2&&calls==2);
 CHECK(ef_attempt(callback,&calls,1,1,&used)==EF_OK&&used==1&&calls==3);
 CHECK(ef_attempt(callback,&calls,0,1,&used)==EF_INVALID&&calls==3);
 CHECK(ef_attempt(callback,&calls,1,0,&used)==EF_INCOMPLETE&&used==0);
 CHECK(ef_attempt(never_ready,NULL,1,16,&used)==EF_INCOMPLETE&&used==16);
 for(size_t n=0;n<=16;++n)for(unsigned distribution=0;distribution<5;++distribution){int input[17],expected[17];for(size_t i=0;i<17;++i)input[i]=distribution==0?(int)i:distribution==1?(int)(16-i):distribution==2?(int)(i%3):distribution==3?(i&1?INT_MIN:INT_MAX):(int)((i*19+7)%13);memcpy(expected,input,sizeof input);qsort(expected,n,sizeof *expected,compare);CHECK(ef_sort(input,n)==EF_OK&&memcmp(input,expected,sizeof input)==0);}
 int rejected[17]={2,1};CHECK(ef_sort(rejected,17)==EF_INVALID&&rejected[0]==2&&rejected[1]==1);
 struct ef_record r[16];for(unsigned i=0;i<16;++i){r[i].key=(int)(i%3);r[i].identity=i;for(unsigned j=0;j<32;++j)r[i].payload[j]=(unsigned char)i;}
 CHECK(ef_sort_records(r,16)==EF_OK);for(unsigned i=0;i<16;++i){CHECK(r[i].payload[0]==r[i].identity);if(i)CHECK(r[i-1].key<r[i].key||(r[i-1].key==r[i].key&&r[i-1].identity<r[i].identity));}
 struct ef_log log={ATOMIC_FLAG_INIT,{0},0};CHECK(ef_log(&log,(const unsigned char*)"event",5)==1&&log.length==5);
 CHECK(!atomic_flag_test_and_set_explicit(&log.owned,memory_order_acquire));
 CHECK(ef_log(&log,(const unsigned char*)"nested",6)==0&&log.length==5&&memcmp(log.bytes,"event",5)==0);
 atomic_flag_clear_explicit(&log.owned,memory_order_release);
 CHECK(ef_log(&log,NULL,33)==-1&&log.length==5);
 puts("EFFECTS_CORE_PASS cache pool parser bounded callback stable sort logger");return 0;
}
