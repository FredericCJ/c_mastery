#include "bits.h"
#include <limits.h>
static void emit(const char *s) {
 register unsigned r0 __asm__("r0")=4;register const char *r1 __asm__("r1")=s;
 __asm__ volatile("bkpt 0xab":"+r"(r0),"+r"(r1)::"memory");
}
_Noreturn static void stop(unsigned rc) {
 unsigned args[2]={0x20026,rc};register unsigned r0 __asm__("r0")=0x20;register unsigned *r1 __asm__("r1")=args;
 __asm__ volatile("bkpt 0xab":"+r"(r0),"+r"(r1)::"memory");for(;;){}
}
#define T(x) do{if(!(x)){emit("FB_TARGET_FAIL\n");stop(1);}}while(0)
void fb_fault(void){emit("FB_TARGET_FAULT\n");stop(2);}
void fb_target_main(void){
 FbWord out=917;FbStats stats;T(fb_shift(16,0x1234,-1,FB_LEFT,&out)==FB_RANGE&&out==917);
 T(fb_shift(16,0x1234,0,FB_ROTATE_LEFT,&out)==FB_OK&&out==0x1234);
 T(fb_shift(16,0x1234,4,FB_ROTATE_LEFT,&out)==FB_OK&&out==0x2341);
 T(fb_shift(8,0x1234,4,FB_RIGHT,&out)==FB_OK&&out==3);
 T(fb_extract(16,0x1234,4,8,&out)==FB_OK&&out==3);
 T(fb_insert(16,0x1234,(FbEdit){4,8,10},&out)==FB_OK&&out==0x12a4);
 T(fb_patch(8,0xa0,(FbEdit){0,4,15},(FbEdit){2,6,0},FB_SEQUENTIAL,&out)==FB_OK&&out==0x83);
 out=917;T(fb_patch(8,0xa0,(FbEdit){0,4,15},(FbEdit){2,6,0},FB_SIMULTANEOUS,&out)==FB_CONFLICT&&out==917);
 T(fb_stats(16,0x1234,&stats)==FB_OK&&stats.population==5&&stats.width==13&&stats.leading==3);
 unsigned char bytes[7]={0};T(fb_encode(UINT32_C(0x12345678),FB_BIG,bytes+3,4)==FB_OK);
 T(fb_decode((FbSpan){bytes+3,4},FB_BIG,&out)==FB_OK&&out==UINT32_C(0x12345678));
 unsigned char left[]={18,52},right[]={86,120};T(fb_decode_two((FbSpan){left,2},(FbSpan){right,2},FB_BIG,&out)==FB_OK&&out==UINT32_C(0x12345678));
 unsigned char known[]={1,1,0,8,18,52,86,120},unknown[]={2,255,0,4};FbRecord record;
 T(fb_record((FbSpan){known,8},FB_REJECT_UNKNOWN,&record)==FB_OK&&record.value==UINT32_C(0x12345678));
 T(fb_record((FbSpan){unknown,4},FB_SKIP_UNKNOWN,&record)==FB_SKIPPED&&record.consumed==4);
 unsigned char v1[]={0,1,0,8,0,0,0,1},v2[]={0,2,0,12,0,0,0,1,0,0,4,0};FbPlugin p;
 T(fb_negotiate((FbSpan){v1,8},&p)==FB_OK&&p.max_items==0);
 T(fb_negotiate((FbSpan){v2,12},&p)==FB_OK&&p.max_items==1024);
 T(fb_negotiate((FbSpan){v2,8},&p)==FB_SHORT&&p.max_items==1024);
 FbRegister r;T(fb_register_init(&r,0xa5,0)==FB_OK);unsigned old=fb_register_read(&r);fb_register_write(&r,(old&240)|3);T(r.state==3);
 T(fb_register_init(&r,0xa5,1)==FB_OK);T(fb_register_update(&r,3,0)==FB_OK&&r.state==0xa3&&r.reads==0);
 T(fb_register_read(&r)==0xa3&&r.state==3);
 emit("FB_TARGET_PASS shifts fields codec prior_integer register_models version_bounds\n");stop(0);
}
