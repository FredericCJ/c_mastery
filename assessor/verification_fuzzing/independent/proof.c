#include "audit.h"
#include <assert.h>
extern unsigned nondet_uint(void);extern unsigned char nondet_uchar(void);
int main(void){unsigned char input[22];for(unsigned i=0;i<22;i++)input[i]=nondet_uchar();unsigned n=nondet_uint();__CPROVER_assume(n<=22);AuditState s;s.present=nondet_uchar();__CPROVER_assume(s.present<=15);for(unsigned i=0;i<4;i++)s.value[i]=nondet_uchar();AuditState before=s;int ok=audit_packet(input,n,&s);assert(s.present<=15);if(!ok){assert(s.present==before.present);for(unsigned i=0;i<4;i++)assert(s.value[i]==before.value[i]);}return 0;}
