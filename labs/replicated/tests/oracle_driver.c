#include "replicated.h"
#include <inttypes.h>
#include <stdio.h>
#include <string.h>
int main(void) {
    char operation[16];
    while(scanf("%15s",operation)==1) {
        unsigned i,n;
        if(!strcmp(operation,"hash")){unsigned char bytes[65],hash[32];if(scanf("%u",&n)!=1||n>65)return 2;for(i=0;i<n;++i){unsigned b;if(scanf("%u",&b)!=1||b>255)return 2;bytes[i]=(unsigned char)b;}if(rp_sha256(bytes,n,hash)!=RP_OK)return 3;for(i=0;i<32;++i)printf("%02x",hash[i]);puts("");}
        else if(!strcmp(operation,"merkle")){uint32_t leaves[8];unsigned char hash[32];if(scanf("%u",&n)!=1||n>8)return 2;for(i=0;i<n;++i)if(scanf("%" SCNu32,&leaves[i])!=1)return 2;if(rp_merkle(leaves,n,hash)!=RP_OK)return 3;for(i=0;i<32;++i)printf("%02x",hash[i]);puts("");}
        else if(!strcmp(operation,"hlc")){RPHLC a,b,out;uint64_t wall;int status;if(scanf("%" SCNu64 " %" SCNu32 " %" SCNu64 " %" SCNu32 " %" SCNu64,&a.physical,&a.logical,&b.physical,&b.logical,&wall)!=5)return 2;status=rp_hlc(a,&b,wall,&out);if(status==RP_OK)printf("0 %" PRIu64 " %" PRIu32 "\n",out.physical,out.logical);else printf("%d\n",status);}
        else if(!strcmp(operation,"wire")){unsigned char bytes[48],out[48];RPMessage message;size_t length;int status;if(scanf("%u",&n)!=1||n>48)return 2;for(i=0;i<n;++i){unsigned b;if(scanf("%u",&b)!=1||b>255)return 2;bytes[i]=(unsigned char)b;}status=rp_decode(bytes,n,&message);printf("%d",status);if(status==RP_OK){if(rp_encode(&message,out,sizeof out,&length)!=RP_OK)return 3;for(i=0;i<length;++i)printf(" %u",out[i]);}puts("");}
        else if(!strcmp(operation,"zigzag")){int32_t value;if(scanf("%" SCNd32,&value)!=1)return 2;printf("%" PRIu32 "\n",rp_zigzag(value));}
        else if(!strcmp(operation,"replay")){int moves[8];RPState state={0,0,0,1},out;RPEffect effects[8];int status;if(scanf("%u",&n)!=1||n>8)return 2;for(i=0;i<n;++i)if(scanf("%d",&moves[i])!=1)return 2;status=rp_replay(state,moves,n,&out,effects);if(status==RP_OK)printf("0 %u %u %u %" PRIu32 "\n",out.tick,out.position,out.score,out.random);else printf("%d\n",status);}
        else if(!strcmp(operation,"log")){RPLog log;unsigned position,key;int value=777,status;rp_log_init(&log);if(scanf("%u",&n)!=1||n>16)return 2;for(i=0;i<n;++i){int v;if(scanf("%u %d",&key,&v)!=2)return 2;if(rp_log_append(&log,key,v)!=RP_OK)return 3;}if(scanf("%u %u",&position,&key)!=2)return 2;status=rp_log_get(&log,position,key,&value);printf("%d %d %u %u\n",status,value,log.count,log.arena.used);}
        else if(!strcmp(operation,"pn")){RPPN a,b;for(i=0;i<3;++i)if(scanf("%" SCNu32,&a.positive[i])!=1)return 2;for(i=0;i<3;++i)if(scanf("%" SCNu32,&a.negative[i])!=1)return 2;for(i=0;i<3;++i)if(scanf("%" SCNu32,&b.positive[i])!=1)return 2;for(i=0;i<3;++i)if(scanf("%" SCNu32,&b.negative[i])!=1)return 2;rp_pn_merge(&a,&b);for(i=0;i<3;++i)printf("%" PRIu32 " ",a.positive[i]);for(i=0;i<3;++i)printf("%" PRIu32 " ",a.negative[i]);printf("%" PRId64 "\n",rp_pn_value(&a));}
        else if(!strcmp(operation,"set")){RPSet states[2]={{{0},{0},{0}},{{0},{0},{0}}},delta[2]={{{0},{0},{0}},{{0},{0},{0}}};if(scanf("%u",&n)!=1||n>32)return 2;for(i=0;i<n;++i){unsigned op,replica,arg;int status=RP_OK;if(scanf("%u %u %u",&op,&replica,&arg)!=3||replica>1)return 2;if(op==0)status=rp_set_add(&states[replica],replica,arg,&delta[replica]);else if(op==1)status=rp_set_remove(&states[replica],arg,&delta[replica]);else if(op==2&&arg<2)rp_set_merge(&states[replica],&states[arg]);else if(op==3&&arg<2)rp_set_merge(&states[replica],&delta[arg]);else return 2;printf("%d ",status);}for(i=0;i<2;++i){unsigned j;for(j=0;j<4;++j)printf("%u %u ",states[i].adds[j],states[i].removed[j]);printf("%u %u ",states[i].next[0],states[i].next[1]);}puts("");}
        else if(!strcmp(operation,"sequence")){RPSequence sequence={{{0,0,0,0}},0};unsigned removed;unsigned char out[16];size_t length;if(scanf("%u",&n)!=1||n>16)return 2;for(i=0;i<n;++i){unsigned id,parent,value;if(scanf("%u %u %u",&id,&parent,&value)!=3||value>255)return 2;if(rp_sequence_insert(&sequence,id,parent,(unsigned char)value)!=RP_OK)return 3;}if(scanf("%u",&removed)!=1)return 2;for(i=0;i<n;++i)if(removed&(1u<<i))if(rp_sequence_remove(&sequence,i+1)!=RP_OK)return 3;if(rp_sequence_render(&sequence,out,sizeof out,&length)!=RP_OK)return 3;printf("%zu",length);for(i=0;i<length;++i)printf(" %u",out[i]);puts("");}
        else return 2;
    }
    return 0;
}
