#ifndef CMASTERY_META_OWNING_RING_H
#define CMASTERY_META_OWNING_RING_H
#include <stddef.h>
#include <stdint.h>
enum { MPO_OK, MPO_INVALID, MPO_FULL, MPO_EMPTY, MPO_CLONE };
typedef struct { unsigned char *bytes; size_t length; } MpoBlob;
typedef struct { uint32_t *values; size_t count; } MpoWords;
typedef struct { void *context; int (*clone)(void *,const MpoBlob *,MpoBlob *); void (*drop)(void *,MpoBlob *); } MpoBlobOps;
typedef struct { void *context; int (*clone)(void *,const MpoWords *,MpoWords *); void (*drop)(void *,MpoWords *); } MpoWordsOps;
typedef struct { MpoBlob data[4]; size_t head,count; MpoBlobOps ops; } MpoBlobRing;
typedef struct { MpoWords data[4]; size_t head,count; MpoWordsOps ops; } MpoWordsRing;
/* Serialized, noncopied owning rings. Inputs/output/control disjoint and live.
   Clone starts with an empty output; nonzero failure may leave a valid partial
   owner that drop must clean. Drop is total/nonfailing and leaves empty.
   Neither callback may reenter/mutate the ring or retain its borrowed inputs.
   Pop output must own nothing; success transfers ownership. */
int mpo_blob_init(MpoBlobRing *,MpoBlobOps);
int mpo_blob_push(MpoBlobRing *,const MpoBlob *);
int mpo_blob_pop(MpoBlobRing *,MpoBlob *);
void mpo_blob_clear(MpoBlobRing *);
int mpo_words_init(MpoWordsRing *,MpoWordsOps);
int mpo_words_push(MpoWordsRing *,const MpoWords *);
int mpo_words_pop(MpoWordsRing *,MpoWords *);
void mpo_words_clear(MpoWordsRing *);
/* Handwritten equivalent with exactly the same typed representation/contract. */
int mpo_manual_init(MpoBlobRing *,MpoBlobOps);
int mpo_manual_push(MpoBlobRing *,const MpoBlob *);
int mpo_manual_pop(MpoBlobRing *,MpoBlob *);
void mpo_manual_clear(MpoBlobRing *);
/* Explicit erased operations operate on real typed caller storage, never a
   declared character array cast into an owning struct. Five disjoint live empty
   objects (four slots + scratch), matching ops and empty pop output are required.
   Move consumes source into empty destination and cannot fail. */
typedef struct { void *context; int (*clone)(void *,const void *,void *); void (*drop)(void *,void *); void (*move)(void *,void *,void *); } MpoErasedOps;
typedef struct { void *slots[4],*scratch; size_t head,count; MpoErasedOps ops; } MpoErasedRing;
int mpo_erased_init(MpoErasedRing *,void *const[4],void *,MpoErasedOps);
int mpo_erased_push(MpoErasedRing *,const void *);
int mpo_erased_pop(MpoErasedRing *,void *);
void mpo_erased_clear(MpoErasedRing *);
#endif
