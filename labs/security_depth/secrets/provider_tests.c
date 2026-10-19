#include "provider.h"
#include <stdio.h>
#include <string.h>
#ifdef SS_FIRST_PRINCIPLES
#include "hashing.h"
#endif
#define CHECK(x) do {if(!(x)){fprintf(stderr,"SS_ASSERT:%s\n",#x);return 41;}}while(0)
int main(void) {
    const unsigned char key[4]={1,2,3,4};unsigned char message[65],out[32];SSLedger ledger;
    for(unsigned i=0;i<65;++i)message[i]=(unsigned char)i;
    for(size_t n=0;n<=64;++n) {
        CHECK(ss_mac(key,message,n,0,out,&ledger)==0);
        CHECK(ledger.fetched==1 && ledger.contexts==1 && ledger.freed_fetches==1 && ledger.freed_contexts==1 && ledger.key_cleared==1);
#ifdef SS_FIRST_PRINCIPLES
        unsigned char independent[32];
        CHECK(rph_hmac(key,4,message,n,independent)==RPH_OK && !memcmp(out,independent,32));
        rph_clear(independent,sizeof independent);
#endif
        printf("TAG %zu ",n);for(unsigned i=0;i<32;++i)printf("%02x",out[i]);putchar('\n');
    }
    for(unsigned f=1;f<=7;++f) {
        memset(out,77,sizeof out);CHECK(ss_mac(key,message,4,f,out,&ledger)==-1);
        for(unsigned i=0;i<32;++i)CHECK(out[i]==77);
        CHECK(ledger.fetched==ledger.freed_fetches && ledger.contexts==ledger.freed_contexts && ledger.key_cleared==1);
        CHECK(ledger.fetched==(f>=2) && ledger.contexts==(f>=3));
    }
    memset(out,77,sizeof out);ledger=(SSLedger){9,8,7,6,5};SSLedger old=ledger;
    CHECK(ss_mac(key,message,65,0,out,&ledger)==-1 && ledger.fetched==old.fetched && ledger.contexts==old.contexts && ledger.freed_fetches==old.freed_fetches && ledger.freed_contexts==old.freed_contexts && ledger.key_cleared==old.key_cleared);
    for(unsigned i=0;i<32;++i)CHECK(out[i]==77);
    for(unsigned i=0;i<4;++i)CHECK(key[i]==i+1);
    puts("OBS provider seven-failures output-preserved key-copy-cleared owner-input-retained");return 0;
}
