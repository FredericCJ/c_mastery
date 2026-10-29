#include "firmware.h"
#include <stdio.h>
#include <stdlib.h>
extern int cf_checks(unsigned);
int main(int argc,char **argv){if(argc!=2)return 2;unsigned n=(unsigned)strtoul(argv[1],0,10);int result=cf_checks(n);if(result){printf("CAP_FIRMWARE_ASSERT case=%u line=%d\n",n,result);return 41;}printf("CAP_FIRMWARE_PASS case=%u static=%zu core=%zu dma-backing=128\n",n,sizeof(struct cf_firmware),sizeof(struct cs_service));return 0;}
