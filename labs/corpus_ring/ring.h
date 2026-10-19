#ifndef CORPUS_RING_H
#define CORPUS_RING_H
#include <stdint.h>
#include <stddef.h>
#define CR_TOKENS 32u
#define CR_OWNERS 16u
typedef struct { uint16_t position; uint8_t owner, ordinal; } CrToken;
typedef struct { CrToken token[CR_TOKENS]; size_t count; uint32_t epoch; } CrRing;
typedef enum { CR_OK,CR_INVALID,CR_EMPTY,CR_STALE,CR_FULL,CR_ABORTED,CR_UNAVAILABLE,CR_HASH_FAILED } CrStatus;
typedef struct { unsigned comparisons, visited; } CrCost;
typedef int (*CrHash)(void *,const unsigned char *,size_t,unsigned char digest[32]);
CrStatus cr_build(const CrToken *,size_t,uint32_t epoch,CrRing *);
/* Read operations require a live ring previously admitted by cr_build/change. */
CrStatus cr_owner(const CrRing *,uint16_t key,uint32_t epoch,unsigned *owner,CrCost *);
CrStatus cr_change(CrRing *,unsigned owner,const CrToken *new_tokens,size_t count,size_t abort_after);
CrStatus cr_replicas(const CrRing *,uint16_t key,uint32_t epoch,unsigned wanted,uint16_t unavailable,unsigned *owners,CrCost *);
CrStatus cr_route(const CrRing *,uint32_t epoch,const unsigned char *,size_t,CrHash,void *,unsigned *,CrCost *);
typedef struct { uint16_t key; uint8_t before,after; } CrMove;
CrStatus cr_plan(const CrRing *before,const CrRing *after,const uint16_t *keys,size_t n,CrMove *moves,size_t capacity,size_t *count);
#endif
