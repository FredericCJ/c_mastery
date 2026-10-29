#include "codec.h"
int sf_leaf_encode(uint32_t i,uint64_t g,const store_node *n,uint8_t out[STORE_PAGE_SIZE]){(void)i;(void)g;(void)n;(void)out;return STORE_UNIMPLEMENTED;}
int sf_leaf_decode(uint32_t i,uint64_t g,const uint8_t *b,size_t n,store_node *out){(void)i;(void)g;(void)b;(void)n;(void)out;return STORE_UNIMPLEMENTED;}
int sf_leaf_upgrade(uint32_t i,uint64_t g,const uint8_t *b,size_t n,uint8_t out[STORE_PAGE_SIZE]){(void)i;(void)g;(void)b;(void)n;(void)out;return STORE_UNIMPLEMENTED;}
int sf_leaf_downgrade(uint32_t i,uint64_t g,const uint8_t *b,size_t n,uint8_t out[STORE_PAGE_SIZE]){(void)i;(void)g;(void)b;(void)n;(void)out;return STORE_UNIMPLEMENTED;}
int sf_format_fresh(struct sf_factory f,struct sf_image *out){(void)f;(void)out;return STORE_UNIMPLEMENTED;}
