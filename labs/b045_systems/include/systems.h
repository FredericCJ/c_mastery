#ifndef CMASTERY_B045_SYSTEMS_H
#define CMASTERY_B045_SYSTEMS_H
#include <stddef.h>
#include <stdint.h>
enum { S_OK=0,S_INVALID=1,S_RANGE=2,S_CONFLICT=3,S_PENDING=4,S_UNIMPLEMENTED=9 };
struct s_instruction { unsigned opcode,destination,left,right,taken; };
enum { S_ALU=0,S_LOAD=1,S_BRANCH=2 };
int s_pipeline(const struct s_instruction*,size_t,unsigned forwarding,unsigned issue[16],unsigned *cycles);
struct s_si2c { unsigned byte,bit,state; };
enum { S_I2C_DATA=0,S_I2C_ACK=1,S_I2C_DONE=2,S_I2C_NACK=3,S_I2C_LOST=4 };
int s_i2c_drive(const struct s_si2c*,unsigned *release_sda);
int s_i2c_sample(struct s_si2c*,unsigned scl,unsigned sda);
struct s_checksum { uint32_t sum;unsigned pending,octet; };
int s_checksum_add(struct s_checksum*,const unsigned char*,size_t);
int s_checksum_finish(const struct s_checksum*,uint16_t*);
int s_paired(const uint32_t *a,const uint32_t *b,size_t,int64_t *median,unsigned *b_wins);
enum { S_PASS=0,S_FAIL=1,S_UNRESOLVED=2 };
typedef int (*s_classify_fn)(void*,const unsigned char*,size_t);
int s_reduce(const unsigned char*,size_t,s_classify_fn,void*,unsigned budget,
             unsigned char output[32],size_t *length,unsigned *calls);
enum { S_FIFO=0,S_OPTIMAL=1,S_LRU=2 };
struct s_replacement { unsigned faults;int evicted[32]; };
int s_replace(const unsigned char*,size_t,unsigned frames,unsigned policy,struct s_replacement*);
struct s_wait { unsigned owner,predicate,waiting,ready; };
enum { S_LOCK=0,S_UNLOCK=1,S_WAIT=2,S_PUBLISH=3,S_SIGNAL=4 };
int s_wait_event(struct s_wait*,unsigned actor,unsigned operation);
struct s_shootdown { uint32_t generation;unsigned active,pending,targets,reclaimed; };
int s_shoot_begin(struct s_shootdown*,unsigned targets);
int s_shoot_ack(struct s_shootdown*,unsigned cpu,uint32_t generation);
int s_shoot_reclaim(struct s_shootdown*);
struct s_peer { uint32_t epoch,sequence;unsigned outstanding,uncertain,ready; };
int s_peer_submit(struct s_peer*,uint32_t *epoch,uint32_t *sequence);
int s_peer_ack(struct s_peer*,uint32_t epoch,uint32_t sequence);
int s_peer_restart(struct s_peer*,unsigned trusted_quiescence);
int s_peer_ready(struct s_peer*,uint32_t epoch);
#endif
