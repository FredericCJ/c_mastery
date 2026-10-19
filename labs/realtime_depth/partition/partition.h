#ifndef CMASTERY_REALTIME_PARTITION_H
#define CMASTERY_REALTIME_PARTITION_H
#include "cmx_advanced.h"
#define RP_TASKS 8u
#define RP_CPUS 4u
#define RP_HORIZON 256u
enum rp_status {RP_OK,RP_INVALID,RP_FULL,RP_IMPOSSIBLE,RP_BUSY,RP_STALE,RP_UNIMPLEMENTED};
typedef struct {cmx_job job;uint32_t cost[RP_CPUS];unsigned affinity;} RPTask;
typedef struct {unsigned n,cpus,cpu[RP_TASKS];uint32_t horizon,used[RP_CPUS];} RPPlan;
/* All objects live/disjoint; core calls externally serialized. cost[c] is the
 * supplied execution bound on CPU c; job.cost is retained producer metadata. */
enum rp_status rp_fit(const RPTask *,unsigned,unsigned,uint32_t,unsigned,RPPlan *);
enum rp_status rp_exact(const RPTask *,unsigned,unsigned,uint32_t,unsigned,RPPlan *);
enum rp_status rp_verify(const RPTask *,unsigned,const RPPlan *,unsigned);
enum rp_status rp_simulate(const RPTask *,unsigned,const RPPlan *,cmx_sim_result *);
/* Explicit one-owner handoff. A borrowed work interval must finish before
 * preparation. All operations require the same external mutex/serialization. */
typedef struct {unsigned owner,target,busy,prepared;uint64_t epoch,work;} RPLease;
enum rp_status rp_lease_init(RPLease *,unsigned);
enum rp_status rp_begin(RPLease *,unsigned,uint64_t);
enum rp_status rp_finish(RPLease *,unsigned,uint64_t,uint32_t);
enum rp_status rp_prepare(RPLease *,unsigned,uint64_t,unsigned);
enum rp_status rp_accept(RPLease *,unsigned,uint64_t);
enum rp_status rp_abort(RPLease *,unsigned,uint64_t);
#endif
