#ifndef SYSTEM_FOLLOW_H
#define SYSTEM_FOLLOW_H
#include "protocol_follow.h"
#include "cmx.h"
#include "adt.h"
#include "algo.h"
enum {PF_COMPLETE=1,PF_RESET=2,PF_QUIET=3};
typedef struct {ADBrand brand;PFRecovery recovery;uint32_t epoch;unsigned kind,idle;} PFMessage;
typedef struct {PFDriver driver;cmx_kernel kernel;cmx_queue queue;ADPool pool;PFMessage messages[4];algo_ring history;unsigned lost_history;uint64_t deadline,last_time;unsigned timer_armed;PFRecovery timer_recovery;_Alignas(16) unsigned char stacks[3][512];} PFSystem;
int pf_system_init(PFSystem *,FwIo,uint32_t,PFFree,void *);
int pf_system_post(PFSystem *,unsigned producer,unsigned kind,PFRecovery,uint32_t,unsigned,ADBrand *);
int pf_system_step(PFSystem *);
int pf_system_arm_deadline(PFSystem *,PFRecovery,uint64_t,unsigned);
int pf_system_tick(PFSystem *,uint64_t);
#endif
