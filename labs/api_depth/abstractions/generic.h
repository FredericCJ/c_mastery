#ifndef CMASTERY_ABSTRACTION_GENERIC_H
#define CMASTERY_ABSTRACTION_GENERIC_H
#include "session.h"
typedef enum { AV_INT,AV_TEXT,AV_ERROR,AV_OWNED } AvTag;
typedef struct { const unsigned char *data;size_t length; } AvText;
typedef struct { int code;size_t offset; } AvError;
typedef struct { unsigned char *data;size_t length;AcAllocate allocator; } AvOwned;
typedef struct {
 AvTag tag;
 union { int integer;AvText text;AvError error;AvOwned owned; } as;
} AsValue;
AsStatus av_text(const unsigned char *,size_t,AsValue *);
AsStatus av_owned(const unsigned char *,size_t,AcAllocate,AsValue *);
AsStatus av_clone(const AsValue *,AsValue *);
AsStatus av_move(AsValue *,AsValue *,int fail_before);
AsStatus av_drop(AsValue *);
AsStatus av_valid(const AsValue *);
AsValue av_parse(const unsigned char *,size_t);
AsStatus av_validate_fields(const AvText *,size_t,AsValue results[4]);
typedef struct { int values[4];size_t count; } AsInts;
AsStatus as_int_push(AsInts *,int);
#define AS_GENERATE_PUSH(Name,Type) \
 typedef struct { Type values[4];size_t count; } Name; \
 static inline AsStatus Name##_push(Name *b,Type value) { \
  if(!b||b->count>4)return AS_INVALID; \
  if(b->count==4)return AS_FULL; \
  b->values[b->count++]=value;return AS_OK; }
AS_GENERATE_PUSH(AsGenerated,int)
#define as_generic_push(buffer,value) _Generic((value),int:as_int_push)((buffer),(value))
typedef AsStatus (*AsCopyElement)(void *,const void *,void *);
typedef void (*AsDropElement)(void *,void *);
typedef struct { size_t size,alignment;AsCopyElement copy;AsDropElement drop;void *context; } AsElements;
typedef struct { void *data;size_t count,capacity;AsElements elements; } AsErased;
/* Actual caller storage and stage must be arrays of the descriptor's real type,
   mutually disjoint from source and each other. Alignment/size facts are caller
   supplied capabilities, not proof that an arbitrary address is valid. */
AsStatus as_erased_append(AsErased *,const void *,size_t,void *stage,
                         size_t stage_capacity,size_t storage_alignment);
AsStatus as_erased_clear(AsErased *);
AsStatus as_value_copy_element(void *,const void *,void *);
void as_value_drop_element(void *,void *);
AsStatus as_int_copy_element(void *,const void *,void *);
void as_int_drop_element(void *,void *);
#endif
