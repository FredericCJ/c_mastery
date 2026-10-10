/* Bounded, process-local Linux scheduler observation. This supplied measurement
   tool is not the learner's original scheduler. Never changes host global knobs. */
#define _GNU_SOURCE
#include <errno.h>
#include <inttypes.h>
#include <sched.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/resource.h>
#include <sys/syscall.h>
#include <time.h>
#include <unistd.h>
struct cm_sched_attr {
    uint32_t size,policy; uint64_t flags; int32_t nice; uint32_t priority;
    uint64_t runtime,deadline,period;
};
static uint64_t ns(clockid_t clock) {
    struct timespec t; if(clock_gettime(clock,&t)) exit(2);
    return (uint64_t)t.tv_sec*UINT64_C(1000000000)+(uint64_t)t.tv_nsec;
}
int main(int argc,char **argv) {
    if(argc!=2 || (strcmp(argv[1],"other") && strcmp(argv[1],"fifo") && strcmp(argv[1],"rr") && strcmp(argv[1],"deadline"))) return 2;
    /* Kernel CPU-time limits and a wall alarm bound admitted RT experiments. */
    struct rlimit rt={200000,300000};
    if(setrlimit(RLIMIT_RTTIME,&rt)) { perror("setrlimit"); return 77; }
    alarm(2);
    int result; struct sched_param parameter={.sched_priority=1};
    if(!strcmp(argv[1],"deadline")) {
        struct cm_sched_attr attr={.size=sizeof attr,.policy=6,
            .runtime=1000000,.deadline=10000000,.period=10000000};
        result=(int)syscall(SYS_sched_setattr,0,&attr,0);
    } else {
        int policy=!strcmp(argv[1],"fifo")?SCHED_FIFO:!strcmp(argv[1],"rr")?SCHED_RR:SCHED_OTHER;
        if(policy==SCHED_OTHER) parameter.sched_priority=0;
        result=sched_setscheduler(0,policy,&parameter);
    }
    if(result) {
        printf("{\"status\":\"unavailable\",\"policy\":\"%s\",\"errno\":%d}\n",argv[1],errno); return 77;
    }
    struct rusage before,after;
    if(getrusage(RUSAGE_SELF,&before)) return 2;
    uint64_t start=ns(CLOCK_MONOTONIC),cpu=ns(CLOCK_PROCESS_CPUTIME_ID),state=1,rounds=0;
    while(ns(CLOCK_MONOTONIC)-start<UINT64_C(100000000)) {
        for(unsigned i=0;i<1000;++i) state=state*UINT64_C(6364136223846793005)+1;
        ++rounds;
    }
    uint64_t cpu_delta=ns(CLOCK_PROCESS_CPUTIME_ID)-cpu,wall_delta=ns(CLOCK_MONOTONIC)-start;
    if(getrusage(RUSAGE_SELF,&after)) return 2;
    printf("{\"status\":\"observed\",\"policy\":\"%s\",\"actual_policy\":%d,\"wall_ns\":%" PRIu64
           ",\"cpu_ns\":%" PRIu64 ",\"rounds\":%" PRIu64 ",\"checksum\":%" PRIu64
           ",\"involuntary_switches\":%ld,\"deadline_guarantee\":false}\n",argv[1],sched_getscheduler(0),wall_delta,cpu_delta,rounds,state,after.ru_nivcsw-before.ru_nivcsw);
    return 0;
}
