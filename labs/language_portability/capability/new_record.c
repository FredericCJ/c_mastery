#include "capability.h"
#include <limits.h>
#include <stdio.h>
int main(void) {
    struct lp_value r;char out[128];long values[3]={LONG_MIN,0,LONG_MAX};
    if(!lp_record(0,0,&r) || !lp_format(r,out,sizeof out))return 1;
    puts(out);
    for(unsigned i=0;i<3;i++) {
        if(!lp_record(1,(intmax_t)values[i],&r) || !lp_format(r,out,sizeof out))return 1;
        puts(out);
    }
    return 0;
}
