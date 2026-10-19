#include "object_runtime_alternatives.h"
ORStatus or_dictionary_set(ORDictionary *d,uint8_t key,int32_t value){(void)d;(void)key;(void)value;return OR_UNIMPLEMENTED;}
ORStatus or_dictionary_delete(ORDictionary *d,uint8_t key){(void)d;(void)key;return OR_UNIMPLEMENTED;}
ORStatus or_dictionary_get(const ORDictionary *d,uint8_t key,int32_t *out){(void)d;(void)key;(void)out;return OR_UNIMPLEMENTED;}
ORStatus or_property_generic(const ORShapes *s,const ORObject *o,uint8_t key,int32_t *out){(void)s;(void)o;(void)key;(void)out;return OR_UNIMPLEMENTED;}
