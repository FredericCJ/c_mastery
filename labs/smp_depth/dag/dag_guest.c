#include "cmk2_abi.h"
void _start(void){static const char marker[]="DG_GUEST_CHECK:unsolved-dag-target-adapter\n";cmk_write(marker,sizeof marker-1);cmk_call(CMK_EXIT,91,0,0);for(;;){}}
