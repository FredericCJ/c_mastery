#include "receiver_macros.h"
int main(void){typedef uint32_t Word;uint32_t x=0;return _Generic(&x,uint32_t*:1,Word*:2);}
