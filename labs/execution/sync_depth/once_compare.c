/* Hosted library comparison only; not a permitted implementation dependency. */
#include <threads.h>
#include <stdatomic.h>
#include <stdio.h>
#ifndef CDO_COMPARE_MUTANT
#define CDO_COMPARE_MUTANT 0
#endif
#define CHECK(c) do{if(!(c)){fprintf(stderr,"CDOC_CHECK:%d:%s\n",__LINE__,#c);return 1;}}while(0)
static once_flag failed_flag=ONCE_FLAG_INIT,ready_flag=ONCE_FLAG_INIT;
static unsigned failed_attempts,ready_attempts,table[3];
static int application_status;
static atomic_uint start;
static void fail_normally(void){failed_attempts++;application_status=-7;}
static void initialize(void){ready_attempts++;table[0]=17;table[1]=23;table[2]=table[0]+table[1];}
static int reader(void *unused){
  (void)unused;while(!atomic_load_explicit(&start,memory_order_acquire))thrd_yield();
  call_once(&ready_flag,initialize);
  return table[0]!=17||table[1]!=23||table[2]!=40;
}
int main(void){
  call_once(&failed_flag,fail_normally);call_once(&failed_flag,fail_normally);
  printf("CDOC_FAILURE:attempts=%u;application_status=%d;no-library-error-result\n",failed_attempts,application_status);
  CHECK(failed_attempts==(CDO_COMPARE_MUTANT?2u:1u)&&application_status==-7);
  thrd_t threads[3];atomic_init(&start,0);
  for(unsigned i=0;i<3;i++)CHECK(thrd_create(&threads[i],reader,0)==thrd_success);
  atomic_store_explicit(&start,1,memory_order_release);
  for(unsigned i=0;i<3;i++){int result=-1;CHECK(thrd_join(threads[i],&result)==thrd_success&&result==0);}
  CHECK(ready_attempts==1);
  puts("CDOC_READY:threads3;attempt1;ordinary-table17+23=40;completion-publication");return 0;
}
