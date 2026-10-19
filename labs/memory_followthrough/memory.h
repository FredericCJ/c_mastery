#ifndef MEMORY_FOLLOWTHROUGH_H
#define MEMORY_FOLLOWTHROUGH_H
#include "foundation.h"
#include <stddef.h>
#include <stdint.h>
typedef enum { MW_OK,MW_INPUT,MW_EXTENT,MW_INIT,MW_NUL,MW_OVERLAP,MW_MULTIPLY,MW_ADD,MW_ZERO,MW_ALIGN,MW_NOMEM,MW_BUSY,MW_STALE,MW_RETIRED,MW_UNIMPLEMENTED=99 } MwStatus;
/* Offsets describe one actual live backing array, so overlap reasoning never
 * orders unrelated pointers. capacity is the independent logical extent. */
typedef struct {size_t offset,capacity,initialized;} MwSpan;
MwStatus mw_fill(unsigned char *,size_t,MwSpan,ptrdiff_t,size_t,unsigned char);
MwStatus mw_scan(const unsigned char *,size_t,MwSpan,size_t *);
MwStatus mw_string(unsigned char *,size_t,MwSpan,MwSpan,size_t,size_t *);
MwStatus mw_move(unsigned char *,size_t,MwSpan,MwSpan,size_t,size_t *);
MwStatus mw_size(size_t,size_t,size_t,size_t,size_t,size_t,size_t *);
MwStatus mw_signed_size(intmax_t,size_t,size_t,size_t *);
typedef struct {size_t low,high;} MwInterval;
typedef struct {MwInterval data,bss,retained,heap,guard,stack;size_t fixed,gap,flash_bytes;unsigned fixed_fits,reservation_fits;} MwLayout;
/* Report representable geometry even when it exceeds region capacity. Invalid
 * arithmetic/alignment leaves the report unchanged. A false fits flag denies use. */
MwStatus mw_layout(size_t,size_t,size_t,size_t,size_t,size_t,size_t,size_t,MwLayout *);
MwStatus mw_startup(unsigned char *,size_t,const unsigned char *,size_t,MwInterval,MwInterval,MwInterval);
typedef struct {void *context;void *(*take)(void *,size_t);void *(*resize)(void *,void *,size_t);void (*drop)(void *,void *);} MwAllocator;
typedef struct {unsigned char *data;size_t size;uint32_t generation;unsigned pins,retired;MwAllocator allocator;} MwOwner;
typedef struct {uint32_t generation;size_t offset;} MwBorrow;
MwStatus mw_owner(MwOwner *,MwAllocator,size_t);
MwStatus mw_resize(MwOwner *,size_t,unsigned);
MwStatus mw_read(const MwOwner *,MwBorrow,unsigned char *);
MwStatus mw_drop(MwOwner *,uint32_t);
typedef struct {uint32_t length,mode,tag,version,optional;unsigned char *payload;MwAllocator allocator;} MwRecord;
MwStatus mw_record(MwRecord *,MwAllocator,const unsigned char *,size_t,unsigned,unsigned,unsigned,unsigned);
MwStatus mw_serialize(const MwRecord *,unsigned char *,size_t,size_t *);
void mw_record_drop(MwRecord *);
typedef struct {FAsync operation;uint32_t generation,request;unsigned started,submit_active,callback_active,callback_pending,retired,scratch;} MwOperation;
typedef struct {MwOperation slots[2];unsigned scratch_mask,callbacks,peak_callbacks,results;} MwAsync;
typedef struct {unsigned slot;uint32_t generation,request;} MwTicket;
void mw_async_init(MwAsync *);
MwStatus mw_submit(MwAsync *,uint32_t,MwTicket *);
MwStatus mw_submit_return(MwAsync *,MwTicket);
MwStatus mw_cancel(MwAsync *,MwTicket);
MwStatus mw_terminal(MwAsync *,MwTicket);
MwStatus mw_callback_begin(MwAsync *,MwTicket,unsigned);
MwStatus mw_callback_end(MwAsync *,MwTicket);
MwStatus mw_reclaim(MwAsync *,MwTicket);
/* Full binary teaching tree: depth<=6, leaves carry value1. Caller work storage
 * owns capacity; on refusal no result is published. breadth=0 DFS,1 BFS. */
MwStatus mw_tree(unsigned,unsigned,unsigned *,size_t,unsigned *,size_t *);
#endif
