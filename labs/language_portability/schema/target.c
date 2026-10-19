#include "schema.h"
#include "generated/config.h"
volatile unsigned ls_observed;
int ls_target_main(void){LsFields f={5,17,2},g;uint16_t w=0;for(unsigned i=0;i<32;++i){if(ls_pack(&f,&w) || w!=653 || ls_unpack(w,&g) || g.tag!=5 || g.length!=17 || g.flags!=2 || ls_require(LS_WIRE_ID,LS_ABI_ID))return 1;ls_observed+=w;}return 0;}
