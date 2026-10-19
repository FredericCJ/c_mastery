/* Owned disposable Linux guest only. No host policy or global knobs. */
#define _GNU_SOURCE
#include "policy.h"
#include <errno.h>
#include <fcntl.h>
#include <inttypes.h>
#include <poll.h>
#include <sched.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/resource.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <sys/utsname.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>
#ifndef LC_MUTANT
#define LC_MUTANT 0
#endif
#define JOBS 20u
#define WORK 50000u
struct Attr {uint32_t size,policy;uint64_t flags;int32_t nice;uint32_t priority;uint64_t runtime,deadline,period;};
typedef struct {uint64_t release,start,finish,cpu,checksum;unsigned missed;} Job;
typedef struct {int policy,denied,setup_error;uint64_t mask,flags,q,d,p,quantum;unsigned jobs,overruns;Job job[JOBS];} Result;
static volatile sig_atomic_t overruns;
static void overrun(int sig){(void)sig;if(overruns<100000)++overruns;}
static uint64_t now(clockid_t id){struct timespec t;if(clock_gettime(id,&t))_exit(91);return (uint64_t)t.tv_sec*UINT64_C(1000000000)+(uint64_t)t.tv_nsec;}
static int affinity(uint64_t mask){cpu_set_t set;CPU_ZERO(&set);for(unsigned i=0;i<64;++i)if(mask&(UINT64_C(1)<<i))CPU_SET(i,&set);return sched_setaffinity(0,sizeof set,&set);}
static uint64_t affinity_get(void){cpu_set_t set;CPU_ZERO(&set);if(sched_getaffinity(0,sizeof set,&set))return 0;uint64_t mask=0;for(unsigned i=0;i<64;++i)if(CPU_ISSET(i,&set))mask|=UINT64_C(1)<<i;return mask;}
static void limits(void){struct rlimit cpu={1,2},rt={200000,300000};if(setrlimit(RLIMIT_CPU,&cpu)||setrlimit(RLIMIT_RTTIME,&rt))_exit(92);}
static int stop(pid_t pid){
 if(pid<=0)return 1;
 if(kill(pid,SIGKILL)&&errno!=ESRCH)return 0;
 int status;pid_t collected;do{collected=waitpid(pid,&status,0);}while(collected<0&&errno==EINTR);
 return collected==pid;
}
static void worker(Result *r,unsigned policy,uint64_t mask,const char *behavior){
 limits();if(affinity(mask)){r->setup_error=errno;_exit(93);}
 struct sigaction action={0};action.sa_handler=overrun;sigemptyset(&action.sa_mask);if(sigaction(SIGXCPU,&action,0))_exit(94);
 if(!strcmp(behavior,"denied")&&setuid(65534))_exit(95);
 struct Attr attr={.size=sizeof attr,.policy=policy,.flags=1,.priority=policy==SCHED_FIFO||policy==SCHED_RR?2:0};
 if(policy==6){attr.flags|=4;attr.runtime=3000000;attr.deadline=5000000;attr.period=10000000;}
 struct Attr requested=attr;
 if(LC_MUTANT==3){attr.policy=0;attr.priority=0;attr.flags=0;attr.runtime=attr.deadline=attr.period=0;}
 if(syscall(SYS_sched_setattr,0,&attr,0)){r->denied=errno;r->policy=sched_getscheduler(0);r->mask=affinity_get();_exit(77);}
 struct Attr got={.size=sizeof got};if(syscall(SYS_sched_getattr,0,&got,sizeof got,0))_exit(96);
 r->policy=(int)got.policy;r->mask=affinity_get();r->flags=got.flags;r->q=got.runtime;r->d=got.deadline;r->p=got.period;
 if(r->policy!=(int)requested.policy||r->mask!=mask||(policy==6&&(got.runtime!=requested.runtime||got.deadline!=requested.deadline||got.period!=requested.period))){r->setup_error=EIO;_exit(97);}
 if(policy==SCHED_RR){struct timespec q;if(sched_rr_get_interval(0,&q))_exit(98);r->quantum=(uint64_t)q.tv_sec*1000000000u+(uint64_t)q.tv_nsec;}
 if(!strcmp(behavior,"infinite")){volatile uint64_t spin=1;for(;;)spin=spin*6364136223846793005u+1;}
 uint64_t base=now(CLOCK_MONOTONIC)+10000000u;unsigned jobs=!strcmp(behavior,"overrun")?4:JOBS;
 for(unsigned i=0;i<jobs;++i){
  Job *j=&r->job[i];j->release=base+(uint64_t)i*10000000u;struct timespec release={(time_t)(j->release/1000000000u),(long)(j->release%1000000000u)};int error;do{error=clock_nanosleep(CLOCK_MONOTONIC,TIMER_ABSTIME,&release,0);}while(error==EINTR);if(error)_exit(99);
  j->start=now(CLOCK_MONOTONIC);uint64_t cpu=now(CLOCK_THREAD_CPUTIME_ID);volatile uint64_t state=1;
  for(unsigned n=0;n<WORK;++n)state=state*6364136223846793005u+1;
  if(!strcmp(behavior,"overrun"))while(now(CLOCK_THREAD_CPUTIME_ID)-cpu<8000000u)state=state*6364136223846793005u+1;
  j->cpu=now(CLOCK_THREAD_CPUTIME_ID)-cpu;j->finish=now(CLOCK_MONOTONIC);j->checksum=state;j->missed=j->finish>j->release+5000000u;++r->jobs;
 }
 r->overruns=(unsigned)overruns;_exit(0);
}
static pid_t background(uint64_t mask,const char *kind){
 if(!strcmp(kind,"none"))return 0;
 int ready[2];if(pipe(ready))return -1;pid_t pid=fork();
 if(pid==0){close(ready[0]);limits();if(affinity(mask))_exit(90);if(!strcmp(kind,"batch")){struct sched_param p={0};if(sched_setscheduler(0,SCHED_BATCH,&p))_exit(90);}
  int policy=sched_getscheduler(0);if(write(ready[1],&policy,sizeof policy)!=(ssize_t)sizeof policy)_exit(90);close(ready[1]);
  if(!strcmp(kind,"io")){int fd=open("/tmp/synthetic-interference",O_CREAT|O_TRUNC|O_RDWR,0600);if(fd<0)_exit(90);unsigned char bytes[512]={0};for(;;){if(pwrite(fd,bytes,sizeof bytes,0)!=(ssize_t)sizeof bytes||pread(fd,bytes,sizeof bytes,0)!=(ssize_t)sizeof bytes)_exit(90);}}
  volatile uint64_t value=1;for(;;)value=value*6364136223846793005u+1;
 }
 close(ready[1]);struct pollfd poller={ready[0],POLLIN,0};int policy=-1;if(pid<0||poll(&poller,1,1000)!=1||read(ready[0],&policy,sizeof policy)!=(ssize_t)sizeof policy){close(ready[0]);stop(pid);return -1;}close(ready[0]);printf("LC_BACKGROUND kind=%s policy=%d mask=%"PRIu64"\n",kind,policy,mask);return pid;
}
int main(int argc,char **argv){
 if(argc!=5)return 2;
 const char *name=argv[1],*restricted=argv[2],*interference=argv[3],*behavior=argv[4];
 unsigned policy=!strcmp(name,"other")?SCHED_OTHER:!strcmp(name,"fifo")?SCHED_FIFO:!strcmp(name,"rr")?SCHED_RR:!strcmp(name,"deadline")?6:99;
 if(policy==99||(strcmp(restricted,"all")&&strcmp(restricted,"one"))||(strcmp(interference,"none")&&strcmp(interference,"batch")&&strcmp(interference,"io"))||(strcmp(behavior,"normal")&&strcmp(behavior,"infinite")&&strcmp(behavior,"overrun")&&strcmp(behavior,"denied")&&strcmp(behavior,"invalid")))return 2;
 struct utsname uts;char token[32]={0};int marker=open("/run/cmastery-owned-vm",O_RDONLY);if(marker<0||read(marker,token,sizeof token-1)!=17||strcmp(token,"CM-SCHED-OWNED-1\n")||uname(&uts)||!strstr(uts.release,"cmastery-b06")||getuid()!=0){if(marker>=0)close(marker);fputs("LC_GUEST_CHECK:requires-owned-image\n",stderr);return 1;}close(marker);
 if(!strcmp(behavior,"invalid")){if(lc_parameters(3,2,10)){fputs("LC_GUEST_CHECK:invalid-parameters\n",stderr);return 1;}puts("LC_INVALID_REFUSED:Q3-D2-P10-before-policy-call");return 0;}
 uint64_t available=affinity_get();if(!available||!(available&1u))return 1;unsigned last=0;for(unsigned i=0;i<64;++i)if(available&(UINT64_C(1)<<i))last=i;
 if(last==0&&(policy==SCHED_FIFO||policy==SCHED_RR)&&!strcmp(behavior,"infinite")){fputs("LC_GUEST_CHECK:unsafe-supervisor-placement\n",stderr);return 1;}
 if(affinity(1))return 1;
 uint64_t mask=!strcmp(restricted,"one")?UINT64_C(1)<<last:available;
 Result *r=mmap(0,sizeof *r,PROT_READ|PROT_WRITE,MAP_SHARED|MAP_ANONYMOUS,-1,0);if(r==MAP_FAILED)return 1;
 pid_t bg=background(UINT64_C(1)<<last,interference);if(bg<0){munmap(r,sizeof *r);return 1;}fflush(stdout);pid_t child=fork();if(child==0)worker(r,policy,mask,behavior);if(child<0){stop(bg);munmap(r,sizeof *r);return 1;}
 uint64_t start=now(CLOCK_MONOTONIC),bound=!strcmp(behavior,"infinite")?250000000u:4000000000u;int status=0,killed=0;pid_t collected;
 for(;;){collected=waitpid(child,&status,WNOHANG);if(collected==child)break;if(collected<0){if(errno==EINTR)continue;stop(child);stop(bg);munmap(r,sizeof *r);return 1;}
  if(now(CLOCK_MONOTONIC)-start>=bound){if(kill(child,SIGKILL)&&errno!=ESRCH){stop(child);stop(bg);munmap(r,sizeof *r);return 1;}killed=1;do{collected=waitpid(child,&status,0);}while(collected<0&&errno==EINTR);break;}struct timespec nap={0,1000000};nanosleep(&nap,0);}
 int background_reaped=stop(bg);uint64_t elapsed=now(CLOCK_MONOTONIC)-start;
 printf("LC_CLEANUP background=%d reaped=%d\n",bg>0,background_reaped);
 printf("LC_META requested=%s actual=%d errno=%d setup=%d mask=%"PRIu64" available=%"PRIu64" flags=%"PRIu64" runtime=%"PRIu64" deadline=%"PRIu64" period=%"PRIu64" quantum=%"PRIu64" kernel=%s\n",name,r->policy,r->denied,r->setup_error,r->mask,available,r->flags,r->q,r->d,r->p,r->quantum,uts.release);
 for(unsigned i=0;i<r->jobs;++i){Job *j=&r->job[i];printf("LC_JOB index=%u release=%"PRIu64" start=%"PRIu64" finish=%"PRIu64" cpu=%"PRIu64" checksum=%"PRIu64" missed=%u\n",i,j->release,j->start,j->finish,j->cpu,j->checksum,j->missed);}
 printf("LC_END jobs=%u overruns=%u supervisor_kill=%d elapsed=%"PRIu64" status=%d reaped=%d\n",r->jobs,r->overruns,killed,elapsed,status,collected==child);
 int code=1;
 if(collected==child&&!r->setup_error&&background_reaped){if(r->denied&&WIFEXITED(status)&&WEXITSTATUS(status)==77&&!killed){puts("LC_UNAVAILABLE:recorded-admission-refusal");code=77;}
 else if(!strcmp(behavior,"infinite")&&killed&&WIFSIGNALED(status)&&WTERMSIG(status)==SIGKILL&&r->policy==(int)policy){puts("LC_SUPERVISOR_PASS:owned-worker-terminated-and-reaped");code=0;}
 else if(strcmp(behavior,"infinite")&&!killed&&WIFEXITED(status)&&WEXITSTATUS(status)==0&&r->jobs==(!strcmp(behavior,"overrun")?4:JOBS)){puts("LC_GUEST_PASS:actual-policy-readback-workload");code=0;}}
 if(code==1)fputs("LC_GUEST_CHECK:unexpected-policy-or-worker-result\n",stderr);
 munmap(r,sizeof *r);return code;
}
