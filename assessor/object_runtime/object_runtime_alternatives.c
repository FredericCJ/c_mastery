#include "object_runtime_alternatives.h"
#ifndef ORA_MUTANT
#define ORA_MUTANT 0
#endif
static int valid_dictionary(const ORDictionary *d) { return d && d->count <= 4; }
ORStatus or_dictionary_set(ORDictionary *d, uint8_t key, int32_t value) {
 unsigned i;
 if (!valid_dictionary(d) || !key) return OR_INVALID;
 if (ORA_MUTANT == 1 && d->count == 4) return OR_FULL;
 for (i=0;i<d->count;i++) if(d->key[i]==key){d->value[i]=value;return OR_OK;}
 if (d->count==4) return OR_FULL;
 d->key[d->count]=key;d->value[d->count]=value;d->count++;return OR_OK;
}
ORStatus or_dictionary_delete(ORDictionary *d,uint8_t key) {
 unsigned i;
 if (!valid_dictionary(d) || !key) return OR_INVALID;
 for(i=0;i<d->count;i++)if(d->key[i]==key)break;
 if(i==d->count)return OR_MISS;
 for(;i+1<d->count;i++){d->key[i]=d->key[i+1];if(ORA_MUTANT!=2)d->value[i]=d->value[i+1];}
 d->count--;d->key[d->count]=0;d->value[d->count]=0;return OR_OK;
}
ORStatus or_dictionary_get(const ORDictionary *d,uint8_t key,int32_t *out){
 unsigned i;if(!valid_dictionary(d)||!key||!out)return OR_INVALID;
 for(i=0;i<d->count;i++)if(d->key[i]==key){*out=d->value[i];return OR_OK;}
 return OR_MISS;
}
ORStatus or_property_generic(const ORShapes *s,const ORObject *o,uint8_t key,int32_t *out){
 unsigned i,n;const ORShape *shape;
 if(!s||!o||!out||!key||!s->used||s->used>16||o->shape>=s->used)return OR_INVALID;
 shape=&s->shape[ORA_MUTANT==3?1:o->shape];n=shape->count;
 if(n>4)return OR_INVALID;
 for(i=0;i<n;i++)if(shape->keys[i]==key){*out=o->fields[i];return OR_OK;}
 if(ORA_MUTANT==4)*out=0;
 return OR_MISS;
}
