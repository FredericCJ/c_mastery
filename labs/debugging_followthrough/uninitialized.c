#include <stdio.h>
#include <stdlib.h>
#ifndef DX_FIXED
#define DX_FIXED 0
#endif
__attribute__((noinline)) static int copied(int x){return x;}
int main(void){int *p=malloc(sizeof *p);if(!p)return 2;if(DX_FIXED)*p=7;int a=*p;int b=copied(a);if(b==7)puts("DX_COPY:seven");else puts("DX_COPY:other");free(p);return 0;}
