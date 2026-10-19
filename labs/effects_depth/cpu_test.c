#define _POSIX_C_SOURCE 200809L
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/resource.h>
static void*work(void*result){volatile uint32_t sum=0;for(uint32_t i=0;i<20000000;++i)sum+=i;*(uint32_t*)result=sum;return NULL;}
static double sec(struct timespec t){return(double)t.tv_sec+(double)t.tv_nsec/1e9;}
int main(int argc,char**argv){if(argc!=2)return 2;struct timespec a,b,c,d,res;struct rusage ra,rb;uint32_t sums[2]={0};clock_getres(CLOCK_MONOTONIC,&res);clock_gettime(CLOCK_MONOTONIC,&a);clock_gettime(CLOCK_PROCESS_CPUTIME_ID,&c);getrusage(RUSAGE_SELF,&ra);
 if(!strcmp(argv[1],"cpu"))work(sums);else if(!strcmp(argv[1],"wait")){struct timespec delay={0,20000000};while(nanosleep(&delay,&delay)){};}else if(!strcmp(argv[1],"threads")){pthread_t x,y;if(pthread_create(&x,NULL,work,&sums[0])||pthread_create(&y,NULL,work,&sums[1]))return 3;if(pthread_join(x,NULL)||pthread_join(y,NULL))return 4;}else if(strcmp(argv[1],"observer"))return 2;
 getrusage(RUSAGE_SELF,&rb);clock_gettime(CLOCK_PROCESS_CPUTIME_ID,&d);clock_gettime(CLOCK_MONOTONIC,&b);printf("{\"mode\":\"%s\",\"wall_seconds\":%.9f,\"process_cpu_seconds\":%.9f,\"user_us\":%ld,\"system_us\":%ld,\"resolution_ns\":%ld,\"checksum\":%u}\n",argv[1],sec(b)-sec(a),sec(d)-sec(c),(long)((rb.ru_utime.tv_sec-ra.ru_utime.tv_sec)*1000000+rb.ru_utime.tv_usec-ra.ru_utime.tv_usec),(long)((rb.ru_stime.tv_sec-ra.ru_stime.tv_sec)*1000000+rb.ru_stime.tv_usec-ra.ru_stime.tv_usec),res.tv_nsec,sums[0]);return 0;}
