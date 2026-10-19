#include "receiver_macros.h"
#include <stdio.h>
int main(void){uint32_t x=7;uint32_t *const p=&x;const uint32_t *q=&x;volatile uint32_t v=8;const volatile uint32_t *cv=&v;MpvRead o={0};if(MPV_READ(p,&o)||o.category!=1||MPV_READ(q,&o)||o.category!=2||MPV_READ(&v,&o)||o.category!=3||MPV_READ(cv,&o)||o.category!=4)return 1;uint32_t a[1]={9};if(MPV_READ(a,&o)||o.value!=9||o.category!=1)return 2;puts("MPV_TYPED top-pointer-unqualified pointee-cv retained array-decay");return 0;}
