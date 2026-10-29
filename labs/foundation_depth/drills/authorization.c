/* Original synthetic teaching target: no files, sockets, credentials or privilege. */
#include "depth.h"
#include <stdio.h>

static enum fd_status vulnerable(const unsigned char *wire,size_t n,int allowed,int32_t *counter) {
    struct fd_record r;
    enum fd_status status=fd_decode(wire,n,&r);
    if (status!=FD_OK) return status;
    int64_t next=(int64_t)*counter+(int64_t)r.count*r.value;
    if (next<INT32_MIN || next>INT32_MAX) return FD_RANGE;
    *counter=(int32_t)next; /* defect: publication before trusted authorization */
    if (!allowed) return FD_DENIED;
    return FD_OK;
}
int main(void) {
    const unsigned char attacker_input[8]={1,0,0,2,0,0,0,3};
    int32_t flawed=10,repaired=10;
    enum fd_status bad=vulnerable(attacker_input,8,0,&flawed);
    enum fd_status good=fd_apply(attacker_input,8,0,&repaired);
    printf("local unauthorized request: flawed status=%d counter=%ld; repaired status=%d counter=%ld\n",
           bad,(long)flawed,good,(long)repaired);
    return !(bad==FD_DENIED && flawed==16 && good==FD_DENIED && repaired==10);
}
