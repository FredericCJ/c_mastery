#ifndef STORAGE_FOLLOW_CACHE_H
#define STORAGE_FOLLOW_CACHE_H
#include "store.h"
enum {SC_DIRECT=0,SC_TWO_WAY=1,SC_PIN_ROOT=2};
typedef struct {int valid;uint64_t identity,generation;uint32_t physical;uint8_t bytes[STORE_PAGE_SIZE];} ScEntry;
typedef struct {ScEntry entries[4];unsigned recent[2],policy;uint64_t reads,hits;} ScCache;
typedef struct {store_io io;uint64_t identity,generation;uint32_t root;int wal;} ScView;
/* Fresh caller storage; externally serialized. View is a trusted snapshot of
   current backend authority. It must be refreshed after every publication.
   Cache/output/backend/callback controls are disjoint and callbacks nonreentrant.
   No dirty write API: transaction scratch remains in the original store. */
int sc_init(ScCache *,unsigned policy);
int sc_read(ScCache *,ScView,uint32_t logical,uint8_t out[STORE_PAGE_SIZE]);
#endif
