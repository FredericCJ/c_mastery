#include "ring.h"
#include <stdio.h>
#include <inttypes.h>
static int tokens(struct cr_token *t,unsigned n)
{
    for(unsigned i=0;i<n;i++){unsigned p,o,v;if(scanf("%u %u %u",&p,&o,&v)!=3||p>65535||o>255||v>255)return 0;t[i]=(struct cr_token){(uint16_t)p,(uint8_t)o,(uint8_t)v};}return 1;
}
int main(void)
{
    char op;
    while(scanf(" %c",&op)==1){
        unsigned n;uint32_t epoch;struct cr_token t[CR_TOKENS];struct cr_ring ring;
        if(scanf("%u %"SCNu32,&n,&epoch)!=2||n>CR_TOKENS)return 2;
        if(op=='Q'){
            unsigned key,wanted,down;uint32_t expected;
            if(scanf("%u %"SCNu32" %u %u",&key,&expected,&wanted,&down)!=4||key>65535||wanted>16||!tokens(t,n))return 2;
            enum cr_status s=cr_build(t,n,epoch,&ring);if(s!=CR_OK)return 3;
            unsigned owner=99,out[CR_OWNERS];for(unsigned i=0;i<CR_OWNERS;i++)out[i]=99;struct cr_cost c={99,99};
            s=cr_owner(&ring,(uint16_t)key,expected,&owner,&c);printf("%d %u",s,owner);
            s=cr_replicas(&ring,(uint16_t)key,expected,wanted,(uint16_t)down,out,&c);printf(" %d",s);
            for(unsigned i=0;i<wanted;i++)printf(" %u",out[i]);
            puts("");
        }else if(op=='M'){
            unsigned owner,k,cut;struct cr_token extra[CR_TOKENS];
            if(scanf("%u %u %u",&owner,&k,&cut)!=3||k>CR_TOKENS||!tokens(t,n)||!tokens(extra,k))return 2;
            enum cr_status s=cr_build(t,n,epoch,&ring);if(s!=CR_OK)return 3;
            s=cr_change(&ring,owner,extra,k,cut==99?SIZE_MAX:cut);
            printf("%d %"PRIu32" %zu",s,ring.epoch,ring.count);
            for(size_t i=0;i<ring.count;i++)printf(" %u %u %u",ring.token[i].position,ring.token[i].owner,ring.token[i].ordinal);
            puts("");
        }else return 2;
    }
    return 0;
}
