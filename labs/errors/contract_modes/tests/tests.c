#include "contract_modes.h"
#include <stdio.h>
#include <string.h>
#include <time.h>
#define CHECK(x) do{if(!(x)){fprintf(stderr,"ER_CHECK ECM line%d: %s\n",__LINE__,#x);return 1;}}while(0)
static int base(void){int c=90;size_t r=0;CHECK(er_contract_counter(ER_ENFORCE,20,&c,&r)==ER_CONTRACT&&c==90&&r==1);CHECK(er_contract_counter(ER_OBSERVE,20,&c,&r)==ER_OK&&c==100&&r==2);CHECK(er_contract_counter(ER_OFF,20,&c,&r)==ER_OK&&c==100&&r==2);r=SIZE_MAX;CHECK(er_contract_counter(ER_OBSERVE,1,&c,&r)==ER_RANGE&&c==100&&r==SIZE_MAX);puts("ECM_BASE actual-counter-modes enforce-unchanged observe-clamped report-exhaustion");return 0;}
static int bounds(void){
 unsigned char bytes[8]={0};struct ecm_buffer b;CHECK(ecm_init(&b,bytes,3,0)==ER_OK);
 for(unsigned mode=0;mode<3;++mode){CHECK(ecm_write(&b,(er_mode)mode,5,99)==ER_CONTRACT&&bytes[5]==0);CHECK(ecm_write(&b,(er_mode)mode,2,7)==ER_OK&&bytes[2]==7);}
 CHECK(b.reports==2);puts("ECM_BOUNDS every-mode-gates-memory unchecked-primitive-never-called-on-bad-index");return 0;
}
static int exhaustion(void){unsigned char bytes[8]={0};struct ecm_buffer b;CHECK(ecm_init(&b,bytes,3,SIZE_MAX)==ER_OK);CHECK(ecm_write(&b,ER_OBSERVE,SIZE_MAX,1)==ER_RANGE&&b.reports==SIZE_MAX&&bytes[0]==0);CHECK(ecm_write(&b,ER_ENFORCE,3,1)==ER_RANGE&&b.reports==SIZE_MAX);CHECK(ecm_write(&b,ER_OFF,3,1)==ER_CONTRACT&&b.reports==SIZE_MAX);CHECK(ecm_write(&b,ER_ENFORCE,0,9)==ER_OK&&bytes[0]==9);puts("ECM_EXHAUSTION report-limit-preserves bytes-off-still-rejects valid-progress");return 0;}
static int admission(void){struct ecm_buffer b={0};b.reports=91;CHECK(ecm_init(&b,NULL,1,0)==ER_INVALID&&b.reports==91);CHECK(ecm_init(&b,NULL,0,0)==ER_OK);CHECK(ecm_write(&b,ER_OFF,0,1)==ER_CONTRACT);CHECK(ecm_write(&b,(er_mode)3,0,1)==ER_INVALID&&b.reports==0);CHECK(ecm_write(NULL,ER_OFF,0,1)==ER_INVALID);puts("ECM_ADMISSION empty-span-null no-pointer-arithmetic invalid-mode-unchanged");return 0;}
struct er_worker {struct ecm_shared *shared;unsigned loops;int ok;};
static int worker(void *p){struct er_worker *w=p;w->ok=1;for(unsigned i=0;i<w->loops;++i){if(ecm_shared_write(w->shared,ER_OBSERVE,7,99)!=ER_CONTRACT||ecm_shared_counter(w->shared,ER_OBSERVE,20)!=ER_OK){w->ok=0;break;}}return 0;}
#ifdef ECM_POSIX_THREADS
static void *posix_worker(void *p){(void)worker(p);return NULL;}
#endif
static int concurrent(void){
 unsigned char bytes[8]={0};struct ecm_shared s;CHECK(ecm_shared_init(&s,bytes,3,90,0)==ER_OK);struct er_worker work[4];unsigned made=0;int joined=1;
#ifdef ECM_POSIX_THREADS
 pthread_t thread[4];
 for(;made<4;++made){work[made]=(struct er_worker){&s,1000,0};if(pthread_create(&thread[made],NULL,posix_worker,&work[made])!=0)break;}
 for(unsigned i=0;i<made;++i){void *result;if(pthread_join(thread[i],&result)!=0||result!=NULL||!work[i].ok)joined=0;}
#else
 thrd_t thread[4];
 for(;made<4;++made){work[made]=(struct er_worker){&s,1000,0};if(thrd_create(&thread[made],worker,&work[made])!=thrd_success)break;}
 for(unsigned i=0;i<made;++i){int result;if(thrd_join(thread[i],&result)!=thrd_success||result!=0||!work[i].ok)joined=0;}
#endif
 CHECK(made==4&&joined);struct ecm_snapshot snap;CHECK(ecm_shared_snapshot(&s,&snap)==ER_OK&&snap.counter==100&&snap.reports==8000&&snap.length==3&&bytes[7]==0);ecm_shared_destroy(&s);
 CHECK(ecm_shared_init(&s,bytes,3,90,SIZE_MAX-1)==ER_OK);CHECK(ecm_shared_write(&s,ER_OBSERVE,3,1)==ER_CONTRACT);CHECK(ecm_shared_counter(&s,ER_OBSERVE,20)==ER_RANGE);CHECK(ecm_shared_snapshot(&s,&snap)==ER_OK&&snap.counter==90&&snap.reports==SIZE_MAX);ecm_shared_destroy(&s);
 puts("ECM_CONCURRENT four-threads 8000-shared-reports exact-count joined-before-destroy shared-exhaustion");return 0;
}
static int oracle(void){unsigned kind,mode,n,index;int counter,amount;size_t reports;
 while(scanf("%u %u %u %u %d %d %zu",&kind,&mode,&n,&index,&counter,&amount,&reports)==7){
  if(kind==0){er_code r=er_contract_counter((er_mode)mode,amount,&counter,&reports);printf("%d %d %zu\n",r,counter,reports);}
  else {unsigned char bytes[8]={0};struct ecm_buffer b={bytes,n,reports};er_code r=ecm_write(&b,(er_mode)mode,index,17);printf("%d %zu",r,b.reports);for(unsigned i=0;i<8;++i)printf(" %u",bytes[i]);putchar('\n');}
 }return ferror(stdin)?2:0;}
