#include <service.h>
#include <stdio.h>
int main(void){
    const unsigned char wire[13]={0xa5,1,2,1,3,0,0,0x41,0xff,0x62,0xf6,0x8b,0x76};
    unsigned cases=0;
    for(unsigned length=0;length<=32;length+=(length==0?3:29)){
        struct cp_message m={258,length,{0}};m.payload[0]=0;m.payload[1]=0x41;m.payload[2]=0xff;
        unsigned char encoded[42];size_t n;if(cp_encode(&m,encoded,42,&n)!=CP_OK)return 1;
        if(length==3)for(size_t i=0;i<n;++i)if(encoded[i]!=wire[i])return 2;
        for(size_t split=0;split<=n;++split){struct cs_service s;size_t used;
            if(cs_init(&s,3,100)!=CS_OK)return 3;
            enum cs_status a=cs_feed(&s,0,encoded,split,&used,1,0);if(used!=split || a!=(split==n?CS_FRAME:CS_MORE))return 4;
            if(split<n){a=cs_feed(&s,0,encoded+split,n-split,&used,1,0);if(a!=CS_FRAME || used!=n-split)return 5;}
            if(s.accepted!=1 || cs_begin(&s)!=CS_OK || s.active.sequence!=258 || s.active.length!=n)return 6;
            for(size_t i=0;i<n;++i)if(s.active.bytes[i]!=encoded[i])return 7;
            ++cases;
        }
    }
    printf("CAP_INSTALLED cases=%u empty10 golden13 maximum42\n",cases);return cases==68?0:8;
}
