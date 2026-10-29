#ifndef CM_LIFECYCLE_EXTENSION_H
#define CM_LIFECYCLE_EXTENSION_H
#include <stddef.h>
#include <stdint.h>
enum { LCE_RECORD_BYTES=24 };
typedef int (*lce_verify_fn)(void *,unsigned,const unsigned char *,size_t,const unsigned char *,size_t);
/* Immutable owned image: LE32 version, one-byte key ID (1 or2), >=1 payload
 * byte. Signature covers this entire extent, not just payload. trusted/revoked
 * use bit key_id. Verifier returns exactly1 for cryptographic success. */
int lce_admit(const unsigned char *,size_t,const unsigned char *,size_t,unsigned,unsigned,uint32_t,lce_verify_fn,void *,uint32_t *);
/* Each record pointer denotes exactly24 readable/writable bytes as appropriate. */
int lce_record(unsigned char *,uint32_t sequence,uint32_t value);
int lce_valid(const unsigned char *,uint32_t *,uint32_t *);
int lce_choose(const unsigned char *,const unsigned char *,unsigned *);
int lce_next(uint32_t current,uint32_t limit,uint32_t *);
struct lce_joint {uint32_t generation,limit;unsigned healthy,outstanding[2];};
int lce_joint_reset(struct lce_joint *);
int lce_joint_ack(struct lce_joint *,unsigned,uint32_t,unsigned);
int lce_joint_confirm(const struct lce_joint *);
struct lce_fault {unsigned busy,valid,nested;uint32_t words[4];};
int lce_fault_begin(struct lce_fault *);
int lce_fault_commit(struct lce_fault *,uint32_t,uint32_t);
#endif