static int metrics(void){double samples[7];unsigned long sum=0;for(unsigned sample=0;sample<7;++sample){clock_t start=clock();for(unsigned i=0;i<10000;++i){unsigned char bytes[8]={0};struct ecm_buffer b;CHECK(ecm_init(&b,bytes,3,0)==ER_OK&&ecm_write(&b,ER_ENFORCE,3,1)==ER_CONTRACT&&ecm_write(&b,ER_OFF,0,7)==ER_OK);sum+=bytes[0]+b.reports;}samples[sample]=(double)(clock()-start)/CLOCKS_PER_SEC;}printf("{\"iterations\":10000,\"checksum\":%lu,\"allocation_calls\":0,\"buffer_size\":%zu,\"shared_size\":%zu,\"seconds\":[",sum,sizeof(struct ecm_buffer),sizeof(struct ecm_shared));for(unsigned i=0;i<7;++i)printf("%s%.9f",i?",":"",samples[i]);puts("]}");return 0;}
int main(int argc,char **argv){if(argc!=2)return 2;
 if(!strcmp(argv[1],"modes_base"))return base();
 if(!strcmp(argv[1],"modes_bounds"))return bounds();
 if(!strcmp(argv[1],"modes_exhaustion"))return exhaustion();
 if(!strcmp(argv[1],"modes_admission"))return admission();
 if(!strcmp(argv[1],"modes_concurrent"))return concurrent();
 if(!strcmp(argv[1],"oracle"))return oracle();
 if(!strcmp(argv[1],"metrics"))return metrics();
 return 2;}
