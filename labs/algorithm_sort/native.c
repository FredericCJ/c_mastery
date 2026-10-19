#include "sort.h"
#include <stdio.h>
#include <string.h>
int as_failure(const char*condition,unsigned line){fprintf(stderr,"AS_ASSERT:%u:%s\n",line,condition);return 41;}
void as_observe(unsigned prefix,const algo_pair*a,unsigned n){printf("PREFIX %u",prefix);for(unsigned i=0;i<n;++i)printf(" %u:%u",a[i].key,a[i].tag);putchar('\n');}
void as_metric(unsigned n,unsigned distribution,const ASStats*w){printf("METRIC %u %u %u %u %u %u\n",n,distribution,w->comparisons,w->shifts,w->placements,w->prefixes);}
int main(int argc,char**argv){const char*names[]={"insertion","counts","descending","wide","failure","prior"};if(argc!=2)return 2;for(unsigned i=0;i<6;++i)if(!strcmp(argv[1],names[i])){int result=as_test(i);if(!result)printf("OBS %s actual-source-states original-contract\n",names[i]);return result;}return 2;}
