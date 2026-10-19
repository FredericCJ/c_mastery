#include "receiver_macros.h"
int main(void){uint32_t x=0;uint32_t *p=&x;return _Generic(p,uint32_t*:1,const uint32_t*:p->missing);}
