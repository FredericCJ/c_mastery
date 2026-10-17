/* Link-audit entry only: no reset startup/vector table and no hardware access. */
#include "depth.h"
volatile unsigned fd_audit_status;
volatile int32_t fd_audit_value;
static FDPool pool;
void _start(void) {
    unsigned char wire[8],move[8]; size_t extent;
    FDRecord input={3,2,-3},decoded;
    FDHandle handle; FDValue value;
    FDCursor c,sub; const unsigned char *view;
    int32_t counter=10;
    fd_audit_status=(unsigned)fd_extent(4,1,4,8,&extent);
    fd_audit_status+=(unsigned)fd_encode(input,wire,8);
    fd_audit_status+=(unsigned)fd_decode(wire,8,&decoded);
    c.base=wire;c.length=8;c.position=0;
    fd_audit_status+=(unsigned)fd_subview(&c,0,8,&sub);
    fd_audit_status+=(unsigned)fd_take(&sub,8,&view);
    fd_transform(move,wire,8,1);
    fd_audit_status+=(unsigned)fd_move(move,8,1,0,7);
    fd_pool_init(&pool);
    value.value=decoded.value;value.serial=decoded.count;
    fd_audit_status+=(unsigned)fd_pool_acquire(&pool,value,&handle);
    fd_audit_status+=(unsigned)fd_pool_get(&pool,handle,&value);
    fd_audit_status+=(unsigned)fd_pool_release(&pool,handle);
    fd_audit_status+=(unsigned)fd_apply(wire,8,1,&counter);
    fd_audit_value=counter;
    for (;;) { }
}
