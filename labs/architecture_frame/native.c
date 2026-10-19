#include <stdio.h>
#include <string.h>
#include "ledger.h"
int mf_run(unsigned);
void mf_emit(const char *name,const uint32_t *v,size_t n){printf("PACKET %s",name);for(size_t i=0;i<n;++i)printf(" %u",v[i]);puts("");}
int mf_fail(const char *s,unsigned line){fprintf(stderr,"MF_ASSERT:%u:%s\n",line,s);return 41;}
int main(int argc,char **argv){const char *names[]={"reset","capture","ownership","nested"};for(unsigned i=0;i<4;++i)if(argc==2&&!strcmp(names[i],argv[1])){int r=mf_run(i);if(!r)printf("OBS %s\n",names[i]);return r;}return 2;}
