#ifndef CMASTERY_OBJECT_RUNTIME_ALTERNATIVES_H
#define CMASTERY_OBJECT_RUNTIME_ALTERNATIVES_H
#include "object_runtime.h"
typedef struct { uint8_t count, key[4]; int32_t value[4]; } ORDictionary;
ORStatus or_dictionary_set(ORDictionary *, uint8_t key, int32_t value);
ORStatus or_dictionary_delete(ORDictionary *, uint8_t key);
ORStatus or_dictionary_get(const ORDictionary *, uint8_t key, int32_t *out);
ORStatus or_property_generic(const ORShapes *, const ORObject *, uint8_t key, int32_t *out);
#endif
