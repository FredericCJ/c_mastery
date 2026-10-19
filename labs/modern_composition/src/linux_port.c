#include "linux_port.h"
int mc_linux_init(McLinuxPort *p,McRead *r,int f){(void)p;(void)r;(void)f;return 0;}
COAsyncPort mc_linux_interface(McLinuxPort *p){(void)p;return (COAsyncPort){0};}
int mc_linux_wait(McLinuxPort *p,unsigned t){(void)p;(void)t;return 0;}
int mc_linux_dispatch(McLinuxPort *p){(void)p;return 0;}
int mc_linux_request_cancel(McLinuxPort *p){(void)p;return 0;}
MsgStatus mc_linux_finish_cancel(McLinuxPort *p){(void)p;return MSG_FAILED;}
int mc_linux_quiescent(McLinuxPort *p){(void)p;return 0;}
int mc_linux_destroy(McLinuxPort *p){(void)p;return 0;}
