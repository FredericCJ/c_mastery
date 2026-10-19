#include "declarations.h"
#include "table.h"
#include <stdio.h>
_Static_assert(LD_T0==0 && LD_T1==1 && LD_T17==153 && LD_T31==496,"independent translation constants");
int main(void){for(uint32_t n=0;n<32;n++){uint32_t v=999;if(ld_triangle(n,&v)!=LD_OK || v!=ld_table[n])return 1;}puts("LD_TABLE translation constants and runtime function agree for32 entries");return 0;}
