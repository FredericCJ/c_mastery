#include "schema.h"
#include "generated/config.h"
int main(void){return ls_require(LS_WIRE_ID,LS_ABI_ID)==-1?0:1;}
