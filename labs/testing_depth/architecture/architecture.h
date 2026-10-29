#ifndef TESTING_ARCHITECTURE_H
#define TESTING_ARCHITECTURE_H
#include <stddef.h>
#include <stdint.h>
/* Caller-owned typed objects, disjoint and live; serialized calls only.
   Read CONTRACTS.md before using these operations. No hidden allocation. */
enum { TA_MAX=8, TA_NONE=8 };
enum ta_status { TA_OK, TA_FULL, TA_EMPTY, TA_INVALID, TA_AGAIN,
               TA_TIMEOUT, TA_IO, TA_DONE, TA_CANCELLED };
enum ta_layout { TA_RING, TA_COMPACT, TA_LINKED, TA_COUNTED };
struct ta_node { unsigned char value; unsigned next; };
struct ta_storage { unsigned char bytes[TA_MAX+1]; struct ta_node nodes[TA_MAX]; };
struct ta_queue {
 struct ta_storage *storage; enum ta_layout layout;
 unsigned capacity,extent,head,tail,count,free_head;
};
enum ta_status ta_init(struct ta_queue *,struct ta_storage *,enum ta_layout,unsigned capacity,unsigned extent);
enum ta_status ta_push(struct ta_queue *,unsigned char);
enum ta_status ta_pop(struct ta_queue *,unsigned char *);
enum ta_status ta_peek(const struct ta_queue *,unsigned char *);
size_t ta_count(const struct ta_queue *);
/* Batch: accepted prefix is committed; *accepted is defined on every result. */
enum ta_status ta_batch(struct ta_queue *,const unsigned char *,size_t,size_t *accepted);
/* Separate representation-specific diagnostic, not the behavioral oracle. */
int ta_invariant(const struct ta_queue *);
/* Original packet: exactly {2,a,b}; staged decoder output on valid input. */
enum ta_status ta_decode(const unsigned char *,size_t,unsigned char out[2]);
enum ta_write_status { TA_WRITE_OK, TA_WRITE_WAIT, TA_WRITE_ERROR };
struct ta_write_result { enum ta_write_status status; size_t count; };
typedef struct ta_write_result (*ta_write_fn)(void *,const unsigned char *,size_t);
struct ta_flow {
 struct ta_queue queue; struct ta_storage storage; unsigned char bytes[2];
 size_t offset; unsigned budget; enum ta_status state; int uncertain;
};
/* Fresh destination; admitted begin copies packet and retains no input borrow. */
enum ta_status ta_begin(struct ta_flow *,const unsigned char *,size_t,enum ta_layout,unsigned budget);
enum ta_status ta_step(struct ta_flow *,ta_write_fn,void *);
enum ta_status ta_cancel(struct ta_flow *);
#endif
