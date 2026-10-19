#include "transfer.h"
#include <stdio.h>
#include <stdlib.h>
int ct_failure(const char*s,unsigned n){fprintf(stderr,"CT_ASSERT:%s:%u\n",s,n);return 41;}
void ct_observation(const char*s,unsigned n,unsigned size){printf("CT_PASS %s cases=%u storage=%u\n",s,n,size);}
int main(int argc,char**argv){if(argc!=2)return 2;return ct_tests((unsigned)strtoul(argv[1],NULL,10));}
