#ifndef AF_TARGET_H
#define AF_TARGET_H
#include "amp.h"
#include "doorbell.h"
#ifndef AF_SHARED
#define AF_SHARED 0x30040000u
#endif
#ifndef AF_CACHED
#define AF_CACHED 0
#endif
#ifndef AF_M7
#define AF_M7 1
#endif
#ifndef AF_QEMU
#define AF_QEMU 0
#endif
typedef struct { AfBoot boot;AfRing request,response;_Atomic uint32_t progress[2],fault[2],irqs[2],ticks[2],samples,dropped,done,traffic,high[3];uint32_t rejected[16],completed[16],trace[128]; } AfShared;
_Static_assert(sizeof(AfShared)<=8192,"Shared noncacheable control region");
_Static_assert(sizeof(_Atomic uint32_t)==4,"Fixed prefix word ABI");
_Static_assert(offsetof(AfBoot,ack)==16,"Fixed prefix ack ABI");
_Static_assert(offsetof(AfRing,tail)==64,"Index ownership isolation");
void af_entry(void);
void af_systick(void);
void af_hsem(void);
void af_fault(void);
extern unsigned char af_vectors[];
#endif
