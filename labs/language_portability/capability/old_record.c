#include <stdio.h>
#include <limits.h>
struct fb_record {long value;unsigned present;};
int main(void) {
    struct fb_record r;int i;long values[3];
    values[0]=LONG_MIN;values[1]=0;values[2]=LONG_MAX;
    r.value=0;r.present=0;
    if(!r.present) puts("empty");
    for(i=0;i<3;i++){r.value=values[i];r.present=1;printf("%ld\n",r.value);}
    return 0;
}
