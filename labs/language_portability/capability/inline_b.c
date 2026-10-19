#include "inline.h"
int (*lp_pointer_b)(int)=lp_inline;
int lp_b(void){return lp_pointer_b(6);}
