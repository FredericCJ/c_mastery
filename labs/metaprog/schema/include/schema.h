#ifndef CMASTERY_MPS_SCHEMA_H
#define CMASTERY_MPS_SCHEMA_H
#include <stddef.h>
#include <stdint.h>
#include "schema_fields.h"
#include "schema_stamp.h"
enum { MPS_OK,MPS_INVALID,MPS_RANGE,MPS_FULL,MPS_STOP };
enum { MPS_U16,MPS_U32,MPS_S32 };
#define MPS_DECLARE(i,name,type,tag,width,required,kind,lo,hi,since) type name;
typedef struct { uint32_t present;MPS_FIELDS(MPS_DECLARE) } MpsRecord;
#undef MPS_DECLARE
#define MPS_COUNT(i,name,type,tag,width,required,kind,lo,hi,since) +1
enum { MPS_FIELD_COUNT=0 MPS_FIELDS(MPS_COUNT),MPS_WIRE_CAPACITY=64 };
#undef MPS_COUNT
typedef struct { const char *name;size_t offset,native_size;unsigned index,tag,width,required,kind,since;int64_t minimum,maximum; } MpsField;
typedef int (*MpsVisitor)(void *,const MpsField *,int64_t);
void MPS_LINK_GUARD(void);
size_t mps_fields(const MpsField **);
int mps_validate(const MpsRecord *);
int mps_equal(const MpsRecord *,const MpsRecord *,unsigned *);
int mps_encode(unsigned,const MpsRecord *,unsigned char *,size_t,size_t *);
int mps_decode(const unsigned char *,size_t,MpsRecord *);
int mps_visit(const MpsRecord *,MpsVisitor,void *,size_t *);
#endif
