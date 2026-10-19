#include "boot_pair.h"
#include "monocypher-ed25519.h"
extern const unsigned char app_start[],app_end[];
extern int fb_selftest(void);
static volatile uint32_t initialized=0xAABBCCDDu,zeroed;
static void emit(const char*s){register unsigned r0 __asm__("r0")=4;register const char*r1 __asm__("r1")=s;__asm__ volatile("bkpt 0xab":"+r"(r0):"r"(r1):"memory");}
static void put(unsigned char*p,uint32_t n){for(unsigned i=0;i<4;++i)p[i]=(unsigned char)(n>>(8*i));}
static _Noreturn void fail(void){emit("FB_GUEST_POLICY_FAIL\n");for(;;)__asm__ volatile("wfi");}
void fb_guest_main(void){
 if(initialized!=0xAABBCCDDu||zeroed||fb_selftest())fail();emit("FB_GUEST_SELFTEST_PASS\n");
 unsigned char image[2080],signature[64],secret[64],seed[32];FBPolicy policy;for(unsigned i=0;i<sizeof policy;++i)((unsigned char*)&policy)[i]=0;for(unsigned i=0;i<32;++i)seed[i]=(unsigned char)i;crypto_ed25519_key_pair(secret,policy.keys[0],seed);policy.present=2;policy.floor[0]=1;policy.generation_floor=1;
 size_t n=(size_t)((uintptr_t)app_end-(uintptr_t)app_start);if(n<514||n>2048)fail();for(unsigned i=0;i<32;++i)image[i]=0;for(size_t i=0;i<n;++i)image[32+i]=app_start[i];put(image,0x31423646u);image[4]=1;put(image+8,1);put(image+12,1);put(image+16,(uint32_t)(uintptr_t)app_start);put(image+20,(uint32_t)n);crypto_ed25519_sign(signature,secret,image,n+32);
 FBImage admitted;FBLayout layout={(uint32_t)(uintptr_t)app_start,(uint32_t)(uintptr_t)app_start+2048u,0x20000000u,0x20020000u};if(!fb_admit(&policy,layout,0,image,n+32,signature,&admitted))fail();
 /* Teaching key/seed is public. Signing exists only in this original emulator
  * fixture; deployed verifier images must not include any signing secret. */
 fb_handoff(admitted.base,admitted.stack,admitted.entry);
}
