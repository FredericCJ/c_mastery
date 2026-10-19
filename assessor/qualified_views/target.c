#include "views.h"
static void emit(const char *s) {
 register unsigned r0 __asm__("r0")=4;
 register const char *r1 __asm__("r1")=s;
 __asm__ volatile("bkpt 0xab":"+r"(r0),"+r"(r1)::"memory");
}
_Noreturn static void stop(unsigned rc) {
 unsigned args[2]={0x20026,rc};register unsigned r0 __asm__("r0")=0x20;
 register unsigned *r1 __asm__("r1")=args;
 __asm__ volatile("bkpt 0xab":"+r"(r0),"+r"(r1)::"memory");
 for(;;){}
}
#define T(x) do{if(!(x)){emit("FV_TARGET_FAIL\n");stop(1);}}while(0)
void qv_fault(void){emit("FV_TARGET_FAULT\n");stop(2);}
void qv_target_main(void) {
 FvRecord record;FvView view=fv_initialize(&record);
 T(view.data==record.values&&view.length==4&&view.data[1]==7&&record.link==NULL);
 int out[4]={91,92,93,94};T(fv_copy((FvDestination){out,3},view)==FV_FULL&&out[0]==91);
 T(fv_copy((FvDestination){out,4},view)==FV_OK&&out[1]==7&&out[3]==9);
 T(fv_update(&record,1,17)==FV_OK&&view.data[1]==17&&out[1]==7);
 T(fv_update(&record,4,17)==FV_RANGE);
 T(fv_move(out,4,1,0,3)==FV_OK&&out[1]==0&&out[2]==7&&out[3]==0);
 T(fv_move(out,4,0,1,3)==FV_OK&&out[1]==7);
 T(fv_move(NULL,0,0,0,0)==FV_OK);
 T(fv_move(out,4,4,0,1)==FV_RANGE);
 int linked=7;record.link=&linked;T(fv_reset(&record)==FV_OK&&record.link==NULL&&record.values[1]==0);
 FvOwner *owner=fv_owner();T(owner!=NULL);FvView opaque=fv_owner_view(owner);int *p=fv_owner_mutable(owner);T(p!=NULL&&opaque.data==p&&opaque.length==4);p[2]=19;T(opaque.data[2]==19);
 volatile unsigned status=4;T(fv_device_twice(&status)==0);
 emit("FV_TARGET_PASS caller_owner live_snapshot overlap reset opaque volatile_model\n");stop(0);
}
