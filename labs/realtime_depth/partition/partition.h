#ifndef CMASTERY_REALTIME_PARTITION_H
#define CMASTERY_REALTIME_PARTITION_H
#include "cmx_advanced.h"
#define RP_TASKS 8u
#define RP_CPUS 4u
#define RP_HORIZON 256u
enum rp_status {RP_OK,RP_INVALID,RP_FULL,RP_IMPOSSIBLE,RP_BUSY,RP_STALE,RP_UNIMPLEMENTED};
struct rp_task {cmx_job job;uint32_t cost[RP_CPUS];unsigned affinity;};
struct rp_plan {unsigned n,cpus,cpu[RP_TASKS];uint32_t horizon,used[RP_CPUS];};
/* All objects live/disjoint; core calls externally serialized. cost[c] is the
 * supplied execution bound on CPU c; job.cost is retained producer metadata. */
enum rp_status rp_fit(const struct rp_task *,unsigned,unsigned,uint32_t,unsigned,struct rp_plan *);
enum rp_status rp_exact(const struct rp_task *,unsigned,unsigned,uint32_t,unsigned,struct rp_plan *);
enum rp_status rp_verify(const struct rp_task *,unsigned,const struct rp_plan *,unsigned);
enum rp_status rp_simulate(const struct rp_task *,unsigned,const struct rp_plan *,cmx_sim_result *);
/* Explicit one-owner handoff. A borrowed work interval must finish before
 * preparation. All operations require the same external mutex/serialization. */
struct rp_lease {unsigned owner,target,busy,prepared;uint64_t epoch,work;};
enum rp_status rp_lease_init(struct rp_lease *,unsigned);
enum rp_status rp_begin(struct rp_lease *,unsigned,uint64_t);
enum rp_status rp_finish(struct rp_lease *,unsigned,uint64_t,uint32_t);
enum rp_status rp_prepare(struct rp_lease *,unsigned,uint64_t,unsigned);
enum rp_status rp_accept(struct rp_lease *,unsigned,uint64_t);
enum rp_status rp_abort(struct rp_lease *,unsigned,uint64_t);
#endif
