#include <stdio.h>
#include <stdlib.h>
volatile unsigned cells[2]={4,8};
volatile unsigned generation=1;
__attribute__((noinline)) void mw_marker(unsigned stage){__asm volatile(""::"r"(stage):"memory");}
int main(void){unsigned *p=malloc(sizeof *p);if(!p)return 2;*p=7;mw_marker(1);cells[1]=99;/* Legal address, wrong logical field: no section growth. */mw_marker(2);unsigned *next=malloc(sizeof *next);if(!next){free(p);return 2;}*next=*p;mw_marker(3);free(p);p=next;generation++;mw_marker(4);*p=82;mw_marker(5);printf("MW:watch:legal-cell%u:live-generation%u:value%u\n",cells[1],generation,*p);free(p);return 0;}
