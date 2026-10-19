#include "performance.h"
#include <float.h>
#include <fenv.h>
#include <math.h>
#include <stdio.h>
int main(void){
 float a[3]={1.0e20f,-1.0e20f,3.0f},zero[1]={-0.0f},nan_input[1]={NAN};
 float sum=bf_reduce(a,3),z=bf_reduce(zero,1),nv=bf_reduce(nan_input,1);
 if(feclearexcept(FE_ALL_EXCEPT))return 2;
 volatile float biggest=FLT_MAX,two=2.0f;volatile float overflow=biggest*two;
 int flags=fetestexcept(FE_ALL_EXCEPT);
 printf("FLOAT sum=%.9g zero_sign=%d nan=%d overflow=%g flags=%d FE_OVERFLOW=%d\n",(double)sum,!!signbit(z),!!isnan(nv),(double)overflow,flags,FE_OVERFLOW);
 return 0;
}
