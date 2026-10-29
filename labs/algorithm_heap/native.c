#include "heap.h"
#include <stdio.h>
#include <string.h>
int ah_failure(const char*x,unsigned line){fprintf(stderr,"AH_ASSERT:%u:%s\n",line,x);return 41;}
void ah_emit(unsigned f,const uint32_t*a,unsigned n,const struct ah_stats*s,const struct ah_trace*t){printf("HEAP %u %u %u %u",f,n,s->comparisons,s->swaps);for(unsigned i=0;i<n;++i)printf(" %u",a[i]);putchar('\n');for(unsigned k=0;k<t->used;++k){printf("SWAP %u %u",f,t->position[k]);for(unsigned i=0;i<n;++i)printf(" %u",t->state[k][i]);putchar('\n');}}
int main(int argc,char**argv){const char*names[]={"premise","scalar","observed","prior","minimum","ternary","binary"};if(argc!=2)return 2;for(unsigned i=0;i<7;++i)if(!strcmp(argv[1],names[i])){int result=ah_test(i);if(!result)printf("OBS %s\n",names[i]);return result;}return 2;}
