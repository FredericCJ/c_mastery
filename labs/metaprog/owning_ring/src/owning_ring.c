#include "owning_ring.h"
#define MPO_PREFIX mpo_blob
#define MPO_RING struct mpo_blob_ring
#define MPO_ELEMENT struct mpo_blob
#define MPO_OPS struct mpo_blob_ops
#include "ring_template.inc"
#define MPO_PREFIX mpo_words
#define MPO_RING struct mpo_words_ring
#define MPO_ELEMENT struct mpo_words
#define MPO_OPS struct mpo_words_ops
#include "ring_template.inc"
