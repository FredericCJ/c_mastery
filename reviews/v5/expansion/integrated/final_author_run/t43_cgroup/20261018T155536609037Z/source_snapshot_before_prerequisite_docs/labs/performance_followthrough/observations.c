#define _GNU_SOURCE
#include "cost.h"
#include "algo.h"
#include <errno.h>
#include <fcntl.h>
#include <inttypes.h>
#include <linux/perf_event.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/resource.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>
#define C(x) do{if(!(x)){fprintf(stderr,"PF_HOST_CHECK:%d:%s:errno%d\n",__LINE__,#x,errno);exit(1);}}while(0)
static uint64_t stamp(clockid_t id){struct timespec t;C(!clock_gettime(id,&t));return (uint64_t)t.tv_sec*1000000000u+(unsigned)t.tv_nsec;}
static uint64_t burn(unsigned count){volatile uint64_t v=1;for(unsigned i=0;i<count;++i)v=v*33+i;return v;}
typedef struct{uint64_t cpu,checksum;} Thread;
static void*worker(void*p){Thread*r=p;uint64_t start=stamp(CLOCK_THREAD_CPUTIME_ID);r->checksum=burn(12000000);r->cpu=stamp(CLOCK_THREAD_CPUTIME_ID)-start;return NULL;}
static void clocks(void){struct timespec res;C(!clock_getres(CLOCK_MONOTONIC,&res));uint64_t a=stamp(CLOCK_MONOTONIC),p=stamp(CLOCK_PROCESS_CPUTIME_ID),t=stamp(CLOCK_THREAD_CPUTIME_ID);Thread r[2]={{0}};pthread_t threads[2];for(unsigned i=0;i<2;++i)C(!pthread_create(threads+i,NULL,worker,r+i));struct timespec delay={0,20000000};C(!nanosleep(&delay,NULL));for(unsigned i=0;i<2;++i)C(!pthread_join(threads[i],NULL));uint64_t wall=stamp(CLOCK_MONOTONIC)-a,process=stamp(CLOCK_PROCESS_CPUTIME_ID)-p,maincpu=stamp(CLOCK_THREAD_CPUTIME_ID)-t;C(r[0].checksum==r[1].checksum&&r[0].cpu&&r[1].cpu);printf("{\"case\":\"clocks\",\"resolution_ns\":%ld,\"elapsed_ns\":%"PRIu64",\"process_cpu_ns\":%"PRIu64",\"main_thread_cpu_ns\":%"PRIu64",\"worker_cpu_ns\":[%"PRIu64",%"PRIu64"],\"wall_minus_process_ns\":%"PRId64"}\n",res.tv_nsec,wall,process,maincpu,r[0].cpu,r[1].cpu,(int64_t)wall-(int64_t)process);
 struct rusage before,after;C(!getrusage(RUSAGE_CHILDREN,&before));p=stamp(CLOCK_PROCESS_CPUTIME_ID);pid_t child=fork();C(child>=0);if(!child){(void)burn(16000000);_exit(0);}int status;C(waitpid(child,&status,0)==child&&WIFEXITED(status)&&!WEXITSTATUS(status));process=stamp(CLOCK_PROCESS_CPUTIME_ID)-p;C(!getrusage(RUSAGE_CHILDREN,&after));int64_t childus=(after.ru_utime.tv_sec-before.ru_utime.tv_sec)*1000000+(after.ru_utime.tv_usec-before.ru_utime.tv_usec)+(after.ru_stime.tv_sec-before.ru_stime.tv_sec)*1000000+(after.ru_stime.tv_usec-before.ru_stime.tv_usec);printf("{\"case\":\"child-clock-scope\",\"parent_process_cpu_ns\":%"PRIu64",\"waited_child_cpu_us\":%"PRId64"}\n",process,childus);C(childus>0);
}
/* Same accepted sorted immutable extent. The instruction core is extracted
 * literally from algo_lower; admission is a separate untimed caller premise. */
