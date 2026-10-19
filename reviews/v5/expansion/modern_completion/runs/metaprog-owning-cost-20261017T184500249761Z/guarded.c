#include "owning_ring.h"
#define MPO_MUTATION 0
#define MPO_PREFIX mpo_blob
#define MPO_ELEMENT MpoBlob
#define MPO_RING MpoBlobRing
#define MPO_OPS MpoBlobOps
#include "guarded.inc"
#define MPO_PREFIX mpo_words
#define MPO_ELEMENT MpoWords
#define MPO_RING MpoWordsRing
#define MPO_OPS MpoWordsOps
#include "guarded.inc"
