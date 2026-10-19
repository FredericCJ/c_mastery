#include "registry.h"
static void drop(void *ctx) {
 AsRegistration *slot=ctx;ApiDrop release=slot->drop;void *context=slot->context;
 slot->occupied=slot->enabled=0;slot->callback=0;slot->context=0;slot->drop=0;
 if(release)release(context);
}
static void dispatch(void *ctx,ApiRegistry *core,ApiHandle handle) {
 (void)core;AsRegistration *slot=ctx;
 if(slot->enabled)slot->callback(slot->context,slot->owner,handle);
}
void as_registry_init(AsRegistry *r,unsigned suppression,AsTerminal terminal,void *ctx) {
 if(!r)return;
 api_registry_init(&r->core);r->suppress=!!suppression;r->terminal=terminal;r->terminal_context=ctx;
 for(unsigned i=0;i<4;++i)r->registrations[i]=(AsRegistration){0};
}
ApiStatus as_register(AsRegistry *r,AsCallback cb,void *ctx,ApiDrop release,ApiHandle *out) {
 if(!r||!cb||!out)return API_INVALID;
 for(unsigned i=0;i<4;++i)if(!r->registrations[i].occupied) {
  AsRegistration *slot=r->registrations+i;
  *slot=(AsRegistration){r,cb,ctx,release,{0,0},1,1};
  ApiStatus status=api_add(&r->core,dispatch,slot,drop,&slot->handle);
  if(status!=API_OK){slot->occupied=0;return status;}
  *out=slot->handle;return API_OK;
 }
 return API_FULL;
}
ApiStatus as_remove(AsRegistry *r,ApiHandle h) {
 if(!r)return API_INVALID;
 ApiStatus result=api_remove(&r->core,h);
 if(result==API_PENDING&&r->suppress)
  for(unsigned i=0;i<4;++i)if(r->registrations[i].occupied&&
   r->registrations[i].handle.index==h.index&&r->registrations[i].handle.generation==h.generation)
    r->registrations[i].enabled=0;
 return result;
}
static void terminal_if_closed(AsRegistry *r) {
 if(r->core.closed&&r->terminal) {
  AsTerminal terminal=r->terminal;void *ctx=r->terminal_context;
  r->terminal=0;r->terminal_context=0;
  terminal(ctx,r); /* Terminal receiver can free r. No access afterward. */
 }
}
ApiStatus as_dispatch(AsRegistry *r) {
 if(!r)return API_INVALID;
 ApiStatus status=api_dispatch(&r->core);terminal_if_closed(r);return status;
}
ApiStatus as_destroy(AsRegistry *r) {
 if(!r)return API_INVALID;
 ApiStatus status=api_destroy(&r->core);terminal_if_closed(r);return status;
}
