#ifndef EDU7_PIPELINE_SUCCESSOR_H
#define EDU7_PIPELINE_SUCCESSOR_H
#include "physical.h"
#include "session.h"
enum {EW_OK,EW_INVALID,EW_BUSY,EW_STUB=99};
typedef struct {
 unsigned master,mode,queued,data_seen;EduFrame queue;EfFramer parser;
 EpMaster m;EpSlave s;
 uint32_t frames,bytes,faults,drops;
} EwPort;
typedef struct {
 unsigned node,profile,ports,event_ready,paused;uint32_t last,network,forwarded,route_drops,transport_drops,upper_drops,ack_drops;
 EwPort port[2];EfRoute route;unsigned route_protocol;EsPeer endpoint;EsEvent event;
 uint8_t incoming[40],outgoing[40];size_t incoming_length,outgoing_length;
} EwNode;
int ew_init(EwNode *,unsigned,unsigned,uint16_t,const R5Io *,const PuClock *,uint32_t);
int ew_queue(EwNode *,unsigned,const EduFrame *);
int ew_tick(EwNode *,uint32_t);
int ew_take(EwNode *,EsEvent *);
int ew_pause(EwNode *);
int ew_drained(const EwNode *);
int ew_rebind(EwNode *,uint16_t);
int ew_run(EwNode *);
int ew_f439_open(EwNode *,unsigned,unsigned,uint16_t,const PuClock *,uint32_t);
int ew_physical_verdict(unsigned software_ok,unsigned measured_mask);
int ew_tests(unsigned);
#endif
