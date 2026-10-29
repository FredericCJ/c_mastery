#include "parse.h"
static void say(const char*s){register unsigned r0 __asm__("r0")=4;register const char*r1 __asm__("r1")=s;__asm__ volatile("bkpt 0xab" : "+r"(r0):"r"(r1):"memory");}
static void finish(unsigned status){unsigned args[2];args[0]=0x20026;args[1]=status;register unsigned r0 __asm__("r0")=0x20;register void*r1 __asm__("r1")=args;__asm__ volatile("bkpt 0xab" : "+r"(r0):"r"(r1):"memory");for(;;){}}
#define TEST(x) do{if(!(x)){say("BF_TARGET_CHECK " #x "\n");finish(1);}}while(0)
static const unsigned char endpoint[]={45,50,49,52,55,52,56,51,54,52,56};
static const unsigned char overflow[]={50,49,52,55,52,56,51,54,52,56};
static const unsigned char invalid[]={33};
void bf_target(void){
 int32_t a=77,b=77;struct bf_parse s;
 TEST(b2_parse(endpoint,sizeof endpoint,&a)==B2_OK && a==INT32_MIN);
 TEST(bf_parse_radix(endpoint,sizeof endpoint,10,&b)==B2_OK && b==a);
 TEST(bf_parse_reset(&s,10)==B2_OK);TEST(bf_parse_feed(&s,overflow,sizeof overflow)==B2_OK);TEST(bf_parse_feed(&s,invalid,1)==B2_OK);TEST(bf_parse_finish(&s,&b)==B2_INVALID && b==INT32_MIN);
 TEST(bf_parse_reset(&s,10)==B2_OK);TEST(bf_parse_feed(&s,endpoint,1)==B2_OK);TEST(bf_parse_feed(&s,endpoint+1,sizeof endpoint-1)==B2_OK);TEST(bf_parse_finish(&s,&b)==B2_OK && b==INT32_MIN);TEST(bf_parse_cancel(&s)==B2_OK);TEST(bf_parse_finish(&s,&b)==BF_CANCELLED);
 say("BF_TARGET_PASS C17 M4 actual-B02-streaming no-hosted-parser\n");finish(0);
}
