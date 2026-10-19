#include "recovery.h"
#ifndef IA_MUTANT
#define IA_MUTANT 0
#endif
int ia_clear_begin(PiRecovery *s,PiLines io,unsigned exclusive,unsigned others,
                    uint32_t now,unsigned budget,unsigned half){
 if(!s || !io.read || !io.release || exclusive>1 || others>1 || !half ||
    half>=UINT32_C(0x40000000) || budget<=half || budget>=UINT32_C(0x80000000))return PI_INVALID;
 if(!exclusive || others)return PI_FORBIDDEN;
 if(s->active)return PI_BUSY;
 if(IA_MUTANT!=5 && io.read(io.context,PI_SCL) && io.read(io.context,PI_SDA)){
  PiRecovery empty={0};empty.result=PI_OK;*s=empty;return PI_OK;
 }
 return pi_recover_begin(s,io,exclusive,others,now,budget,half);
}
