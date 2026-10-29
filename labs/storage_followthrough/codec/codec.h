#ifndef STORAGE_FOLLOWTHROUGH_CODEC_H
#define STORAGE_FOLLOWTHROUGH_CODEC_H
#include "store.h"
/* Original CST3L leaf-only page format, separate from the retained CST2 engine.
   All inputs live for the synchronous call; outputs preserve on failure.
   Byte input/output may overlap because complete decoding/encoding is staged. */
int sf_leaf_encode(uint32_t id,uint64_t generation,const store_node *,uint8_t out[STORE_PAGE_SIZE]);
int sf_leaf_decode(uint32_t id,uint64_t generation,const uint8_t *,size_t,store_node *);
int sf_leaf_upgrade(uint32_t id,uint64_t generation,const uint8_t *,size_t,uint8_t out[STORE_PAGE_SIZE]);
int sf_leaf_downgrade(uint32_t id,uint64_t generation,const uint8_t *,size_t,uint8_t out[STORE_PAGE_SIZE]);
/* acquire returns a fresh exclusive image, never an established image. Success
   transfers one candidate to the formatter. discard is nonfailing; callbacks
   do not reenter or jump. Result output owns no prior image. */
struct sf_image { store_io io; uint64_t identity; };
struct sf_factory { void *context; int (*acquire)(void *,struct sf_image *); void (*discard)(void *,struct sf_image); };
int sf_format_fresh(struct sf_factory,struct sf_image *);
#endif
