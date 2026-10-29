#ifndef ETHERNET_FOLLOWTHROUGH_PHY_H
#define ETHERNET_FOLLOWTHROUGH_PHY_H
#include "ethernet_depth.h"
enum {EP_IDLE,EP_RESET,EP_RESTART,EP_RESTART_WAIT,EP_NEGOTIATE,EP_READY,EP_FAILED};
struct ep_phy {
 struct ed_bus bus;
 uint32_t hclk,incarnation,start,last,deadline;
 unsigned address,access_polls,access_timeout,remaining,phase,initialized,ready;
 struct ed_phy mode;
};
struct ep_link {const struct ep_phy *owner;uint32_t incarnation;struct ed_phy mode;};
int ep_write(const struct ed_bus *,uint32_t hclk,unsigned address,unsigned reg,unsigned polls,uint32_t timeout,uint16_t value);
int ep_find(const struct ed_bus *,uint32_t hclk,unsigned polls,uint32_t timeout,unsigned *address);
int ep_init(struct ep_phy *,const struct ed_bus *,uint32_t hclk,unsigned address,unsigned access_polls,unsigned access_timeout);
int ep_begin(struct ep_phy *,unsigned total_steps,uint32_t deadline);
int ep_step(struct ep_phy *);
int ep_poll(struct ep_phy *);
int ep_invalidate(struct ep_phy *);
int ep_snapshot(const struct ep_phy *,struct ep_link *);
int ep_validate(const struct ep_phy *,const struct ep_link *);
struct ep_mmio_clock {void *context;uint32_t (*clock)(void *);};
int ep_f439_bus(struct ep_mmio_clock *,struct ed_bus *);
#endif
