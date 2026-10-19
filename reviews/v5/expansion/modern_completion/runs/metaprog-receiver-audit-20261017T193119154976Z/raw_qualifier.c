#include "receivers.h"
#include <stdio.h>
int main(void){uint32_t x=0;uint32_t *const p=&x;int category=_Generic((p),uint32_t*:1,uint32_t *const:2);printf("MPV_RAW %d\n",category);return category!=1;}
