#include "opaque.h"
struct EfrOpaque {er_fraction value;EfrMemory memory;};
er_code efr_opaque_make(EfrMemory memory,uint32_t n,uint32_t d,EfrOpaque **out){
 if(!out||*out||!memory.take||!memory.release)return ER_INVALID;
 er_fraction value;if(er_refine(n,d,&value)!=ER_OK)return ER_INVALID;
 EfrOpaque *p=memory.take(memory.context,sizeof *p);if(!p)return ER_IO;
 p->value=value;p->memory=memory;*out=p;return ER_OK;
}
er_code efr_opaque_consume(const EfrOpaque *p,uint32_t *out){
 if(!p||!out)return ER_INVALID;
 *out=p->value.numerator/p->value.denominator;return ER_OK;
}
void efr_opaque_drop(EfrOpaque **owner){
 EfrOpaque *p=*owner;if(!p)return;
 EfrMemory memory=p->memory;*owner=NULL;memory.release(memory.context,p);
}
