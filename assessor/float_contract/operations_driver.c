#define _POSIX_C_SOURCE 200809L
#include <float.h>
#include <math.h>
#include <stdio.h>
#include <stddef.h>
#include <time.h>
#include <fenv.h>
#ifdef __clang__
#pragma STDC FENV_ACCESS ON
#endif
double ff_expression(double,double,double),ff_separate(double,double,double),ff_zero(double);
double ff_missing(const double *,size_t),ff_kernel(const double *,size_t);
static volatile double sink;
int main(void) {
    if (fesetround(FE_TONEAREST)!=0) return 1;
    volatile double a=1+0x1p-27,b=1-0x1p-27,c=-1;
    double expr=ff_expression(a,b,c),sep=ff_separate(a,b,c);
    double missing[]={1,NAN,2},filtered[3];size_t used=0;
    for(size_t i=0;i<3;i++) if (isfinite(missing[i])) filtered[used++]=missing[i];
    double repaired=ff_kernel(filtered,used),raw=ff_missing(missing,3),zero=ff_zero(-0.0);
    double tri[]={1e16,1,-1e16};
    printf("OPERATIONS expression=%a separate=%a missing_nan=%d raw_missing=%a repaired=%a zero_sign=%d cancellation=%a eval=%d\n",expr,sep,!!isnan(raw),raw,repaired,!!signbit(zero),ff_kernel(tri,3),FLT_EVAL_METHOD);
    if (sep!=0 || repaired!=3) return 1;
    double finite[64],expected=0;
    for(size_t i=0;i<64;i++) {finite[i]=((int)i-32)*0x1p-10;expected+=finite[i];}
    if (ff_kernel(finite,64)!=expected) return 1;
    for (int trial=0;trial<5;trial++) {
        struct timespec start,end;
        if(clock_gettime(CLOCK_MONOTONIC,&start)!=0) return 1;
        for (int i=0;i<32768;i++) sink=ff_kernel(finite,64);
        if(clock_gettime(CLOCK_MONOTONIC,&end)!=0) return 1;
        long long ns=(long long)(end.tv_sec-start.tv_sec)*1000000000LL+end.tv_nsec-start.tv_nsec;
        if (sink!=expected || ns<0) return 1;
        printf("TIMING trial=%d nanoseconds=%lld calls=32768 count=64 exact=%a\n",trial,ns,expected);
    }
    puts("FF_OPERATIONS_PASS strict_caller finite_admission nan_filter_repair raw_timings");return 0;
}
