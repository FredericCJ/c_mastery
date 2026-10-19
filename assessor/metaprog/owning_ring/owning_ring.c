#include "owning_ring.h"
#ifndef MPO_MUTATION
#define MPO_MUTATION 0
#endif
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
#if defined(MPO_PREFIX) || defined(MPO_RING) || defined(MPO_ELEMENT) || defined(MPO_OPS) || defined(MPO_JOIN) || defined(MPO_JOIN_INNER)
#error MPO_PARAMETER_LEAK
#endif
