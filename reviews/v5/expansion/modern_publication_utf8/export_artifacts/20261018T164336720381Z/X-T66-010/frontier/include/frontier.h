#ifndef CMASTERY_OWNERSHIP_FRONTIER_H
#define CMASTERY_OWNERSHIP_FRONTIER_H
#include "ownership.h"
typedef enum { OCF_OK, OCF_INVALID, OCF_NOMEM, OCF_INIT_FAILED, OCF_NOT_IMPLEMENTED } OcfStatus;
typedef struct { const int32_t *block[4]; size_t length[4]; unsigned initialized; } OcfView;
typedef int (*OcfInitialize)(void *,unsigned,int32_t *,size_t,const OcfView *);
typedef void (*OcfDestroy)(void *,unsigned,int32_t *,size_t,const OcfView *);
typedef struct { void *context; OcfInitialize initialize; OcfDestroy destroy; } OcfCallbacks;
typedef struct { unsigned order[4]; size_t length[4]; } OcfPlan;
typedef struct {
 int32_t *block[4]; size_t length[4]; unsigned order[4];
 unsigned count,acquired,initialized;
 V4Allocator allocator; OcfCallbacks callbacks;
} OcfOwner;
/* Fresh zero owner. IDs A=0 B=1 C=2 D=3. A precedes B/C; B precedes D.
 * See FRONTIER.md for authentic controls, callback and partial-init obligations. */
OcfStatus ocf_construct(OcfOwner *,V4Allocator,OcfCallbacks,const OcfPlan *);
OcfStatus ocf_destroy(OcfOwner *);
#endif
