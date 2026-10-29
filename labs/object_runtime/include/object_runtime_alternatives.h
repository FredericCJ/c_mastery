#ifndef CMASTERY_OBJECT_RUNTIME_ALTERNATIVES_H
#define CMASTERY_OBJECT_RUNTIME_ALTERNATIVES_H
#include "object_runtime.h"

struct or_dictionary
{
    uint8_t count, key[4];
    int32_t value[4];
};
enum or_status or_dictionary_set(struct or_dictionary *, uint8_t key,
                                 int32_t value);
enum or_status or_dictionary_delete(struct or_dictionary *, uint8_t key);
enum or_status or_dictionary_get(const struct or_dictionary *, uint8_t key,
                                 int32_t *out);
enum or_status or_property_generic(const struct or_shapes *,
                                   const struct or_object *, uint8_t key,
                                   int32_t *out);
#endif
