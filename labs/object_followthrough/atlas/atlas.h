#ifndef OA_ATLAS_H
#define OA_ATLAS_H
#include <stddef.h>
#include <stdint.h>
struct oa_record { char marker; int32_t number; double measure; };
struct oa_storage { struct oa_record records[4]; };
struct oa_wide { _Alignas(64) double value; };
struct oa_view { const char *data; size_t length; };
struct oa_owner { unsigned char *data; size_t length; unsigned generation; };
struct oa_allocator { void *context; void *(*get)(void *,size_t); void (*put)(void *,void *); };
int oa_resize(struct oa_owner *,size_t,const struct oa_allocator *);
void oa_release(struct oa_owner *,const struct oa_allocator *);
int oa_label(char *,size_t,struct oa_view *);
int oa_deep_copy(struct oa_view,char *,size_t,struct oa_view *);
int oa_equal(const struct oa_record *,const struct oa_record *);
int oa_encode(const struct oa_record *,unsigned char *,size_t);
int oa_place(size_t,size_t,size_t,size_t,size_t,size_t *);
int oa_slot(struct oa_storage *,size_t,size_t,struct oa_record **);
int oa_wide_admit(size_t);
int oa_decode_u32(const unsigned char *,size_t,uint32_t *);
#endif
