#ifndef OWNERSHIP_AUTHORITY_BOUNDARY_H
#define OWNERSHIP_AUTHORITY_BOUNDARY_H
#include "ownership.h"
#include <stddef.h>
#include <stdint.h>
enum { OAF_READ=1, OAF_WRITE=2, OAF_WIRE_SIZE=16 };
typedef enum { OAF_OK, OAF_INVALID, OAF_DENIED, OAF_STALE, OAF_FULL } OafStatus;
/* Trusted, externally serialized, live and noncopied authority. Init is only
   for fresh storage. Arena and root outlive every view. Native C fields are
   not protected against arbitrary same-process writes. */
typedef struct { OwTenantArena *arena; uint32_t epoch; unsigned tenant,rights,retired; } OafRoot;
typedef struct { OafRoot *root; uint32_t epoch,offset,count; unsigned rights; } OafView;
/* Every field is untrusted. The caller/server selects the trusted parent
   out of band; a request never supplies a trusted native pointer. */
typedef struct { uint32_t epoch,offset,count; unsigned tenant,rights; } OafRequest;
OafStatus oaf_init(OafRoot *,OwTenantArena *,unsigned tenant,unsigned rights);
OafStatus oaf_view(OafRoot *,OafView *);
OafStatus oaf_subview(const OafView *,uint32_t offset,uint32_t count,unsigned rights,OafView *);
OafStatus oaf_revoke(OafRoot *);
OafStatus oaf_encode(OafRequest,unsigned char out[OAF_WIRE_SIZE]);
OafStatus oaf_decode(const unsigned char *,size_t,OafRequest *);
OafStatus oaf_write(const OafView *,OafRequest,int32_t);
OafStatus oaf_read(const OafView *,OafRequest,int32_t out[8]);
OafStatus oaf_wire_write(const OafView *,const unsigned char *,size_t,int32_t);
OafStatus oaf_wire_read(const OafView *,const unsigned char *,size_t,int32_t out[8]);
#endif
