#include "lookup.h"
#include <stdio.h>
#include <string.h>
int al_failure(const char*s,unsigned line){fprintf(stderr,"AL_ASSERT:%u:%s\n",line,s);return 41;}
void al_emit_lookup(unsigned kind,uint32_t key,const struct al_probe*p){printf("LOOKUP %u %u %d %u",kind,key,p->index,p->used);for(unsigned i=0;i<p->used;++i)printf(" %u",p->visits[i]);puts("");}
void al_emit_match(unsigned t,size_t n,unsigned p,size_t m,const uint64_t*out,size_t used,uint64_t cost){printf("MATCH %u %zu %u %zu %zu %llu",t,n,p,m,used,(unsigned long long)cost);for(size_t i=0;i<used;++i)printf(" %llu",(unsigned long long)out[i]);puts("");}
void al_emit_stream(unsigned t,size_t n,unsigned p,size_t m,size_t split,const uint64_t*out,size_t used){printf("STREAM %u %zu %u %zu %zu %zu",t,n,p,m,split,used);for(size_t i=0;i<used;++i)printf(" %llu",(unsigned long long)out[i]);puts("");}
int main(int argc,char**argv){const char*names[]={"lookup","prior","matching","callbacks","stream"};if(argc!=2)return 2;for(unsigned i=0;i<5;++i)if(!strcmp(argv[1],names[i])){int r=al_test(i);if(!r)printf("OBS %s\n",names[i]);return r;}return 2;}
