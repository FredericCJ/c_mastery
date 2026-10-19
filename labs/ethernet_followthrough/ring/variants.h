#ifndef ETHERNET_RING_VARIANTS_H
#define ETHERNET_RING_VARIANTS_H
#include "ring.h"
/* Separate changed-condition models; not the selected hardware ring format. */
typedef struct {const unsigned char *borrow;size_t length;uint32_t token;unsigned count,prepared,owned,aborting;} EvChain;
int ev_prepare(EvChain *,const unsigned char *,size_t,unsigned count,uint32_t token);
int ev_own(EvChain *,unsigned index);
int ev_complete(EvChain *,unsigned returned,unsigned error);
int ev_abort(EvChain *,unsigned stopped);
/* Stored bytes have already been validated and copied from a declared capture
 * boundary. This does not infer changed-policy RDES0 FL from a guessed value. */
int ev_span(const unsigned char *,size_t stored,unsigned cstf,size_t *exposed);
int ev_order(const unsigned char events[8],uint32_t *violated_edges);
typedef struct {unsigned char cpu[64],device[64];unsigned dirty,device_owned;} EvVisibility;
int ev_store(EvVisibility *,unsigned index,unsigned char value);
int ev_clean(EvVisibility *,unsigned line);
int ev_publish(EvVisibility *,unsigned line);
int ev_device_store(EvVisibility *,unsigned index,unsigned char value);
int ev_return(EvVisibility *,unsigned line,unsigned returned);
int ev_invalidate(EvVisibility *,unsigned line);
#endif
