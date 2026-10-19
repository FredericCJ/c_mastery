#include "owning_ring.h"
#define MPO_MUTATION 0
#define MPO_JOIN x
#define MPO_PREFIX mpo_blob
#define MPO_ELEMENT MpoBlob
#define MPO_RING MpoBlobRing
#define MPO_OPS MpoBlobOps
#include "ring_template.inc"
