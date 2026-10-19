#ifndef CMASTERY_B045_SYSTEMS_H
#define CMASTERY_B045_SYSTEMS_H
#include <stddef.h>
#include <stdint.h>
enum { S_OK=0,S_INVALID=1,S_RANGE=2,S_CONFLICT=3,S_PENDING=4,S_UNIMPLEMENTED=9 };
typedef struct { unsigned opcode,destination,left,right,taken; } SInstruction;
enum { S_ALU=0,S_LOAD=1,S_BRANCH=2 };
int s_pipeline(const SInstruction*,size_t,unsigned forwarding,unsigned issue[16],unsigned *cycles);
typedef struct { unsigned byte,bit,state; } SI2c;
enum { S_I2C_DATA=0,S_I2C_ACK=1,S_I2C_DONE=2,S_I2C_NACK=3,S_I2C_LOST=4 };
int s_i2c_drive(const SI2c*,unsigned *release_sda);
int s_i2c_sample(SI2c*,unsigned scl,unsigned sda);
typedef struct { uint32_t sum;unsigned pending,octet; } SChecksum;
int s_checksum_add(SChecksum*,const unsigned char*,size_t);
int s_checksum_finish(const SChecksum*,uint16_t*);
int s_paired(const uint32_t *a,const uint32_t *b,size_t,int64_t *median,unsigned *b_wins);
enum { S_PASS=0,S_FAIL=1,S_UNRESOLVED=2 };
typedef int (*SClassify)(void*,const unsigned char*,size_t);
int s_reduce(const unsigned char*,size_t,SClassify,void*,unsigned budget,
             unsigned char output[32],size_t *length,unsigned *calls);
enum { S_FIFO=0,S_OPTIMAL=1,S_LRU=2 };
typedef struct { unsigned faults;int evicted[32]; } SReplacement;
int s_replace(const unsigned char*,size_t,unsigned frames,unsigned policy,SReplacement*);
typedef struct { unsigned owner,predicate,waiting,ready; } SWait;
enum { S_LOCK=0,S_UNLOCK=1,S_WAIT=2,S_PUBLISH=3,S_SIGNAL=4 };
int s_wait_event(SWait*,unsigned actor,unsigned operation);
typedef struct { uint32_t generation;unsigned active,pending,targets,reclaimed; } SShootdown;
int s_shoot_begin(SShootdown*,unsigned targets);
int s_shoot_ack(SShootdown*,unsigned cpu,uint32_t generation);
int s_shoot_reclaim(SShootdown*);
typedef struct { uint32_t epoch,sequence;unsigned outstanding,uncertain,ready; } SPeer;
int s_peer_submit(SPeer*,uint32_t *epoch,uint32_t *sequence);
int s_peer_ack(SPeer*,uint32_t epoch,uint32_t sequence);
int s_peer_restart(SPeer*,unsigned trusted_quiescence);
int s_peer_ready(SPeer*,uint32_t epoch);
#endif
