#ifndef OA_ATLAS_H
#define OA_ATLAS_H
#include <stddef.h>
#include <stdint.h>
typedef struct { char marker; int32_t number; double measure; } OaRecord;
typedef struct { OaRecord records[4]; } OaStorage;
typedef struct { _Alignas(64) double value; } OaWide;
typedef struct { const char *data; size_t length; } OaView;
typedef struct { unsigned char *data; size_t length; unsigned generation; } OaOwner;
typedef struct { void *context; void *(*get)(void *,size_t); void (*put)(void *,void *); } OaAllocator;
int oa_resize(OaOwner *,size_t,const OaAllocator *);
void oa_release(OaOwner *,const OaAllocator *);
int oa_label(char *,size_t,OaView *);
int oa_deep_copy(OaView,char *,size_t,OaView *);
int oa_equal(const OaRecord *,const OaRecord *);
int oa_encode(const OaRecord *,unsigned char *,size_t);
int oa_place(size_t,size_t,size_t,size_t,size_t,size_t *);
int oa_slot(OaStorage *,size_t,size_t,OaRecord **);
int oa_wide_admit(size_t);
int oa_decode_u32(const unsigned char *,size_t,uint32_t *);
#endif
