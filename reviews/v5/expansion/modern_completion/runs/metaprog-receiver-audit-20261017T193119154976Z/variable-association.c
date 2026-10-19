#include "receiver_macros.h"
int main(void){unsigned n=2;typedef uint32_t A[n];uint32_t x=0;return _Generic(&x,A:1,default:2);}
