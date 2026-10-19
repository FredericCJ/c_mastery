#ifndef EF_FOUNDATION_H
#define EF_FOUNDATION_H
#include "com_edu7.h"
#include "edu7_depth.h"
enum { EF_OK, EF_INVALID, EF_BUSY, EF_STALE, EF_FULL, EF_DROP, EF_STUB=99 };
typedef struct { unsigned canonical,session,count,last,transport,network,body,wire; } EfBudget;
typedef struct {
  EduDatagram packet;
  const uint8_t *borrow;
  uint32_t generation;
  unsigned active,mode,egress,next;
} EfRoute;
typedef struct { EduFramer legacy; uint8_t bytes[47]; unsigned length,active,escape,discard; } EfFramer;
typedef struct { EduRecord base; int64_t value; uint8_t extra[8],extra_length; } EfWide;
int ef_budget(unsigned schema,unsigned extra,EfBudget *);
int ef_enqueue(EfRoute *,const uint8_t *,size_t,unsigned ingress,unsigned copy,unsigned version,uint32_t *);
int ef_peek(const EfRoute *,uint32_t,EduFrame *);
int ef_release(EfRoute *,uint32_t);
int ef_local(const uint8_t *,size_t,unsigned node,EduDatagram *);
int ef_encode(const EduRecord *,uint8_t *,size_t,size_t *);
int ef_wide_encode(const EfWide *,uint8_t *,size_t,size_t *);
int ef_wide_decode(const uint8_t *,size_t,EfWide *);
int ef_frame_encode(const EduFrame *,unsigned version,uint8_t *,size_t,size_t *);
int ef_frame_byte(EfFramer *,unsigned version,uint8_t,unsigned fault,EduFrame *);
#endif
