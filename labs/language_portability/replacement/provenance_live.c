/* Defined selected-profile control only; expired example is reasoning-only. */
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
int main(void){int *p=malloc(sizeof *p);if(!p)return 2;*p=41;
 uintptr_t saved=(uintptr_t)(void*)p;void *back=(void*)saved;int *q=back;
 if(q!=p || *q!=41){free(p);return 1;}*q=42;if(*p!=42){free(p);return 1;}
 free(p);/* Do not inspect p,q,back or cast/use saved after lifetime end. */
 puts("LP_PROVENANCE live void-pointer integer roundtrip only");return 0;}
