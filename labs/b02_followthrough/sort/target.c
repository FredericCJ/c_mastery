#include "sort.h"
static void say(const char*s){register unsigned r0 __asm__("r0")=4;register const char*r1 __asm__("r1")=s;__asm__ volatile("bkpt 0xab" : "+r"(r0):"r"(r1):"memory");}
static void finish(unsigned status){unsigned args[2];args[0]=0x20026;args[1]=status;register unsigned r0 __asm__("r0")=0x20;register void*r1 __asm__("r1")=args;__asm__ volatile("bkpt 0xab" : "+r"(r0):"r"(r1):"memory");for(;;){}}
#define TEST(x) do{if(!(x)){say("BF_TARGET_CHECK " #x "\n");finish(1);}}while(0)
void bf_target(void){
 struct b2_item a[4]={{2,0},{1,1},{2,2},{1,3}},b[4],scratch[4];struct bf_work work;size_t indices[4],iscratch[4],expected[4]={1,3,0,2};unsigned char payload[4]={1,2,3,4};struct bf_record records[4];
 for(unsigned i=0;i<4;++i){b[i]=a[i];records[i]=(struct bf_record){a[i].key,i,&records[i],&payload[i],1};}
 TEST(b2_sort(a,4,scratch,4)==B2_OK);TEST(bf_sort_bottom(b,4,scratch,4,&work)==B2_OK);
 TEST(bf_sort_indirect(records,4,indices,4,iscratch,4,&work)==B2_OK&&work.payload_writes==0);
 for(unsigned i=0;i<4;++i){TEST(a[i].serial==expected[i]&&b[i].serial==expected[i]&&indices[i]==expected[i]);TEST(records[i].self==&records[i]&&payload[i]==i+1);}
 size_t next=0;TEST(bf_sort_width(SIZE_MAX,SIZE_MAX-1,&next)==B2_OK&&next==SIZE_MAX);
 say("BF_TARGET_PASS C17 M4 actual-B02 stable-bottom-up borrowed-permutation\n");finish(0);
}
