/* Incomplete learner policy. Complete the contract, not just these markers. */
#include "amp.h"
int af_elapsed(uint32_t a,uint32_t b,uint32_t c,unsigned*d){(void)a;(void)b;(void)c;(void)d;return AF_UNIMPLEMENTED;}
int af_boot_begin(AfBootOwner*a,uint32_t b,uint32_t c,uint32_t d,unsigned e,unsigned f){(void)a;(void)b;(void)c;(void)d;(void)e;(void)f;return AF_UNIMPLEMENTED;}
int af_boot_stage(AfBootOwner*a,AfBoot*b,unsigned c,uint32_t d,uint32_t e){(void)a;(void)b;(void)c;(void)d;(void)e;return AF_UNIMPLEMENTED;}
int af_boot_ack(AfBoot*a,uint32_t b,uint32_t c,uint32_t d){(void)a;(void)b;(void)c;(void)d;return AF_UNIMPLEMENTED;}
int af_boot_close(AfBootOwner*a,AfBoot*b,unsigned c,unsigned d){(void)a;(void)b;(void)c;(void)d;return AF_UNIMPLEMENTED;}
int af_boot_status(const AfBootOwner*a,const AfBoot*b,uint32_t c){(void)a;(void)b;(void)c;return AF_UNIMPLEMENTED;}
void af_ring_init(AfRing*q,uint32_t initial){atomic_init(&q->head,initial);atomic_init(&q->tail,initial);}
uint32_t af_queued(const AfRing*q){(void)q;return 0;}
int af_batch(AfRing*a,uint32_t b,const AfMessage*c,unsigned d){(void)a;(void)b;(void)c;(void)d;return AF_UNIMPLEMENTED;}
int af_receive(AfRing*a,uint32_t b,void*c,size_t d,uint32_t*e,size_t*f){(void)a;(void)b;(void)c;(void)d;(void)e;(void)f;return AF_UNIMPLEMENTED;}
int af_cache_span(uint32_t a,uint32_t b,uint32_t c,uint32_t*d,uint32_t*e){(void)a;(void)b;(void)c;(void)d;(void)e;return AF_UNIMPLEMENTED;}
int af_effect(AfReceipt*a,uint32_t b,uint32_t c,int32_t d,int32_t*e){(void)a;(void)b;(void)c;(void)d;(void)e;return AF_UNIMPLEMENTED;}
int af_region(const AfBank*a,size_t b,uint32_t c,uint32_t d,unsigned e,unsigned*f){(void)a;(void)b;(void)c;(void)d;(void)e;(void)f;return AF_UNIMPLEMENTED;}
int af_owner_claim(AfOwners*a,unsigned b,unsigned c){(void)a;(void)b;(void)c;return AF_UNIMPLEMENTED;}
