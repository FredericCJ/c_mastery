#ifndef CMASTERY_META_OWNING_RING_H
#define CMASTERY_META_OWNING_RING_H
#include <stddef.h>
#include <stdint.h>
enum { MPO_OK, MPO_INVALID, MPO_FULL, MPO_EMPTY, MPO_CLONE };
struct mpo_blob { unsigned char *bytes; size_t length; };
struct mpo_words { uint32_t *values; size_t count; };
struct mpo_blob_ops { void *context; int (*clone)(void *,const struct mpo_blob *,struct mpo_blob *); void (*drop)(void *,struct mpo_blob *); };
struct mpo_words_ops { void *context; int (*clone)(void *,const struct mpo_words *,struct mpo_words *); void (*drop)(void *,struct mpo_words *); };
struct mpo_blob_ring { struct mpo_blob data[4]; size_t head,count; struct mpo_blob_ops ops; };
struct mpo_words_ring { struct mpo_words data[4]; size_t head,count; struct mpo_words_ops ops; };
/* Serialized, noncopied owning rings. Inputs/output/control disjoint and live.
   Clone starts with an empty output; nonzero failure may leave a valid partial
   owner that drop must clean. Drop is total/nonfailing and leaves empty.
   Neither callback may reenter/mutate the ring or retain its borrowed inputs.
   Pop output must own nothing; success transfers ownership. */
int mpo_blob_init(struct mpo_blob_ring *,struct mpo_blob_ops);
int mpo_blob_push(struct mpo_blob_ring *,const struct mpo_blob *);
int mpo_blob_pop(struct mpo_blob_ring *,struct mpo_blob *);
void mpo_blob_clear(struct mpo_blob_ring *);
int mpo_words_init(struct mpo_words_ring *,struct mpo_words_ops);
int mpo_words_push(struct mpo_words_ring *,const struct mpo_words *);
int mpo_words_pop(struct mpo_words_ring *,struct mpo_words *);
void mpo_words_clear(struct mpo_words_ring *);
/* Handwritten equivalent with exactly the same typed representation/contract. */
int mpo_manual_init(struct mpo_blob_ring *,struct mpo_blob_ops);
int mpo_manual_push(struct mpo_blob_ring *,const struct mpo_blob *);
int mpo_manual_pop(struct mpo_blob_ring *,struct mpo_blob *);
void mpo_manual_clear(struct mpo_blob_ring *);
/* Explicit erased operations operate on real typed caller storage, never a
   declared character array cast into an owning struct. Five disjoint live empty
   objects (four slots + scratch), matching ops and empty pop output are required.
   Move consumes source into empty destination and cannot fail. */
struct mpo_erased_ops { void *context; int (*clone)(void *,const void *,void *); void (*drop)(void *,void *); void (*move)(void *,void *,void *); };
struct mpo_erased_ring { void *slots[4],*scratch; size_t head,count; struct mpo_erased_ops ops; };
int mpo_erased_init(struct mpo_erased_ring *,void *const[4],void *,struct mpo_erased_ops);
int mpo_erased_push(struct mpo_erased_ring *,const void *);
int mpo_erased_pop(struct mpo_erased_ring *,void *);
void mpo_erased_clear(struct mpo_erased_ring *);
#endif
