#include "object_runtime_vm.h"
void orv_init(ORVM *o,Cvm *v){(void)o;(void)v;}
ORStatus orv_pause(ORVM *o,int p){(void)o;(void)p;return OR_UNIMPLEMENTED;}
ORStatus orv_register(ORVM *o,ORVDerived *r,uint64_t *t){(void)o;(void)r;(void)t;return OR_UNIMPLEMENTED;}
ORStatus orv_unregister(ORVM *o,uint64_t t){(void)o;(void)t;return OR_UNIMPLEMENTED;}
ORStatus orv_collect(ORVM *o,unsigned b,unsigned f,ORVStats *s){(void)o;(void)b;(void)f;(void)s;return OR_UNIMPLEMENTED;}
ORStatus orv_pair(ORVM *o,ORVDerived *a,ORVDerived *b,int c,unsigned f,CvmValue *v){(void)o;(void)a;(void)b;(void)c;(void)f;(void)v;return OR_UNIMPLEMENTED;}
ORStatus orv_call(ORVM *o,CvmValue a,CvmValue b,ORVForeign f,void *c,CvmValue *v){(void)o;(void)a;(void)b;(void)f;(void)c;(void)v;return OR_UNIMPLEMENTED;}
