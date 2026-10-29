#include "dispatch_resolution.h"
#include <limits.h>
#ifndef DS_RESOLUTION_MUTANT
#define DS_RESOLUTION_MUTANT 0
#endif
static int subtype(unsigned child,unsigned ancestor){static const unsigned mask[4]={1,3,5,15};return (mask[child]&(1u<<ancestor))!=0;}
static void count(unsigned *p){if(*p<UINT_MAX)++*p;}
void ds_resolution_init(struct ds_resolution *m){*m=(struct ds_resolution){0};m->epoch=1;}
int ds_resolution_set(struct ds_resolution *m,unsigned l,unsigned r,ds_combine_fn method,struct ds_loaded *module){
 if(!m||l>=4||r>=4||(method&&module))return DS_INVALID;
 if(m->epoch>=31)return DS_FULL;
 struct ds_method_lease next={method,module};if(module){const struct ds_plugin *api;int status=ds_loaded_acquire(module,&api);if(status)return status;next.method=api->apply;}
 struct ds_method_lease old=m->entries[l][r];m->entries[l][r]=next;++m->epoch;if(old.module)(void)ds_loaded_release(old.module);return DS_OK;
}
void ds_resolution_clear(struct ds_resolution *m){for(unsigned l=0;l<4;++l)for(unsigned r=0;r<4;++r){struct ds_loaded *old=m->entries[l][r].module;m->entries[l][r]=(struct ds_method_lease){0};if(old)(void)ds_loaded_release(old);}m->epoch=32;}
static int resolve(const struct ds_resolution *m,unsigned l,unsigned r,struct ds_method_lease *out){
 unsigned matches=0;struct ds_method_lease chosen={0};
 for(unsigned a=0;a<4;++a)for(unsigned b=0;b<4;++b){if(!m->entries[a][b].method||!subtype(l,a)||!subtype(r,b))continue;int dominated=0;
  for(unsigned c=0;c<4;++c)for(unsigned d=0;d<4;++d)if((c!=a||d!=b)&&m->entries[c][d].method&&subtype(l,c)&&subtype(r,d)&&subtype(c,a)&&subtype(d,b))dominated=1;
  if(!dominated){chosen=m->entries[a][b];++matches;}}
 if(!matches)return DS_UNSUPPORTED;
#if DS_RESOLUTION_MUTANT != 2
 if(matches!=1)return DS_AMBIGUOUS;
#endif
 *out=chosen;return DS_OK;
}
static int invoke(struct ds_method_lease lease,int a,int b,int *out){int value,status=lease.method(a,b,&value);if(status==DS_OK)*out=value;return status;}
int ds_resolution_uncached(const struct ds_resolution *m,unsigned l,unsigned r,int a,int b,int *out){if(!m||!out||l>=4||r>=4)return DS_INVALID;struct ds_method_lease lease;int status=resolve(m,l,r,&lease);return status?status:invoke(lease,a,b,out);}
void ds_resolution_cache_init(struct ds_resolution_cache *c){*c=(struct ds_resolution_cache){0};}
static void release_entry(struct ds_resolution_entry *entry){struct ds_loaded *old=entry->lease.module;*entry=(struct ds_resolution_entry){0};if(old)(void)ds_loaded_release(old);}
void ds_resolution_cache_clear(struct ds_resolution_cache *c){release_entry(&c->entries[0]);release_entry(&c->entries[1]);c->next=0;}
int ds_resolution_call(const struct ds_resolution *m,struct ds_resolution_cache *c,unsigned l,unsigned r,int a,int b,int *out){
 if(!m||!c||!out||l>=4||r>=4)return DS_INVALID;
 for(unsigned i=0;i<2;++i){struct ds_resolution_entry *e=&c->entries[i];int valid=e->epoch==m->epoch;
#if DS_RESOLUTION_MUTANT == 1
 valid=1;
#elif DS_RESOLUTION_MUTANT == 4
 if(e->status==DS_UNSUPPORTED)valid=1;
#endif
 if(e->owner==m&&valid&&e->left==l&&e->right==r){count(&c->hits);return e->status?e->status:invoke(e->lease,a,b,out);}}
 count(&c->misses);struct ds_method_lease lease={0};int status=resolve(m,l,r,&lease);
 if(status==DS_OK&&lease.module){
#if DS_RESOLUTION_MUTANT != 3
  const struct ds_plugin *api;int acquire=ds_loaded_acquire(lease.module,&api);if(acquire)return acquire;
#else
  lease.module=NULL;
#endif
 }
 struct ds_resolution_entry *e=&c->entries[c->next];release_entry(e);*e=(struct ds_resolution_entry){m,m->epoch,l,r,status,lease};c->next^=1u;
 return status?status:invoke(lease,a,b,out);
}
