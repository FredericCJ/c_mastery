#include "readers.h"
#include <string.h>
#ifndef SR_MUTANT
#define SR_MUTANT 0
#endif
static int rd(void *ctx,uint32_t page,void *bytes){struct sr_hub *h=ctx;if(page<2||page>=66)return h->backend.read(h->backend.ctx,page,bytes);int wal=page>=34;uint32_t logical=page-(wal?34u:2u);struct sc_view view={h->backend,h->identity,h->revision,h->writer.root?h->writer.root:1u,wal};return sc_read(&h->cache,view,logical,bytes)==STORE_OK?0:1;}
static int wr(void *ctx,uint32_t page,const void *bytes){struct sr_hub *h=ctx;return h->backend.write(h->backend.ctx,page,bytes);}
static int fl(void *ctx){struct sr_hub *h=ctx;return h->backend.flush(h->backend.ctx);}
static int lock(struct sr_hub *h){return !h||!h->initialized?STORE_STATE:pthread_mutex_lock(&h->mutex)?STORE_STATE:STORE_OK;}
static int finish(struct sr_hub *h,int result){(void)pthread_mutex_unlock(&h->mutex);return result;}
int sr_init(struct sr_hub *h,store_io io,uint64_t identity){
 if(!h||!identity||!io.read||!io.write||!io.flush||io.page_count<67)return STORE_INVALID;
 memset(h,0,sizeof *h);if(pthread_mutex_init(&h->mutex,NULL))return STORE_STATE;
 h->backend=io;h->identity=identity;h->revision=1;
 int r=sc_init(&h->cache,SC_TWO_WAY);store_io shared={h,rd,wr,fl,io.page_count};
 if(!r)r=store_open(&h->writer,shared);
 if(!r)r=store_open(&h->reader,shared);
 if(r){(void)pthread_mutex_destroy(&h->mutex);return r;}
 h->initialized=1;return SR_OK;
}
int sr_publish(struct sr_hub *h,const void *key,size_t n,const void *value,size_t z){
 if(!key||!n||n>16||z>64||(!value&&z))return STORE_INVALID;
 int r=lock(h);if(r)return r;
 if(h->closing||h->poisoned)return finish(h,h->closing?SR_CLOSED:STORE_STATE);
 if(h->revision==UINT64_MAX)return finish(h,STORE_FULL);
 r=store_begin(&h->writer);if(!r)r=store_put(&h->writer,key,n,value,z);
 if(r){store_abort(&h->writer);if(h->writer.poisoned)h->poisoned=1;return finish(h,r);}
 r=store_commit(&h->writer);
 if(r){h->poisoned=1;return finish(h,r);}
 if(SR_MUTANT!=1)h->revision++;
 if(SR_MUTANT==4)for(unsigned i=0;i<2;i++)if(h->slots[i].live)(void)sl_snapshot_capture(&h->writer,&h->slots[i].snapshot);
 r=store_checkpoint(&h->writer);
 if(!r&&SR_MUTANT!=2)r=store_open(&h->reader,h->writer.io);
 if(r)h->poisoned=1;
 return finish(h,r);
}
int sr_get(struct sr_hub *h,const void *key,size_t n,void *value,size_t z,size_t *length){int r=lock(h);if(r)return r;if(h->closing||h->poisoned)return finish(h,h->closing?SR_CLOSED:STORE_STATE);return finish(h,store_get(&h->reader,key,n,value,z,length));}
int sr_capture(struct sr_hub *h,struct sr_lease *out){
 if(!out)return STORE_INVALID;
 int r=lock(h);if(r)return r;if(h->closing||h->poisoned)return finish(h,h->closing?SR_CLOSED:STORE_STATE);
 unsigned slot;for(slot=0;slot<2;slot++)if(!h->slots[slot].live&&h->slots[slot].serial<UINT64_MAX)break;
 if(slot==2)return finish(h,STORE_FULL);
 struct sr_slot *s=&h->slots[slot];r=sl_snapshot_capture(&h->reader,&s->snapshot);if(r)return finish(h,r);
 s->serial++;s->live=1;*out=(struct sr_lease){h,slot,s->serial};return finish(h,SR_OK);
}
static int valid(struct sr_lease l){return l.slot<2&&l.serial&&l.owner->slots[l.slot].live&&(SR_MUTANT==3||l.owner->slots[l.slot].serial==l.serial);}
static int compare(const void *key,size_t n,const struct sl_record *r){const uint8_t *p=key;size_t count=n<r->klen?n:r->klen;for(size_t i=0;i<count;i++){if(p[i]<r->key[i])return -1;if(p[i]>r->key[i])return 1;}return n<r->klen?-1:n>r->klen?1:0;}
int sr_snapshot_get(struct sr_lease l,const void *key,size_t n,void *value,size_t z,size_t *length){
 if(!key||!n||n>16||!length||(!value&&z))return STORE_INVALID;
 int r=lock(l.owner);if(r)return r;if(!valid(l))return finish(l.owner,SR_STALE);
 const struct sl_snapshot *s=&l.owner->slots[l.slot].snapshot;
 for(size_t i=0;i<s->count;i++)if(!compare(key,n,&s->records[i])){const struct sl_record *record=&s->records[i];if(z<record->vlen)return finish(l.owner,STORE_FULL);if(record->vlen)memcpy(value,record->value,record->vlen);*length=record->vlen;return finish(l.owner,SR_OK);}
 return finish(l.owner,STORE_NOT_FOUND);
}
int sr_release(struct sr_lease l){int r=lock(l.owner);if(r)return r;if(!valid(l))return finish(l.owner,SR_STALE);struct sr_slot *s=&l.owner->slots[l.slot];memset(&s->snapshot,0,sizeof s->snapshot);s->live=0;return finish(l.owner,SR_OK);}
int sr_close(struct sr_hub *h){int r=lock(h);if(r)return r;h->closing=1;for(unsigned i=0;i<2;i++)if(h->slots[i].live)return finish(h,SR_BUSY);return finish(h,SR_OK);}
int sr_destroy(struct sr_hub *h){if(!h||!h->initialized||!h->closing||h->slots[0].live||h->slots[1].live)return STORE_STATE;if(pthread_mutex_destroy(&h->mutex))return STORE_STATE;h->initialized=0;return SR_OK;}
