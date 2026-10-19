#include "owning_sum.h"
void ado_init(AdoValue *v){v->tag=ADO_EMPTY;v->data.number=0;}
void ado_drop(AdoValue *v){(void)v;}
AdoStatus ado_from_value(AdoValue *o,ADValue v){(void)o;(void)v;return ADO_UNIMPLEMENTED;}
AdoStatus ado_text(AdoValue *o,const unsigned char *p,size_t n,AdoOps ops){(void)o;(void)p;(void)n;(void)ops;return ADO_UNIMPLEMENTED;}
AdoStatus ado_copy(AdoValue *o,const AdoValue *v){(void)o;(void)v;return ADO_UNIMPLEMENTED;}
AdoStatus ado_move(AdoValue *o,AdoValue *v){(void)o;(void)v;return ADO_UNIMPLEMENTED;}
AdoStatus ado_measure(const AdoValue *v,uint64_t *o){(void)v;(void)o;return ADO_UNIMPLEMENTED;}
AdoStatus ado_number(const AdoValue *v,uint32_t *o){(void)v;(void)o;return ADO_UNIMPLEMENTED;}
