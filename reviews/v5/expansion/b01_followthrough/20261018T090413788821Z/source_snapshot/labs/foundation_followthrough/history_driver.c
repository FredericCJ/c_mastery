#include "depth.h"
#include <stdio.h>
static int history(unsigned steps) {
    FDPool pool,other;FDHandle handles[2];char action;
    fd_pool_init(&pool);fd_pool_init(&other);
    handles[0]=handles[1]=(FDHandle){&pool,SIZE_MAX,0};
    puts("R");
    for(unsigned step=0;step<steps;++step) {
        if(scanf(" %c",&action)!=1)return 2;
        unsigned r=0;if(scanf("%u",&r)!=1 || r>1)return 2;
        if(action=='A') {
            int value;if(scanf("%d",&value)!=1)return 2;
            FDStatus s=fd_pool_acquire(&pool,(FDValue){value,(uint32_t)value+100},&handles[r]);
            printf("A %d %zu %u\n",s,handles[r].index,handles[r].generation);
        } else if(action=='L')printf("L %d\n",fd_pool_release(&pool,handles[r]));
        else if(action=='G' || action=='F' || action=='I') {
            FDValue out={917,919};FDHandle h=handles[r];if(action=='I')h.index=SIZE_MAX;
            FDStatus s=fd_pool_get(action=='F'?&other:&pool,h,&out);
            printf("%c %d %d %u\n",action,s,(int)out.value,(unsigned)out.serial);
        } else return 2;
    }
    return 0;
}
int main(void) {
    unsigned steps;
    printf("SIZE_MAX %zu\n",SIZE_MAX);
    /* Each complete function invocation owns a new pool lifetime; no handle
     * escapes. No production owner is reinitialized while it remains live. */
    while(scanf("%u",&steps)==1)if(history(steps))return 2;
    return 0;
}
