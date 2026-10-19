#include "owning_ring.h"
#define MPO_PREFIX mpo_blob
#define MPO_RING MpoBlobRing
#define MPO_ELEMENT MpoBlob
#define MPO_OPS MpoBlobOps
#include "ring_template.inc"
#define MPO_PREFIX mpo_words
#define MPO_RING MpoWordsRing
#define MPO_ELEMENT MpoWords
#define MPO_OPS MpoWordsOps
#include "ring_template.inc"
