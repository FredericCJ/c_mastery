#include "readers.h"
#define UNUSED(x) (void)(x)
int sr_init(struct sr_hub *h,store_io io,uint64_t id){UNUSED(h);UNUSED(io);UNUSED(id);return STORE_UNIMPLEMENTED;}
int sr_publish(struct sr_hub *h,const void *k,size_t n,const void *v,size_t z){UNUSED(h);UNUSED(k);UNUSED(n);UNUSED(v);UNUSED(z);return STORE_UNIMPLEMENTED;}
int sr_get(struct sr_hub *h,const void *k,size_t n,void *v,size_t z,size_t *l){UNUSED(h);UNUSED(k);UNUSED(n);UNUSED(v);UNUSED(z);UNUSED(l);return STORE_UNIMPLEMENTED;}
int sr_capture(struct sr_hub *h,struct sr_lease *l){UNUSED(h);UNUSED(l);return STORE_UNIMPLEMENTED;}
int sr_snapshot_get(struct sr_lease l,const void *k,size_t n,void *v,size_t z,size_t *r){UNUSED(l);UNUSED(k);UNUSED(n);UNUSED(v);UNUSED(z);UNUSED(r);return STORE_UNIMPLEMENTED;}
int sr_release(struct sr_lease l){UNUSED(l);return STORE_UNIMPLEMENTED;}
int sr_close(struct sr_hub *h){UNUSED(h);return STORE_UNIMPLEMENTED;}
int sr_destroy(struct sr_hub *h){UNUSED(h);return STORE_UNIMPLEMENTED;}
