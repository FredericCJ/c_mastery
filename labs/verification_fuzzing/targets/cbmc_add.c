/* Host verification tool; not a target runtime dependency. */
#include "core.h"
#include <assert.h>
extern uint32_t nondet_uint32_t(void);
int main(void) {uint32_t a=nondet_uint32_t(),b=nondet_uint32_t(),out=77;
 int ok=vf_add(a,b,&out); uint64_t exact=(uint64_t)a+b;
 assert(ok==(exact<=UINT32_MAX));
 if(ok)assert(out==exact);else assert(out==77);
 return 0;
}
