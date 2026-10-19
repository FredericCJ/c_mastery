#include "receiver_macros.h"
int main(void){uint32_t x=0;return _Generic(&x,void(void):1,default:2);}
