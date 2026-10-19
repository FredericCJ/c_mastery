#include "port.h"
#ifndef TP_MUTANT
#define TP_MUTANT 0
#endif
#define REG(a) (*(volatile uint32_t *)(uintptr_t)(a))
#define CSR REG(0xe000e010u)
#define RVR REG(0xe000e014u)
#define CVR REG(0xe000e018u)
#define ICSR REG(0xe000ed04u)
#define TIMER_CTRL REG(0x40000000u)
#define TIMER_VALUE REG(0x40000004u)
#define TIMER_RELOAD REG(0x40000008u)
static uint32_t lock(void){uint32_t mask;__asm__ volatile("mrs %0,primask\ncpsid i":"=r"(mask)::"memory");return mask;}
static void unlock(uint32_t mask){__asm__ volatile("dsb\nisb\nmsr primask,%0"::"r"(mask):"memory");}
uint32_t tp_raw(void){return UINT32_MAX-TIMER_VALUE;}
uint32_t tp_now(const TPClock *clock){return clock->logical_origin+(tp_raw()-clock->raw_origin);}
enum tq_status tp_init(TPClock *clock,TQTimer *timer,uint32_t origin){
 uint32_t mask=lock();CSR=0;ICSR=1u<<25;TIMER_CTRL=0;TIMER_RELOAD=UINT32_MAX;TIMER_CTRL=1;
 *clock=(TPClock){.timer=timer,.raw_origin=tp_raw(),.logical_origin=origin};
 enum tq_status status=tq_init(timer,origin);unlock(mask);return status;
}
enum tq_status tp_wait(TPClock *clock,unsigned task,uint32_t delay,unsigned infinite){
 uint32_t mask=lock();if(clock->stopped){unlock(mask);return TQ_INVALID;}
 unsigned old_armed=clock->timer->armed;
 enum tq_status status=tq_advance(clock->timer,tp_now(clock));
 if(status==TQ_OK)status=tq_wait(clock->timer,task,delay,infinite);
 if(TP_MUTANT!=1)CSR=0;else clock->timer->armed=old_armed;
 unlock(mask);return status;
}
enum tq_status tp_at(TPClock *clock,unsigned task,uint32_t at){
 uint32_t mask=lock();if(clock->stopped){unlock(mask);return TQ_INVALID;}
 unsigned old_armed=clock->timer->armed;
 enum tq_status status=tq_advance(clock->timer,tp_now(clock));
 if(status==TQ_OK)status=tq_wait(clock->timer,task,at-clock->timer->kernel.now,0);
 if(TP_MUTANT!=1)CSR=0;else clock->timer->armed=old_armed;
 unlock(mask);return status;
}
enum tq_status tp_arm(TPClock *clock,uint32_t injected){
 if(injected>=UINT32_C(0x80000000))return TQ_INVALID;
 uint32_t mask=lock();if(clock->stopped){unlock(mask);return TQ_INVALID;}
 TQTimer *timer=clock->timer;
 if(TP_MUTANT==1&&timer->armed){unlock(mask);return TQ_OK;}
 CSR=0;TQPlan plan;enum tq_status status=tq_plan(timer,tp_now(clock),&plan);
 if(status!=TQ_OK){unlock(mask);return status;}
 ICSR=1u<<25;
 if(timer->events){(void)tq_abort(timer);unlock(mask);return TQ_RETRY;}
 if(plan.armed){
  uint32_t distance=plan.at-plan.observed;
  if(distance>0x00ffffffu)distance=0x00ffffffu;
  RVR=distance;CVR=0;CSR=7;
 }
 uint32_t start=tp_raw();while((uint32_t)(tp_raw()-start)<injected){}
 uint32_t after=tp_now(clock);unsigned pending=(ICSR&(1u<<26))!=0;
 if(TP_MUTANT==2){after=plan.observed;pending=0;}
 status=tq_commit(timer,plan,after,pending);
 if(status!=TQ_OK){CSR=0;ICSR=1u<<25;}
 unlock(mask);return status;
}
enum tq_status tp_interrupt(TPClock *clock){
 uint32_t mask=lock();if(clock->stopped){unlock(mask);return TQ_INVALID;}
 CSR=0;clock->timer->armed=0;clock->interrupts++;
 enum tq_status status=tq_advance(clock->timer,tp_now(clock));unlock(mask);return status;
}
enum tq_status tp_stop(TPClock *clock){
 uint32_t mask=lock();if(clock->stopped){unlock(mask);return TQ_INVALID;}
 CSR=0;ICSR=1u<<25;(void)tq_abort(clock->timer);
 uint32_t raw=tp_raw();enum tq_status status=tq_advance(clock->timer,clock->logical_origin+(raw-clock->raw_origin));
 if(status==TQ_OK){clock->resume=(TQResume){raw,1,1,0};clock->stopped=1;}
 unlock(mask);return status;
}
enum tq_status tp_resume(TPClock *clock){
 uint32_t mask=lock();if(clock->stopped!=1){unlock(mask);return TQ_INVALID;}
 enum tq_status status=tq_resume(clock->timer,&clock->resume,tp_raw());
 if(status==TQ_OK)clock->stopped=0;
 unlock(mask);return status;
}
void tp_shutdown(TPClock *clock){uint32_t mask=lock();CSR=0;ICSR=1u<<25;TIMER_CTRL=0;(void)tq_abort(clock->timer);clock->stopped=2;unlock(mask);}
