#include "receiver_macros.h"
int main(void){volatile uint32_t x=0;MpvRead o;return mpv_read_m(&x,&o);}
