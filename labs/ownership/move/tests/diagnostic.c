#include "move.h"
OwOwner permitted_struct_copy(OwOwner source){return source;}
void ignored_result(OwOwner *old,OmmValue *value){
#ifdef OMM_DIAGNOSTIC
 (void)value;ow_consume(old);
#else
 (void)old;omm_consume(value);
#endif
}
