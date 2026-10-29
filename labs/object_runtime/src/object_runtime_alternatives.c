#include "object_runtime_alternatives.h"
enum or_status or_dictionary_set(struct or_dictionary *d,uint8_t key,int32_t value){(void)d;(void)key;(void)value;return OR_UNIMPLEMENTED;}
enum or_status or_dictionary_delete(struct or_dictionary *d,uint8_t key){(void)d;(void)key;return OR_UNIMPLEMENTED;}
enum or_status or_dictionary_get(const struct or_dictionary *d,uint8_t key,int32_t *out){(void)d;(void)key;(void)out;return OR_UNIMPLEMENTED;}
enum or_status or_property_generic(const struct or_shapes *s,const struct or_object *o,uint8_t key,int32_t *out){(void)s;(void)o;(void)key;(void)out;return OR_UNIMPLEMENTED;}
