#include "sleep.h"
#include "mcu.h"
void mf_main(void){mf_clock_init();uint32_t counts;uint64_t remaining;
 mf_require(mf_counts(101,32768,0xffffff,&counts,&remaining)==CMX_OK&&counts==4&&remaining==0,210);
 for(unsigned boundary=0;boundary<8;boundary++){
  struct mf_nap s;mf_nap(&s,25000,boundary);uint32_t credited=s.elapsed;
  mf_require(!mf_nap_finish(&s)&&s.elapsed==credited,211);
  if(boundary<6)mf_require(s.elapsed<25000&&s.remaining>0,212);
  if(boundary==6)mf_require(s.slept&&s.elapsed>0&&s.elapsed<25000&&s.remaining>0,213);
  if(boundary==7)mf_require(s.slept&&s.elapsed>=25000&&s.remaining==0&&s.wake_irq,214);
  mf_text("MF wake-boundary/elapsed/remaining/slept\n");mf_number(boundary);mf_number(s.elapsed);mf_number(s.remaining);mf_number(s.slept);
 }
 /* Same units, split hardware maximum. Compensation is measured each time;
  * execution/interrupt overhead may shorten the remaining wall-clock wait. */
 uint32_t total=0,left=65000,segments=0;while(left){uint32_t n=left>25000?25000:left;
  struct mf_nap s;mf_nap(&s,n,7);total+=s.elapsed;left=total>=65000?0:65000-total;segments++;mf_require(segments<=3,215);}
 mf_require(total>=65000&&segments==3,216);
 mf_text("MF split-three elapsed counts\n");mf_number(total);
 mf_text("PASS MF sleep\n");mf_finish(0);
}
