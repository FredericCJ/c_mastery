#ifndef CMASTERY_OWNING_OPTION_H
#define CMASTERY_OWNING_OPTION_H
#include "errors.h"
#include "ownership.h"
#include "option_types.h"
int eop_scalar_make(uint32_t value,unsigned present,EopScalar *out);
int eop_scalar_get(const EopScalar *option,uint32_t *out);
int eop_result_get(er_result result,uint32_t *out,size_t *error_offset);
int eop_create(EopOwner *empty,V4Allocator allocator,uint32_t id,int32_t value);
int eop_clone(const EopOwner *source,EopOwner *empty);
int eop_move(EopOwner *empty,EopOwner *source);
int eop_read(const EopOwner *source,uint32_t *id,int32_t *value);
int eop_drop(EopOwner *owner);
int eop_unit(EopUnit unit);
#endif
