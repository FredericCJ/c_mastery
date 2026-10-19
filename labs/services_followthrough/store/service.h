#ifndef SF_STORE_SERVICE_H
#define SF_STORE_SERVICE_H
#include "receipt.h"
enum {SFS_PUT=1,SFS_GET,SFS_BATCH,SFS_INCREMENT,SFS_VALUE,SFS_CHECKPOINT,
 SFS_DAMAGE_UNCOMMITTED,SFS_ARGUMENT=-30,SFS_SESSION=-31};
struct sfs_request {uint64_t session,id;uint32_t operation,client,epoch,klen,vlen,flags;int32_t delta;uint8_t key[16],value[64];};
struct sfs_reply {uint64_t session,id;int32_t status,result;uint32_t length;uint8_t value[64];};
struct sfs_config {uint64_t clients[2],reply[2];};
/* Single owner serializes all operations. Batch returns two fixed-key values
 * from one owner turn. Receipt IDs (32-bit) and epochs retain SDR semantics.
 * PUT success follows actual durable commit. It is an idempotent assignment;
 * only INCREMENT promises durable receipt deduplication. */
int sfs_apply(store *,const struct sfs_request *,struct sfs_reply *);
#endif
