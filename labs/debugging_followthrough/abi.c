#include "algo.h"
#include <stdio.h>
#ifndef DX_ABI_BAD
#define DX_ABI_BAD 0
#endif
int main(void){uint32_t a[]={1,3,3,7},result=99;int okay=algo_abi_probe(a,4,3,&result);printf("DX_ABI:preserved=%d:result=%u:expected-preserved=%d\n",okay,result,!DX_ABI_BAD);return !(result==1&&okay==!DX_ABI_BAD);}
