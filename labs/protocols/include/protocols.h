#ifndef CMASTERY_PROTOCOLS_H
#define CMASTERY_PROTOCOLS_H
#include <stddef.h>
#include <stdint.h>
enum { PT_OK=0,PT_INVALID=1,PT_RANGE=2,PT_STATE=3,PT_PENDING=4,PT_IO=5,PT_RECOVERY=6,PT_DONE=7,PT_UNIMPLEMENTED=9 };
enum { PT_CLOSED=0,PT_READY=1,PT_RECOVERY_REQUIRED=2 };
typedef struct { uint32_t generation;unsigned state,control,cleanup; } PTDevice;
typedef struct { PTDevice *device;uint32_t generation; } PTClosed;
typedef struct { PTDevice *device;uint32_t generation; } PTReady;
void pt_device_init(PTDevice*,PTClosed*);
int pt_configure(PTClosed*,unsigned fail_stage,PTReady*);
int pt_recover(PTDevice*,unsigned trusted_quiescence,PTClosed*);
int pt_close(PTReady*,PTClosed*);
typedef struct { unsigned mask,address,length,timeout; } PTBuilder;
typedef struct { unsigned address,length,timeout; } PTConfig;
int pt_builder_set(PTBuilder*,unsigned field,unsigned value);
int pt_builder_finish(const PTBuilder*,PTConfig*);
typedef struct { unsigned role,phase,remaining; } PTSession;
enum { PT_HELLO=1,PT_ACCEPT=2,PT_DATA=3,PT_ACK=4,PT_BYE=5 };
int pt_session_step(PTSession*,unsigned sending,unsigned tag);
enum { PT_ROOT=0,PT_OFFLINE=1,PT_ONLINE=2,PT_IDLE=3,PT_BUSY=4,PT_FAULT=5 };
enum { PT_CONNECT=0,PT_START=1,PT_FINISH=2,PT_FAIL=3,PT_RESET=4,PT_PING=5 };
typedef struct { unsigned active,dispatching,pings; } PTMachine;
int pt_dispatch(PTMachine*,unsigned event,unsigned char *trace,size_t capacity,size_t *used);
typedef struct { PTMachine machine;unsigned queue[4],head,count; } PTActive;
int pt_post(PTActive*,unsigned event);
int pt_active_step(PTActive*,unsigned char *trace,size_t capacity,size_t *used);
typedef struct { unsigned pc,count;uint32_t sum; } PTThread;
int pt_thread_sum(PTThread*,unsigned available,uint32_t value,uint32_t *sum);
/* Explicit labels make duplicate labels a translation constraint, not a
 * collision hidden behind __LINE__. No automatic local survives suspension. */
#define PT_BEGIN(p) switch((p)->pc) { case 0:
#define PT_WAIT(p,label,condition) do { (p)->pc=(label); case(label): if(!(condition))return PT_PENDING; } while(0)
#define PT_END(p) } (p)->pc=UINT32_MAX;return PT_DONE
enum { PT_BT_LEAF=0,PT_BT_SEQUENCE=1,PT_BT_SELECTOR=2 };
enum { PT_BT_SUCCESS=0,PT_BT_FAILURE=1,PT_BT_RUNNING=2 };
typedef struct { unsigned kind,first,count; } PTNode;
typedef struct { unsigned cursor[8],active_mask,ticking; } PTBehavior;
typedef unsigned (*PTTick)(void*,unsigned leaf);
typedef void (*PTAbort)(void*,unsigned leaf);
int pt_behavior_tick(PTBehavior*,const PTNode*,size_t,PTTick,void*,unsigned *result);
int pt_behavior_cancel(PTBehavior*,PTAbort,void*);
typedef struct {
 void *context;
 int (*start_dma)(void*,const unsigned char*,size_t);
 int (*stop_and_quiesce)(void*);
 void (*watchdog)(void*);
} PTPort;
typedef struct { PTDevice device;PTPort port;const unsigned char *buffer;size_t length;unsigned dma_owned; } PTDriver;
int pt_driver_start(PTDriver*,const unsigned char*,size_t);
int pt_driver_complete(PTDriver*,unsigned device_error,unsigned trusted_quiescence);
int pt_driver_recover(PTDriver*);
#endif
