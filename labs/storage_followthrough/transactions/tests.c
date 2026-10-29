#include "transactions.h"
#include "fault_port.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define CHECK(x) do{if(!(x)){fprintf(stderr,"ST_CHECK line%d: %s\n",__LINE__,#x);return 1;}}while(0)
static uint8_t disk[67][4096],saved_disk[67][4096];static unsigned io_calls,io_fail,alloc_calls,fail_at,refresh_visits;
int st_page_admit(uint32_t id,int leaf){(void)id;(void)leaf;alloc_calls++;return alloc_calls!=fail_at;}
void st_refresh_visit(uint32_t id){(void)id;refresh_visits++;}
static int rd(void *c,uint32_t p,void *b){(void)c;if(p>=67)return 1;io_calls++;memcpy(b,disk[p],io_calls==io_fail?137:4096);return io_calls==io_fail;}
static int wr(void *c,uint32_t p,const void *b){(void)c;if(p>=67)return 1;io_calls++;memcpy(disk[p],b,io_calls==io_fail?137:4096);return io_calls==io_fail;}
static int fl(void *c){(void)c;io_calls++;return io_calls==io_fail;}
static int prepare(store *s){store_io io={NULL,rd,wr,fl,67};memset(disk,0,sizeof disk);fail_at=io_fail=0;if(store_format(io)||store_open(s,io)||store_begin(s))return 1;for(unsigned i=0;i<35;i++){uint8_t k=(uint8_t)i;if(store_put(s,&k,1,&k,1))return 1;}return store_commit(s);}
static int get(store *s,unsigned key,unsigned expected){uint8_t k=(uint8_t)key,v=0;size_t n=777;return store_get(s,&k,1,&v,1,&n)!=STORE_OK||n!=1||v!=expected;}
static int original_abort(void){
 for(unsigned cut=1;cut<=3;cut++){store s;CHECK(prepare(&s)==0);CHECK(store_begin(&s)==STORE_OK);uint8_t key=0,v=99;CHECK(store_put(&s,&key,1,&v,1)==STORE_OK);alloc_calls=0;fail_at=cut;key=35;CHECK(store_put(&s,&key,1,&key,1)==STORE_FULL&&alloc_calls==cut&&!s.active);fail_at=0;CHECK(get(&s,0,0)==0);struct st_txn t;CHECK(st_init(&t,&s)==STORE_OK);}
 puts("ST original_abort validated cuts=3");return 0;
}
static int operation(void){
 for(unsigned cut=1;cut<=3;cut++){store s;CHECK(prepare(&s)==0);struct st_txn t;CHECK(st_init(&t,&s)==STORE_OK&&st_begin(&t)==STORE_OK);uint8_t key=0,v=99;CHECK(st_put(&t,&key,1,&v,1)==STORE_OK);store before=s;unsigned io_before=io_calls;alloc_calls=0;fail_at=cut;key=35;CHECK(st_put(&t,&key,1,&key,1)==STORE_FULL&&alloc_calls==cut&&s.active&&io_calls==io_before);CHECK(s.root==before.root&&s.used==before.used&&memcmp(s.scratch,before.scratch,sizeof s.scratch)==0&&get(&s,0,99)==0);fail_at=0;alloc_calls=0;CHECK(st_put(&t,&key,1,&key,1)==STORE_OK&&alloc_calls==3&&store_check(&s)==STORE_OK&&get(&s,35,35)==0);CHECK(st_abort(&t)==STORE_OK&&get(&s,0,0)==0);}
 puts("ST operation validated cuts=3");return 0;
}
static int savepoint(void){
 store s;CHECK(prepare(&s)==0);struct st_txn t;CHECK(st_init(&t,&s)==STORE_OK&&st_begin(&t)==STORE_OK);uint8_t key=0,v=99;CHECK(st_put(&t,&key,1,&v,1)==STORE_OK);struct st_savepoint a,b;CHECK(st_savepoint(&t,&a)==STORE_OK);key=1;CHECK(st_delete(&t,&key,1)==STORE_OK);key=36;CHECK(st_put(&t,&key,1,&key,1)==STORE_OK);CHECK(st_rollback(&t,a)==STORE_OK&&get(&s,0,99)==0&&get(&s,1,1)==0);size_t n=777;CHECK(store_get(&s,&key,1,&v,1,&n)==STORE_NOT_FOUND&&n==777);CHECK(st_savepoint(&t,&b)==STORE_OK&&b.serial!=a.serial);CHECK(st_rollback(&t,a)==STORE_STATE&&t.saved);CHECK(st_release(&t,b)==STORE_OK&&st_release(&t,b)==STORE_STATE);CHECK(st_abort(&t)==STORE_OK);puts("ST savepoint validated");return 0;
}
static int lifecycle(void){
 store s;CHECK(prepare(&s)==0);struct st_txn t;CHECK(st_init(&t,&s)==STORE_OK);CHECK(st_commit(&t)==STORE_STATE&&st_begin(&t)==STORE_OK&&st_begin(&t)==STORE_STATE);uint8_t k=0,v=99;CHECK(st_put(&t,&k,1,&v,1)==STORE_OK);CHECK(st_put(&t,&k,0,&v,1)==STORE_INVALID&&get(&s,0,99)==0);struct st_savepoint a;CHECK(st_savepoint(&t,&a)==STORE_OK&&st_commit(&t)==STORE_OK&&!t.saved);CHECK(st_begin(&t)==STORE_OK&&st_rollback(&t,a)==STORE_STATE);t.serial=UINT64_MAX-1;CHECK(st_savepoint(&t,&a)==STORE_OK&&a.serial==UINT64_MAX);CHECK(st_release(&t,a)==STORE_OK);struct st_savepoint before=a;CHECK(st_savepoint(&t,&a)==STORE_FULL&&a.owner==before.owner&&a.serial==before.serial);CHECK(st_abort(&t)==STORE_OK&&st_commit(&t)==STORE_STATE&&get(&s,0,99)==0);puts("ST lifecycle validated");return 0;
}
static int nosteal(void){
 store s;CHECK(prepare(&s)==0);struct st_txn t;CHECK(st_init(&t,&s)==STORE_OK&&st_begin(&t)==STORE_OK);memcpy(saved_disk,disk,sizeof disk);unsigned before=io_calls;uint8_t key=0,value=99;CHECK(st_put(&t,&key,1,&value,1)==STORE_OK&&st_delete(&t,(uint8_t[]){1},1)==STORE_OK);CHECK(memcmp(saved_disk,disk,sizeof disk)==0&&io_calls==before&&get(&s,0,99)==0);CHECK(st_abort(&t)==STORE_OK&&get(&s,0,0)==0&&get(&s,1,1)==0);
 CHECK(st_begin(&t)==STORE_OK&&st_put(&t,&key,1,&value,1)==STORE_OK&&st_commit(&t)==STORE_OK);uint8_t bytes[4096];CHECK(disk[3][8]==1&&disk[35][8]==2&&s.wal_active);CHECK(store_page_read(&s,1,bytes)==STORE_OK&&bytes[8]==2&&get(&s,0,99)==0);puts("ST nosteal validated home-generation1 wal-generation2");return 0;
}
static int capacity(void){
 store s;CHECK(prepare(&s)==0);struct st_txn t;CHECK(st_init(&t,&s)==STORE_OK&&st_begin(&t)==STORE_OK);uint8_t k=0,v=99;CHECK(st_put(&t,&k,1,&v,1)==STORE_OK);unsigned stop=0;
 for(unsigned i=35;i<256;i++){k=(uint8_t)i;int r=st_put(&t,&k,1,&k,1);if(r==STORE_FULL){stop=i;break;}CHECK(r==STORE_OK&&store_check(&s)==STORE_OK);}
 CHECK(stop>35&&stop<256&&s.active);for(unsigned i=0;i<stop;i++)CHECK(get(&s,i,i?i:99)==0);store baseline=s;k=(uint8_t)stop;CHECK(store_put(&baseline,&k,1,&k,1)==STORE_FULL&&!baseline.active&&get(&baseline,0,0)==0);alloc_calls=0;k=0;v=101;CHECK(st_put(&t,&k,1,&v,1)==STORE_OK&&alloc_calls==0&&get(&s,0,101)==0);CHECK(st_abort(&t)==STORE_OK&&get(&s,0,0)==0);printf("ST capacity validated retained=%u\n",stop);return 0;
}
static int uncertain(void){
 for(unsigned i=0;i<2;i++){store s;CHECK(prepare(&s)==0);struct st_txn t;CHECK(st_init(&t,&s)==STORE_OK&&st_begin(&t)==STORE_OK);struct st_savepoint point;CHECK(st_savepoint(&t,&point)==STORE_OK);uint8_t key=0,v=99;CHECK(st_put(&t,&key,1,&v,1)==STORE_OK);io_fail=io_calls+(i?37:1);CHECK(st_commit(&t)==(i?STORE_INDETERMINATE:STORE_IO)&&s.poisoned&&!t.saved);CHECK(st_rollback(&t,point)==STORE_STATE);io_fail=0;store recovered;store_io io={NULL,rd,wr,fl,67};CHECK(store_open(&recovered,io)==STORE_OK&&get(&recovered,0,i?99:0)==0);}
 puts("ST uncertain validated old-or-new-selected-model");return 0;
}
static unsigned bits(uint32_t n){unsigned c=0;while(n){c+=n&1u;n>>=1;}return c;}
static int cost(void){store s;CHECK(prepare(&s)==0);struct st_txn t;CHECK(st_init(&t,&s)==STORE_OK&&st_begin(&t)==STORE_OK);uint8_t k=0,v=99;refresh_visits=0;alloc_calls=0;CHECK(st_put(&t,&k,1,&v,1)==STORE_OK&&alloc_calls==0&&refresh_visits==bits(s.used)-1);printf("ST cost validated store=%zu wrapper=%zu scratch=%zu clean=%zu refresh=%u\n",sizeof(store),sizeof(struct st_txn),sizeof s.scratch,sizeof s.cache,refresh_visits);return 0;}
static int oracle(void){
 store_io io={NULL,rd,wr,fl,67};memset(disk,0,sizeof disk);fail_at=0;CHECK(store_format(io)==STORE_OK);memcpy(saved_disk,disk,sizeof disk);store s;struct st_txn t;struct st_savepoint point={0};unsigned reset,op;
 while(scanf("%u %u",&reset,&op)==2){if(reset){memcpy(disk,saved_disk,sizeof disk);CHECK(store_open(&s,io)==STORE_OK&&st_init(&t,&s)==STORE_OK&&st_begin(&t)==STORE_OK);point=(struct st_savepoint){0};}
  int r;uint8_t key=(uint8_t)(op==2),value=(uint8_t)op;
  switch(op){case 1:case 2:r=st_put(&t,&key,1,&value,1);break;case 3:key=0;r=st_delete(&t,&key,1);break;case 4:r=st_savepoint(&t,&point);break;case 5:r=st_rollback(&t,point);break;case 6:r=st_release(&t,point);break;case 7:r=st_abort(&t);break;case 8:r=st_begin(&t);break;case 9:r=st_commit(&t);break;default:return 2;}
  unsigned mask=0;for(unsigned i=0;i<2;i++){uint8_t k=(uint8_t)i,v=0;size_t n=0;int result=store_get(&s,&k,1,&v,1,&n);CHECK(result==STORE_OK||result==STORE_NOT_FOUND);if(result==STORE_OK){CHECK(n==1&&v==i+1);mask|=1u<<i;}}
  printf("%d %d %d %llu %llu %u\n",r,s.active,t.saved,(unsigned long long)t.serial,(unsigned long long)s.generation,mask);
 }
 return 0;
}
int main(int argc,char **argv){if(argc!=2)return 2;
 if(!strcmp(argv[1],"original_abort"))return original_abort();
 if(!strcmp(argv[1],"operation"))return operation();
 if(!strcmp(argv[1],"savepoint"))return savepoint();
 if(!strcmp(argv[1],"lifecycle"))return lifecycle();
 if(!strcmp(argv[1],"nosteal"))return nosteal();
 if(!strcmp(argv[1],"cost"))return cost();
 if(!strcmp(argv[1],"capacity"))return capacity();
 if(!strcmp(argv[1],"uncertain"))return uncertain();
 if(!strcmp(argv[1],"oracle"))return oracle();
 return 2;
}
