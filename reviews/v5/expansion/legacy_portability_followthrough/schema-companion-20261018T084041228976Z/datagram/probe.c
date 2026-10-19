#include "schema.h"
#include "generated/config.h"
#include "generated/fields.h"
#include <stdio.h>
int main(void){LsFields f={5,17,2},g;uint16_t w; if(ls_pack(&f,&w)||w!=653||ls_unpack(w,&g)||g.tag!=5||g.length!=17||g.flags!=2||ls_require(LS_WIRE_ID,LS_ABI_ID))return 1;printf("%u %u %u %zu\n",(unsigned)LS_TAG_MASK,(unsigned)LS_LENGTH_MASK,(unsigned)LS_FLAGS_MASK,ls_record_size());return 0;}
