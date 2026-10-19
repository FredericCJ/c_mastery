#include "schema.h"
#include "generated/config.h"
#include "generated/codec.h"
int main(void){LsFields f={5,17,2},g;uint16_t w,again;if(ls_require(LS_WIRE_ID,LS_ABI_ID)||ls_pack(&f,&w)||lsg_unpack(w,&g)||lsg_pack(&g,&again)||again!=653)return 1;return 0;}
