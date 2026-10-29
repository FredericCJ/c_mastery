#ifndef CMASTERY_OWNING_OPTION_H
#define CMASTERY_OWNING_OPTION_H
#include "errors.h"
#include "ownership.h"
#include "option_types.h"
int eop_scalar_make(uint32_t value, unsigned present, struct eop_scalar *out);
int eop_scalar_get(const struct eop_scalar *option, uint32_t *out);
int eop_result_get(er_result result, uint32_t *out, size_t *error_offset);
int eop_create(struct eop_owner *empty, struct v4_allocator allocator,
               uint32_t id, int32_t value);
int eop_clone(const struct eop_owner *source, struct eop_owner *empty);
int eop_move(struct eop_owner *empty, struct eop_owner *source);
int eop_read(const struct eop_owner *source, uint32_t *id, int32_t *value);
int eop_drop(struct eop_owner *owner);
int eop_unit(enum eop_unit unit);
#endif
