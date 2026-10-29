#ifndef CORPUS_RING_H
#define CORPUS_RING_H
#include <stdint.h>
#include <stddef.h>
#define CR_TOKENS 32u
#define CR_OWNERS 16u
struct cr_token { uint16_t position; uint8_t owner, ordinal; };
struct cr_ring { struct cr_token token[CR_TOKENS]; size_t count; uint32_t epoch; };
enum cr_status { CR_OK,CR_INVALID,CR_EMPTY,CR_STALE,CR_FULL,CR_ABORTED,CR_UNAVAILABLE,CR_HASH_FAILED };
struct cr_cost { unsigned comparisons, visited; };
typedef int (*cr_hash_fn)(void *,const unsigned char *,size_t,unsigned char digest[32]);
enum cr_status cr_build(const struct cr_token *,size_t,uint32_t epoch,struct cr_ring *);
/* Read operations require a live ring previously admitted by cr_build/change. */
enum cr_status cr_owner(const struct cr_ring *,uint16_t key,uint32_t epoch,unsigned *owner,struct cr_cost *);
enum cr_status cr_change(struct cr_ring *,unsigned owner,const struct cr_token *new_tokens,size_t count,size_t abort_after);
enum cr_status cr_replicas(const struct cr_ring *,uint16_t key,uint32_t epoch,unsigned wanted,uint16_t unavailable,unsigned *owners,struct cr_cost *);
enum cr_status cr_route(const struct cr_ring *,uint32_t epoch,const unsigned char *,size_t,cr_hash_fn,void *,unsigned *,struct cr_cost *);
struct cr_move { uint16_t key; uint8_t before,after; };
enum cr_status cr_plan(const struct cr_ring *before,const struct cr_ring *after,const uint16_t *keys,size_t n,struct cr_move *moves,size_t capacity,size_t *count);
#endif
