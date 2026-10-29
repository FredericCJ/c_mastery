/* Supplied native observation adapter, not a learner solution.
 * Linux ABI/GCC or Clang builtins are explicit dependencies. */
#define _GNU_SOURCE
#include "pmu.h"
#include "pmu_integrations.h"
#include "algo.h"
#include <errno.h>
#include <inttypes.h>
#include <linux/perf_event.h>
#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/prctl.h>
#include <sys/syscall.h>
#include <time.h>
#include <unistd.h>

typedef struct {uint64_t value;unsigned char padding[56];} Wide;
typedef struct {
    size_t n; uint64_t *values; Wide *wide; algo_pair *records;
    unsigned family,variant; uint64_t (*dispatch[2])(uint64_t);
} Work;
static volatile uint64_t observable;
static uint64_t clock_ns(void){
    struct timespec t;
    if(clock_gettime(CLOCK_MONOTONIC_RAW,&t)){perror("clock_gettime");exit(2);}
    return (uint64_t)t.tv_sec*1000000000u+(uint64_t)t.tv_nsec;
}
__attribute__((noinline)) static uint64_t operation0(uint64_t v){return v*33+17;}
__attribute__((noinline)) static uint64_t operation1(uint64_t v){return v*33+19;}
static uint64_t execute(Work *w,unsigned repetitions){
    if(w->family==6)return pf_concurrency_work(w->n,w->variant,repetitions);
    if(w->family==7)return pf_service_work(w->n,w->variant,repetitions);
    uint64_t sum=0;
    for(unsigned repeat=0;repeat<repetitions;++repeat){
        if(w->family==0){ /* same sorted immutable lower-bound contract */
            for(unsigned q=0;q<256;++q){
                uint64_t key=((uint64_t)q*2654435761u+repeat)%(2*w->n+1);
                size_t i=0;
                if(w->variant){size_t hi=w->n;while(i<hi){size_t mid=i+(hi-i)/2;if(w->values[mid]<key)i=mid+1;else hi=mid;}}
                else while(i<w->n&&w->values[i]<key)++i;
                sum+=i;
            }
        }else if(w->family==1){
            for(size_t i=0;i<w->n;++i)sum+=w->variant?w->wide[i].value:w->values[i];
        }else if(w->family==2){
            if(w->variant){uint64_t a=0,b=0,c=0,d=0;size_t i=0;
                for(;i+3<w->n;i+=4){a+=w->values[i];b+=w->values[i+1];c+=w->values[i+2];d+=w->values[i+3];}
                sum+=a+b+c+d;for(;i<w->n;++i)sum+=w->values[i];
            }else for(size_t i=0;i<w->n;++i)sum+=w->values[i];
        }else if(w->family==3){
            for(size_t i=0;i<w->n;++i){uint64_t v=w->values[i]+repeat;
                sum+=w->variant?w->dispatch[v&1u](v):((v&1u)?operation1(v):operation0(v));}
        }else if(w->family==4){
            /* Same allocation lifetime and values; representation changes. */
            if(w->variant){uint64_t *block=malloc(w->n*sizeof *block);if(!block)return UINT64_MAX;
                for(size_t i=0;i<w->n;++i)block[i]=w->values[i]+repeat;
                for(size_t i=0;i<w->n;++i)sum+=block[i];
                free(block);
            }else{uint64_t **items=calloc(w->n,sizeof *items);if(!items)return UINT64_MAX;size_t i;
                for(i=0;i<w->n;++i){items[i]=malloc(sizeof **items);if(!items[i])break;*items[i]=w->values[i]+repeat;}
                if(i!=w->n){for(size_t j=0;j<i;++j)free(items[j]);free(items);return UINT64_MAX;}
                for(i=0;i<w->n;++i){sum+=*items[i];free(items[i]);}free(items);
            }
        }else{ /* existing retained stable sorting interfaces, n <=1024 */
            for(size_t i=0;i<w->n;++i)w->records[i]=(algo_pair){(uint32_t)((w->n-i)%17),(uint32_t)i};
            int rc=w->variant?algo_merge(w->records,(uint32_t)w->n,(uint32_t)w->n,NULL):algo_insertion(w->records,(uint32_t)w->n,(uint32_t)w->n,NULL);
            if(rc)return UINT64_MAX;
            for(size_t i=0;i<w->n;++i)sum+=(i+1)*(w->records[i].key+31u*w->records[i].tag);
        }
    }
    return sum;
}
static uint64_t oracle(const Work *w,unsigned repetitions){
    uint64_t n=w->n,total=0;
    for(unsigned r=0;r<repetitions;++r){
        if(w->family==0){for(unsigned q=0;q<256;++q){uint64_t key=((uint64_t)q*2654435761u+r)%(2*n+1);total+=(key+1)/2;}}
        else if(w->family==1||w->family==2)total+=n*(n-1);
        else if(w->family==3)total+=33*(n*(n-1)+n*r)+(17+2*(r&1u))*n;
        else if(w->family==4)total+=n*(n-1)+n*r;
        else if(w->family==5){size_t position=0;for(unsigned key=0;key<17;++key)for(size_t tag=0;tag<n;++tag)
            if((n-tag)%17==key)total+=(++position)*(key+31u*tag);}
        else if(w->family==6)total+=n*2;
        else for(size_t i=0;i<n;++i)if(i%5)total+=18+i;
    }
    return total;
}
static void emit_sample(void *unused,const PFSample *s){
    (void)unused;
    printf("{\"kind\":\"sample\",\"ip\":%"PRIu64",\"pid\":%u,\"tid\":%u,\"time\":%"PRIu64",\"period\":%"PRIu64"}\n",s->ip,s->pid,s->tid,s->time,s->period);
}
static void mapping_identity(void){
    FILE *maps=fopen("/proc/self/maps","r");char line[1024];
    if(!maps)return;
    while(fgets(line,sizeof line,maps)){
        fputs("{\"kind\":\"mapping\",\"line\":\"",stdout);
        for(const char *p=line;*p;++p){
            if(*p=='\n')fputs("\\n",stdout);
            else if(*p=='\\'||*p=='\"'){putchar('\\');putchar(*p);}
            else if((unsigned char)*p>=32)putchar(*p);
        }
        puts("\"}");
    }
    fclose(maps);
}
static int sampled(Work *w,unsigned precision){
    struct perf_event_attr a={0};
    a.size=sizeof a;a.type=PERF_TYPE_HARDWARE;a.config=PERF_COUNT_HW_CPU_CYCLES;
    a.disabled=1;a.exclude_kernel=1;a.exclude_hv=1;a.sample_period=100000;
    a.sample_type=PERF_SAMPLE_IP|PERF_SAMPLE_TID|PERF_SAMPLE_TIME|PERF_SAMPLE_PERIOD;
    a.wakeup_events=1;
    a.precise_ip=precision;
    a.read_format=PERF_FORMAT_TOTAL_TIME_ENABLED|PERF_FORMAT_TOTAL_TIME_RUNNING;
    mapping_identity();
    int fd=(int)syscall(SYS_perf_event_open,&a,0,-1,-1,PERF_FLAG_FD_CLOEXEC);
    if(fd<0){printf("{\"kind\":\"sampling\",\"status\":\"%s\",\"errno\":%d}\n",errno==EACCES||errno==EPERM?"denied":"unavailable",errno);return 0;}
    long page=sysconf(_SC_PAGESIZE);
    if(page<=0||((size_t)page&(size_t)(page-1))){close(fd);return 2;}
    size_t length=(size_t)page*9;
    void *mapping=mmap(NULL,length,PROT_READ|PROT_WRITE,MAP_SHARED,fd,0);
    if(mapping==MAP_FAILED){int e=errno;close(fd);printf("{\"kind\":\"sampling\",\"status\":\"map-failed\",\"errno\":%d}\n",e);return 0;}
    struct perf_event_mmap_page *meta=mapping;
    uint64_t offset=meta->data_offset,size=meta->data_size;
    int rc=0,error=0;
    PFRingStats stats={0};uint64_t tail=0;
    if(offset>length||size>length-offset||!size||(size&(size-1)))rc=PF_RING_INVALID;
    if(!rc&&(ioctl(fd,PERF_EVENT_IOC_RESET,0)||ioctl(fd,PERF_EVENT_IOC_ENABLE,0))){rc=PF_RING_INVALID;error=errno;}
    /* Print only after disable: serialization is outside measured workload. */
    if(!rc)observable^=execute(w,256);
    if(ioctl(fd,PERF_EVENT_IOC_DISABLE,0)){rc=PF_RING_INVALID;error=errno;}
    uint64_t counts[3]={0};ssize_t got=-1;
    for(unsigned tries=0;tries<8;++tries){got=read(fd,counts,sizeof counts);if(got>=0||errno!=EINTR)break;}
    if(!rc){
        uint64_t head=__atomic_load_n(&meta->data_head,__ATOMIC_ACQUIRE);
        rc=pf_ring_drain((unsigned char *)mapping+offset,(size_t)size,head,&tail,4096,&stats,emit_sample,NULL);
        /* Linux perf ABI: all record reads precede publication of data_tail. */
        __atomic_thread_fence(__ATOMIC_SEQ_CST);
        __atomic_store_n(&meta->data_tail,tail,__ATOMIC_RELEASE);
        if(tail!=head&&!rc)rc=PF_RING_PARTIAL;
    }
    int cleanup=0;if(munmap(mapping,length))cleanup=errno;if(close(fd)&&!cleanup)cleanup=errno;
    const char *status=rc||got!=(ssize_t)sizeof counts||counts[2]>counts[1]?"invalid":
                       !counts[2]?"never-scheduled":!stats.samples?"no-samples":"measured";
    printf("{\"kind\":\"sampling\",\"status\":\"%s\",\"errno\":%d,\"cleanup_errno\":%d,\"parser_status\":%d,\"samples\":%"PRIu64",\"lost\":%"PRIu64",\"unknown\":%"PRIu64",\"enabled\":%"PRIu64",\"running\":%"PRIu64",\"precise_ip\":%u,\"scope\":\"calling-thread-user\"}\n",status,error,cleanup,rc,stats.samples,stats.lost,stats.unknown,counts[1],counts[2],precision);
    return rc?2:0;
}
int main(int argc,char **argv){
    const char *families[]={"search","layout","numeric","dispatch","allocation","sort","concurrency","service"};
    if(argc<5){fprintf(stderr,"usage: %s count|work|sample FAMILY N VARIANT [REPETITIONS]\n",argv[0]);return 2;}
    Work w={0};for(;w.family<8&&strcmp(argv[2],families[w.family]);++w.family){}
    char *end;unsigned long n=strtoul(argv[3],&end,10);if(*end||!n||n>4194304||w.family==8)return 2;
    w.n=(size_t)n;w.variant=(unsigned)strtoul(argv[4],&end,10);if(*end||w.variant>1)return 2;
    unsigned repetitions=argc>5?(unsigned)strtoul(argv[5],&end,10):16;
    if(!repetitions||repetitions>1024||*end)return 2;
    if((w.family==4&&n>65536)||((w.family==5||w.family==7)&&n>1024))return 2;
    w.values=malloc(w.n*sizeof *w.values);w.wide=w.family==1?malloc(w.n*sizeof *w.wide):NULL;
    w.records=w.family==5?malloc(w.n*sizeof *w.records):NULL;
    if(!w.values||(w.family==1&&!w.wide)||(w.family==5&&!w.records)){free(w.values);free(w.wide);free(w.records);return 2;}
    for(size_t i=0;i<w.n;++i){w.values[i]=i*2;if(w.wide)w.wide[i].value=i*2;}
    w.dispatch[0]=operation0;w.dispatch[1]=operation1;
    /* Independent arithmetic oracle for sums/search; full stable-sort order
     * validation in retained tests, plus checksum parity outside interval. */
    unsigned saved=w.variant;w.variant=0;uint64_t expected=execute(&w,repetitions);w.variant=1;
    uint64_t alternate=execute(&w,repetitions);w.variant=saved;
    if(expected==UINT64_MAX||expected!=alternate||expected!=oracle(&w,repetitions)){fputs("PF_NATIVE_ASSERT:equivalent results and independent oracle\n",stderr);return 1;}
    if(w.family==1||w.family==2){uint64_t independent=(uint64_t)w.n*(w.n-1)*repetitions;if(expected!=independent)return 1;}
    if(w.family==6&&expected!=(uint64_t)n*repetitions*2)return 1;
    if(w.family==7){uint64_t independent=0;for(size_t i=0;i<n;++i)if(i%5)independent+=18+i;if(expected!=independent*repetitions)return 1;}
    observable^=execute(&w,1);
    int rc=0;
    if(!strcmp(argv[1],"sample")){
        unsigned precision=argc>6?(unsigned)strtoul(argv[6],&end,10):0;
        if(precision>3||*end)return 2;
        rc=sampled(&w,precision);
    }
    else{
        PFPmu group={0};PFPmuState state=PF_PMU_EMPTY;
        uint64_t brackets[31],overhead=0;struct timespec resolution;
        if(clock_getres(CLOCK_MONOTONIC_RAW,&resolution))return 2;
        uint64_t resolution_ns=(uint64_t)resolution.tv_sec*1000000000u+(uint64_t)resolution.tv_nsec;
        for(unsigned i=0;i<31;++i){uint64_t begin=clock_ns();brackets[i]=clock_ns()-begin;if(brackets[i]>overhead)overhead=brackets[i];}
        int count=!strcmp(argv[1],"count");
        if(!count&&strcmp(argv[1],"work"))return 2;
        const PFPmuEvent events[]={{PERF_TYPE_HARDWARE,PERF_COUNT_HW_INSTRUCTIONS},{PERF_TYPE_HARDWARE,PERF_COUNT_HW_CPU_CYCLES}};
        if(count){state=pf_pmu_open(&group,pf_pmu_linux(),events,2);if(state==PF_PMU_READY)state=pf_pmu_start(&group);}
        else if(prctl(PR_TASK_PERF_EVENTS_ENABLE))return 2;
        int before=sched_getcpu();uint64_t start=clock_ns(),checksum=execute(&w,repetitions),elapsed=clock_ns()-start;int after=sched_getcpu();
        if(count&&state==PF_PMU_ACTIVE)state=pf_pmu_stop(&group);
        else if(!count&&prctl(PR_TASK_PERF_EVENTS_DISABLE))return 2;
        if(checksum!=expected){fputs("PF_NATIVE_ASSERT:workload checksum\n",stderr);rc=1;}
        observable^=checksum;
        printf("{\"kind\":\"count\",\"family\":\"%s\",\"n\":%zu,\"variant\":%u,\"repetitions\":%u,\"checksum\":%"PRIu64",\"elapsed_ns\":%"PRIu64",\"cpu_before\":%d,\"cpu_after\":%d,\"status\":\"%s\",\"errno\":%d,\"cleanup_errno\":%d,\"enabled\":%"PRIu64",\"running\":%"PRIu64",\"raw_instructions\":%"PRIu64",\"raw_cycles\":%"PRIu64",\"instruction_id\":%"PRIu64",\"cycle_id\":%"PRIu64",\"scope\":\"calling-thread-user\",\"strict_group\":true,\"clock_resolution_ns\":%"PRIu64",\"clock_overhead_max_ns\":%"PRIu64",\"clock_brackets_ns\":[",families[w.family],w.n,w.variant,repetitions,checksum,elapsed,before,after,pf_pmu_state_name(state),group.error,group.cleanup_error,group.enabled,group.running,group.raw[0],group.raw[1],group.ids[0],group.ids[1],resolution_ns,overhead);
        for(unsigned i=0;i<31;++i)printf("%s%"PRIu64,i?",":"",brackets[i]);
        puts("]}");
        pf_pmu_close(&group);
    }
    free(w.values);free(w.wide);free(w.records);return rc;
}