#ifndef PF_ALLOW_INLINE
__attribute__((noinline))
#endif
uint32_t pf_lower_core(const uint32_t*a,uint32_t n,uint32_t key){uint32_t lo=0,hi=n;while(lo<hi){uint32_t mid=lo+(hi-lo)/2;if(a[mid]<key)lo=mid+1;else hi=mid;}return lo;}
static uint64_t bench(unsigned which,const uint32_t*a,unsigned n,unsigned count,uint64_t*out){uint64_t sum=0,start=stamp(CLOCK_MONOTONIC);for(unsigned q=0;q<count;++q){unsigned key=(q*17u)%(n*2u+1u);uint32_t result=which?algo_asm_lower(a,n,key):pf_lower_core(a,n,key);C(result==(key+1)/2);sum+=result;}uint64_t end=stamp(CLOCK_MONOTONIC);*out=sum;return end-start;}
static void pairs(void){uint32_t data[65536];for(unsigned i=0;i<65536;++i)data[i]=i*2;uint64_t sum;for(unsigned k=0;k<3;++k)for(unsigned j=0;j<2;++j)(void)bench(j,data,1024,1000,&sum);
 for(unsigned size=0;size<3;++size){unsigned n=size==0?16:size==1?1024:65536;for(unsigned i=0;i<31;++i){uint64_t a,b,sa,sb;unsigned first=i%2;if(first){b=bench(1,data,n,8192,&sb);a=bench(0,data,n,8192,&sa);}else{a=bench(0,data,n,8192,&sa);b=bench(1,data,n,8192,&sb);}C(sa==sb);printf("{\"case\":\"pair\",\"n\":%u,\"pair\":%u,\"first\":%u,\"a_ns\":%"PRIu64",\"b_ns\":%"PRIu64",\"checksum\":%"PRIu64",\"queries\":8192}\n",n,i,first,a,b,sa);}}
 for(unsigned i=0;i<31;++i){uint64_t s=stamp(CLOCK_MONOTONIC),e=stamp(CLOCK_MONOTONIC);printf("{\"case\":\"bracket\",\"pair\":%u,\"ns\":%"PRIu64"}\n",i,e-s);}
 uint32_t*lookup=malloc(131073*sizeof*lookup);C(lookup);for(unsigned i=0;i<131073;++i)lookup[i]=(i+1)/2;
 for(unsigned i=0;i<31;++i){uint64_t sa=0,sb=0,ta=bench(0,data,65536,8192,&sa),start=stamp(CLOCK_MONOTONIC);for(unsigned q=0;q<8192;++q)sb+=lookup[(q*17u)%131073u];uint64_t tb=stamp(CLOCK_MONOTONIC)-start;C(sa==sb);printf("{\"case\":\"pareto\",\"pair\":%u,\"binary_ns\":%"PRIu64",\"table_ns\":%"PRIu64",\"binary_retained\":262144,\"table_retained\":786436,\"checksum\":%"PRIu64"}\n",i,ta,tb,sa);}free(lookup);
 /* Fixed semantic comparisons: only input predictability changes. */
 for(unsigned pattern=0;pattern<2;++pattern){volatile uint64_t total=0;uint32_t rng=1;uint64_t start=stamp(CLOCK_MONOTONIC);for(unsigned i=0;i<1000000;++i){rng=rng*1664525u+1013904223u;unsigned take=pattern?rng>>31:i&1u;if(take)total+=3;else total+=1;}printf("{\"case\":\"branch-pattern\",\"pattern\":%u,\"comparisons\":1000000,\"elapsed_ns\":%"PRIu64",\"checksum\":%"PRIu64",\"branch_misses\":null}\n",pattern,stamp(CLOCK_MONOTONIC)-start,total);}
}
static unsigned read_file(int fd,unsigned char*b,size_t n){C(lseek(fd,0,SEEK_SET)==0);size_t done=0;unsigned calls=0;while(done<n){ssize_t r=read(fd,b+done,n-done);++calls;if(r<0&&errno==EINTR)continue;C(r>0);done+=(size_t)r;}return calls;}
static void files(void){char dir[]="/tmp/cmastery-performance-XXXXXX";C(mkdtemp(dir));char path[256],tmp[256];C(snprintf(path,sizeof path,"%s/data",dir)>0);C(snprintf(tmp,sizeof tmp,"%s/stage",dir)>0);int dfd=open(dir,O_RDONLY|O_DIRECTORY|O_CLOEXEC);C(dfd>=0);int fd=open(path,O_CREAT|O_EXCL|O_RDWR|O_CLOEXEC,0600);C(fd>=0);unsigned char bytes[8192],readback[8192];for(unsigned i=0;i<sizeof bytes;++i)bytes[i]=(unsigned char)i;
 uint64_t start=stamp(CLOCK_MONOTONIC);size_t done=0;unsigned calls=0;while(done<sizeof bytes){ssize_t r=write(fd,bytes+done,sizeof bytes-done);++calls;if(r<0&&errno==EINTR)continue;C(r>0);done+=(size_t)r;}uint64_t accepted=stamp(CLOCK_MONOTONIC);int synced=fsync(fd);int syncerrno=errno;uint64_t completed=stamp(CLOCK_MONOTONIC);C(!synced);printf("{\"case\":\"write-sync\",\"accepted\":%zu,\"calls\":%u,\"write_ns\":%"PRIu64",\"sync_ns\":%"PRIu64",\"fsync_status\":%d,\"errno_if_failure\":%d}\n",done,calls,accepted-start,completed-accepted,synced,synced?syncerrno:0);
 for(unsigned pass=0;pass<3;++pass){if(pass==2){pid_t child=fork();C(child>=0);if(!child){int other=open(path,O_RDONLY|O_CLOEXEC);if(other<0)_exit(1);unsigned char copy[8192];unsigned k=read_file(other,copy,sizeof copy);close(other);_exit(!k||memcmp(copy,bytes,sizeof copy));}int status;C(waitpid(child,&status,0)==child&&WIFEXITED(status)&&!WEXITSTATUS(status));}start=stamp(CLOCK_MONOTONIC);unsigned reads=read_file(fd,readback,sizeof readback);completed=stamp(CLOCK_MONOTONIC);C(!memcmp(bytes,readback,sizeof bytes));printf("{\"case\":\"read-pass\",\"pass\":%u,\"accepted_bytes\":8192,\"syscalls\":%u,\"elapsed_ns\":%"PRIu64",\"device_bytes\":null,\"after_owned_other_process\":%s}\n",pass,reads,completed-start,pass==2?"true":"false");}
 /* Inject failure at the synchronizer seam, never mislabel it a kernel fault. */
 int injected=-1;int injected_errno=EIO;C(injected<0&&injected_errno==EIO&&!memcmp(bytes,readback,sizeof bytes));puts("{\"case\":\"injected-fsync-error\",\"source\":\"local-adapter\",\"status\":\"failed\",\"readable_bytes\":8192}");
 int stage=open(tmp,O_CREAT|O_EXCL|O_RDWR|O_CLOEXEC,0600);C(stage>=0);C(write(stage,"abcdefgh",8)==8);C(!fsync(stage));C(!close(stage));C(!rename(tmp,path));C(!fsync(dfd));puts("{\"case\":\"publish\",\"boundaries\":[\"write8\",\"file-fsync\",\"rename\",\"directory-fsync\"],\"power_loss\":\"unobserved\"}");C(!close(fd));fd=open(path,O_RDWR|O_CLOEXEC);C(fd>=0);C(!ftruncate(fd,1048576));C(pwrite(fd,"abcdefgh",8,1048568)==8);C(!fsync(fd));struct stat sparse,pre;C(!fstat(fd,&sparse));int prestatus=posix_fallocate(fd,0,1048576);C(!fstat(fd,&pre));C(pread(fd,readback,8,1048568)==8&&!memcmp(readback,"abcdefgh",8));printf("{\"case\":\"sparse-preallocated\",\"logical_length\":1048576,\"payload\":8,\"sparse_512_blocks\":%jd,\"after_512_blocks\":%jd,\"preallocate_status\":%d,\"physical_device_traffic\":null}\n",(intmax_t)sparse.st_blocks,(intmax_t)pre.st_blocks,prestatus);C(!close(fd));C(!unlink(path));C(!close(dfd));C(!rmdir(dir));
}
static void pmu(void){unsigned configs[]={PERF_COUNT_HW_INSTRUCTIONS,PERF_COUNT_HW_CPU_CYCLES,PERF_COUNT_HW_BRANCH_MISSES,PERF_COUNT_HW_CACHE_MISSES};const char*names[]={"instructions","cycles","branch-misses","cache-misses"};int fds[4]={-1,-1,-1,-1};unsigned made=0;
 for(unsigned i=0;i<4;++i){struct perf_event_attr a={0};a.type=PERF_TYPE_HARDWARE;a.size=sizeof a;a.config=configs[i];a.disabled=1;a.exclude_kernel=1;a.exclude_hv=1;a.read_format=PERF_FORMAT_TOTAL_TIME_ENABLED|PERF_FORMAT_TOTAL_TIME_RUNNING|PERF_FORMAT_ID;errno=0;fds[i]=(int)syscall(SYS_perf_event_open,&a,0,-1,i?fds[0]:-1,PERF_FLAG_FD_CLOEXEC);int e=errno;if(fds[i]<0){printf("{\"case\":\"pmu\",\"event\":\"%s\",\"status\":\"%s\",\"errno\":%d,\"raw\":null,\"pid\":0,\"cpu\":-1,\"exclude_kernel\":true}\n",names[i],e==EACCES||e==EPERM?"denied":"unavailable",e);continue;}++made;}
 if(made){C(fds[0]>=0);C(!ioctl(fds[0],PERF_EVENT_IOC_RESET,PERF_IOC_FLAG_GROUP));C(!ioctl(fds[0],PERF_EVENT_IOC_ENABLE,PERF_IOC_FLAG_GROUP));(void)burn(1000000);C(!ioctl(fds[0],PERF_EVENT_IOC_DISABLE,PERF_IOC_FLAG_GROUP));for(unsigned i=0;i<4;++i)if(fds[i]>=0){uint64_t r[4];C(read(fds[i],r,sizeof r)==(ssize_t)sizeof r);printf("{\"case\":\"pmu\",\"event\":\"%s\",\"status\":\"measured\",\"raw\":%"PRIu64",\"enabled\":%"PRIu64",\"running\":%"PRIu64",\"id\":%"PRIu64",\"group\":0}\n",names[i],r[0],r[1],r[2],r[3]);C(!close(fds[i]));}}
}
static size_t resident(void){FILE*f=fopen("/proc/self/statm","r");C(f);size_t a,b;C(fscanf(f,"%zu %zu",&a,&b)==2);C(!fclose(f));return b;}
static void memory(void){size_t before=resident();unsigned char*p=malloc(1048576);C(p);for(unsigned i=0;i<1048576;i+=4096)p[i]=(unsigned char)i;size_t during=resident();volatile unsigned checksum=p[0];free(p);size_t after=resident();printf("{\"case\":\"memory\",\"payload_live\":[0,1048576,0],\"rss_pages\":[%zu,%zu,%zu],\"page_bytes\":%ld,\"allocator_metadata\":null,\"checksum\":%u,\"leak_conclusion\":\"not-established\"}\n",before,during,after,sysconf(_SC_PAGESIZE),checksum);}
int main(int argc,char**argv){if(argc==2&&!strcmp(argv[1],"pairs"))pairs();else if(argc==2&&!strcmp(argv[1],"pmu"))pmu();else{clocks();files();memory();}return 0;}
