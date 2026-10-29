#ifndef CMASTERY_B04_H
#define CMASTERY_B04_H
#include <stddef.h>
#include <stdint.h>
enum { B4_OK=0, B4_INVALID=1, B4_RANGE=2, B4_CONFLICT=3, B4_STATE=4, B4_UNIMPLEMENTED=9 };
/* Finite original models; no claim that metadata establishes native pointers. */
int b4_serial(uint32_t first,uint32_t second,unsigned bits,int *order);
int b4_window(uint32_t base,uint32_t sequence,uint32_t span,unsigned bits,int *inside);
struct b4_graph { uint32_t cost[6][6]; unsigned vertices; };
/* UINT32_MAX denotes unreachable/no edge; predecessor is index or UINT_MAX. */
int b4_certificate(const struct b4_graph *g,unsigned source,const uint32_t *distance,
                   const unsigned *predecessor);
struct b4_framing { unsigned chunked; size_t body; };
/* Complete field section (no request line), final CRLF included; max512 octets. */
int b4_framing(const unsigned char *fields,size_t length,size_t body_limit,struct b4_framing *out);
struct b4_dma { unsigned owner,dirty,valid; uint32_t generation; };
enum { B4_CPU=0, B4_DEVICE=1, B4_DONE=2 };
enum { B4_WRITE=0,B4_CLEAN=1,B4_SUBMIT=2,B4_COMPLETE=3,B4_INVALIDATE=4,B4_RELEASE=5 };
int b4_dma(struct b4_dma *d,unsigned event,uint32_t generation);
int b4_cache_extent(size_t offset,size_t length,size_t line,size_t capacity,size_t *begin,size_t *end);
struct b4_allocation { uint32_t identity; size_t bytes; unsigned live; };
struct b4_ledger { struct b4_allocation slots[4]; size_t live,peak; };
enum { B4_ALLOC=0,B4_RESIZE=1,B4_FREE=2 };
/* A failed real allocator resize is recorded with succeeded=0: old object stays. */
int b4_memory(struct b4_ledger *l,unsigned operation,uint32_t identity,size_t bytes,int succeeded);
#endif
