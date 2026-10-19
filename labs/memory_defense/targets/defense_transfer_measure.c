#define _POSIX_C_SOURCE 200809L
#include "defense_transfer.h"
#include <openssl/crypto.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
static volatile unsigned witness;
static double now(void){struct timespec t;clock_gettime(CLOCK_MONOTONIC,&t);return t.tv_sec+t.tv_nsec*1e-9;}
int main(void){unsigned char a[256],b[256];memset(a,19,256);puts("backend,position,sample,seconds");for(unsigned kind=0;kind<2;kind++)for(unsigned position=0;position<4;position++)for(unsigned sample=0;sample<9;sample++){memcpy(b,a,256);if(position<3)b[position==0?0:position==1?127:255]^=1;size_t steps=0;double begin=now();for(unsigned i=0;i<10000;i++)witness+=(unsigned)(kind?CRYPTO_memcmp(a,b,256)==0:dt_equal(a,256,b,256,&steps));printf("%u,%u,%u,%.9g\n",kind,position,sample,now()-begin);}printf("DT_TIMING_DONE %u\n",witness);return 0;}
