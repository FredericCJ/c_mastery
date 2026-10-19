#ifndef AF_LEDGER_H
#define AF_LEDGER_H
#include "amp.h"
enum { AF_ACCEPTED=1,AF_DONE,AF_DROPPED,AF_UNCERTAIN };
typedef struct { uint32_t epoch,state; } AfRecord;
typedef struct { uint32_t epoch,last;AfRecord item[128]; } AfLedger;
int af_admit(AfLedger *,uint32_t,uint32_t);
int af_terminal(AfLedger *,uint32_t,uint32_t,unsigned);
int af_ledger_restart(AfLedger *,unsigned,unsigned);
#endif
