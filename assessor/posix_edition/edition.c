#include "edition.h"
int pe_close(enum PEVersion v,enum PECall c,unsigned r,enum PEOutcome o,PEDecision *out){
 PEDecision d={PE_RETIRED,o!=PE_OK};
 if(!out||r>1||v<PE_2017||v>PE_2024||c<PE_CLOSE||c>PE_PC_INVALID||o<PE_OK||o>PE_AGAIN)return 1;
 if(v!=PE_2024&&c!=PE_CLOSE)return 1;
 if(o==PE_AGAIN)return 1; /* Out of the selected old/Linux outcome set too. */
 if(v==PE_2024){
  if(o==PE_INTR){
   if(!r||c==PE_PC_ZERO||c==PE_PC_INVALID)return 1;
   d.action=PE_RETRY_CLOSE_ONLY;
  }
  if(o==PE_INVAL&&c!=PE_PC_INVALID)return 1;
 }else{
  if(o==PE_PROGRESS||o==PE_INVAL)return 1;
  if(v==PE_2017&&(o==PE_INTR||o==PE_IO))d.action=PE_UNRESOLVED;
 }
 if(o==PE_BADF)d.action=PE_INVALID_HANDLE;
#ifdef PE_MUTANT_RETRY
 if(o==PE_INTR)d.action=PE_RETRY_CLOSE_ONLY;
#endif
#ifdef PE_MUTANT_RETAIN
 if(o==PE_IO||o==PE_PROGRESS)d.action=PE_RETRY_CLOSE_ONLY;
#endif
#ifdef PE_MUTANT_ZERO
 if(v==PE_2024&&o==PE_OK)d.diagnostic=1;
#endif
 *out=d;return 0;
}
int pe_read(unsigned init,unsigned sw,unsigned ow,unsigned sr,unsigned waiting,unsigned tps,unsigned pri,enum PERead *out){
 enum PERead d;
 if(!out||init>1||sw>1||ow>1||sr>1||waiting>1||tps>1||pri>1||
    (sw&&(ow||sr))||(ow&&sr)||(pri&&!waiting))return 1;
 if(!init)d=PE_UNDEFINED;
 else if(sw)d=PE_MAY_DEADLOCK;
 else if(ow)d=PE_BLOCK;
 else if(!waiting)d=PE_ACQUIRE;
 else if(!tps)d=PE_IMPL_CHOICE;
 else if(sr||!pri)d=PE_ACQUIRE;
 else d=PE_BLOCK;
#ifdef PE_MUTANT_PRIORITY
 if(init&&!sw&&!ow&&tps&&waiting&&pri)d=PE_BLOCK;
#endif
#ifdef PE_MUTANT_FAIR
 if(init&&!sw&&!ow&&!tps&&waiting)d=PE_BLOCK;
#endif
 *out=d;return 0;
}
