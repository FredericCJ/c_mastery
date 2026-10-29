#include "transfer.h"
#include <assert.h>
uint32_t nondet_uint32_t(void);int64_t nondet_int64_t(void);int32_t nondet_int32_t(void);unsigned nondet_uint(void);
int main(void){
#if defined(CHECK_SEARCH)
 struct vt_record a[6];unsigned n=nondet_uint();__CPROVER_assume(n<=6);for(unsigned i=0;i<n;i++){a[i].key=nondet_int32_t();a[i].identity=i;if(i)__CPROVER_assume(a[i-1].key>=a[i].key);}int32_t key=nondet_int32_t();size_t p=vt_desc(a,n,key);assert(p<=n);for(size_t i=0;i<p;i++)assert(a[i].key>key);for(size_t i=p;i<n;i++)assert(a[i].key<=key);
#else
 uint32_t a=nondet_uint32_t(),b=nondet_uint32_t(),out=91;int ok=vt_mul(a,b,&out);uint64_t wide=(uint64_t)a*b;assert(ok==(b==0||a<=UINT32_MAX/b));assert(ok?out==(uint32_t)wide:out==91);int64_t v=nondet_int64_t();uint16_t small=91;ok=vt_narrow(v,&small);assert(ok==(v>=0&&v<=UINT16_MAX));assert(ok?small==(uint64_t)v:small==91);
#endif
 return 0;}
