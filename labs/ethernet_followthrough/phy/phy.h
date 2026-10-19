#ifndef ETHERNET_FOLLOWTHROUGH_PHY_H
#define ETHERNET_FOLLOWTHROUGH_PHY_H
#include "ethernet_depth.h"
enum {EP_IDLE,EP_RESET,EP_RESTART,EP_RESTART_WAIT,EP_NEGOTIATE,EP_READY,EP_FAILED};
typedef struct {
 EDBus bus;
 uint32_t hclk,incarnation,start,last,deadline;
 unsigned address,access_polls,access_timeout,remaining,phase,initialized,ready;
 EDPhy mode;
} EpPhy;
typedef struct {const EpPhy *owner;uint32_t incarnation;EDPhy mode;} EpLink;
int ep_write(const EDBus *,uint32_t hclk,unsigned address,unsigned reg,unsigned polls,uint32_t timeout,uint16_t value);
int ep_find(const EDBus *,uint32_t hclk,unsigned polls,uint32_t timeout,unsigned *address);
int ep_init(EpPhy *,const EDBus *,uint32_t hclk,unsigned address,unsigned access_polls,unsigned access_timeout);
int ep_begin(EpPhy *,unsigned total_steps,uint32_t deadline);
int ep_step(EpPhy *);
int ep_poll(EpPhy *);
int ep_invalidate(EpPhy *);
int ep_snapshot(const EpPhy *,EpLink *);
int ep_validate(const EpPhy *,const EpLink *);
typedef struct {void *context;uint32_t (*clock)(void *);} EpMmioClock;
int ep_f439_bus(EpMmioClock *,EDBus *);
#endif
