#include "transaction.h"
static void say(const char*s){register unsigned r0 __asm__("r0")=4;register const char*r1 __asm__("r1")=s;__asm__ volatile("bkpt 0xab" : "+r"(r0):"r"(r1):"memory");}
_Noreturn static void finish(unsigned status){unsigned args[2];args[0]=0x20026;args[1]=status;register unsigned r0 __asm__("r0")=0x20;register void*r1 __asm__("r1")=args;__asm__ volatile("bkpt 0xab" : "+r"(r0):"r"(r1):"memory");for(;;){}}
#define TEST(x) do{if(!(x)){say("BF_TARGET_CHECK " #x "\n");finish(1);}}while(0)
typedef struct {unsigned mask,failure,compensation,events;} Backend;
static int acquire(void*p,unsigned i){Backend*b=p;++b->events;if(i==b->failure)return 10+(int)i;TEST(!(b->mask&(1u<<i)));b->mask|=1u<<i;return 0;}
static int release(void*p,unsigned i){Backend*b=p;++b->events;TEST((b->mask&(1u<<i))&&!(b->mask&~((1u<<(i+1))-1u)));if(i==2&&b->compensation){--b->compensation;return 22;}b->mask&=~(1u<<i);return 0;}
static void nofail(void*p,unsigned i){TEST(release(p,i)==0);}
static int commit(void*p){Backend*b=p;TEST(b->mask==7);++b->events;return 41;}
void bf_target(void){
 for(unsigned style=0;style<4;++style)for(unsigned f=0;f<4;++f){Backend b={0};b.failure=f;B2Backend api={acquire,nofail,&b};unsigned out=99;TEST(bf_construct(style,api,&out)==(f==3?B2_OK:B2_FAILED)&&out==(f==3?3u:99u));if(f==3){nofail(&b,2);nofail(&b,1);nofail(&b,0);}TEST(!b.mask);}
 Backend b={0};b.failure=3;b.compensation=1;BfTx tx;bf_tx_init(&tx);BfTxBackend api={acquire,release,commit,&b};unsigned out=99;TEST(bf_tx_begin(&tx,api,&out)==BF_TX_RECOVERY&&out==99&&tx.mask==7&&b.mask==7&&tx.primary==41&&tx.cleanup_error==22);TEST(bf_tx_begin(&tx,api,&out)==B2_INVALID&&b.events==5);TEST(bf_tx_close(&tx)==B2_FAILED&&tx.primary==41&&tx.cleanup_error==22&&!tx.mask&&!b.mask&&b.events==8);TEST(bf_tx_close(&tx)==B2_FAILED&&b.events==8);
 say("BF_TARGET_PASS C17 M4 actual-B02 styles retained-registration compensation\n");finish(0);
}
