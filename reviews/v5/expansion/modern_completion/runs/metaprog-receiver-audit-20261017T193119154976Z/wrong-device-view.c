#include "receiver_macros.h"
int main(void){volatile uint32_t x=0;MpvRW rw={&x,15};return mpv_ack(rw,1);}
