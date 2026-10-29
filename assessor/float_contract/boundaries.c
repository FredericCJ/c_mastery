#include "float_contract.h"
#include <float.h>
#include <math.h>
#include <stdio.h>
int main(void) {
    fenv_t saved;if(fegetenv(&saved)!=0 || fesetround(FE_TONEAREST)!=0) return 2;
    double seeds[]={0,DBL_MIN,0x1p-100,1,0x1p100,DBL_MAX};unsigned pairs=0;
    for(size_t i=0;i<sizeof seeds/sizeof seeds[0];i++) {
        for(int sign=-1;sign<=1;sign+=2) {
            double x=sign*seeds[i];
            for(int direction=-1;direction<=1;direction+=2) {
                double y=nextafter(x,direction<0?-INFINITY:INFINITY);uint64_t distance=917;
                enum ff_status rc=ff_ulps(x,y,&distance);
                if(isfinite(y)) {if(rc!=FF_OK || distance!=1) return 1;}
                else if(rc!=FF_RANGE || distance!=917) return 1;
                printf("BOUNDARY partition=%zu sign=%d direction=%d x=%a y=%a finite=%d\n",i,sign,direction,x,y,!!isfinite(y));pairs++;
            }
        }
    }
    float fseeds[]={0,FLT_MIN,1,FLT_MAX};unsigned fpairs=0;
    for(size_t i=0;i<4;i++) for(int direction=-1;direction<=1;direction+=2) {
        float x=fseeds[i],y=nextafterf(x,direction<0?-INFINITY:INFINITY);
        printf("FLOAT_BOUNDARY partition=%zu direction=%d x=%a y=%a precision=%d\n",i,direction,(double)x,(double)y,FLT_MANT_DIG);fpairs++;
        if(x==y) return 1;
    }
    if(fesetenv(&saved)!=0) return 2;
    printf("FF_BOUNDARY_PASS double_pairs=%u float_pairs=%u algorithmic_neighbors\n",pairs,fpairs);return 0;
}
