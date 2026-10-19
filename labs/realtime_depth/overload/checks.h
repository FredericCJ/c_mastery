#ifndef CMASTERY_OVERLOAD_CHECKS_H
#define CMASTERY_OVERLOAD_CHECKS_H
#include "overload.h"
#include "cmx.h"
#define OA(x) do { if(!(x))return __LINE__; }while(0)
static int od_check_admission(void){
 ODSet s={0};enum od_decision d;OA(od_admit(&s,(ODTask){1,4,1},4,&d)==OD_OK&&d==OD_ADMITTED&&s.count==1);
 OA(od_admit(&s,(ODTask){2,4,0},4,&d)==OD_OK&&d==OD_ADMITTED&&s.count==2);
 OA(od_admit(&s,(ODTask){1,4,0},4,&d)==OD_OK&&d==OD_REJECTED&&s.count==2);
 ODSet prefix={0};OA(od_admit(&prefix,(ODTask){1,7,1},8,&d)==OD_OK&&d==OD_INCONCLUSIVE&&prefix.count==0);
 OA(od_admit(&prefix,(ODTask){1,7,1},28,&d)==OD_OK&&d==OD_ADMITTED&&prefix.count==1);return 0;
}
static int od_check_queue(void){
 ODSet s={{{1,4,1}},1};ODRequest r[12];ODResult o;
 for(unsigned i=0;i<12;i++)r[i]=(ODRequest){i,0,i/3,1,20,0};
 OA(od_run(&s,r,12,2,4,OD_REJECT_NEW,0,OD_NONE,OD_NONE,&o)==OD_OK);
 unsigned done=0,reject=0,live=0;for(unsigned i=0;i<12;i++){done+=o.items[i].state==OD_DONE;reject+=o.items[i].state==OD_REJECT;live+=o.items[i].state==OD_PENDING;}
 OA(done==4&&reject==7&&live==1&&o.peak==2&&o.work==4);OA(o.items[9].state==OD_PENDING);return 0;
}
static int od_check_policy(void){
 ODSet s={{{2,4,1}},1};ODRequest r[3]={{10,0,0,2,20,0},{11,0,1,2,20,0},{12,0,1,2,20,0}};ODResult o;
 OA(od_run(&s,r,3,2,4,OD_DROP_OLDEST,0,OD_NONE,OD_NONE,&o)==OD_OK);
 OA(o.items[0].state==OD_DROP&&o.items[0].executed==1&&o.items[0].finish==1);
 OA(o.items[1].state==OD_DONE&&o.items[2].state==OD_PENDING);
 OA(od_run(&s,r,3,2,4,OD_LATEST,0,OD_NONE,OD_NONE,&o)==OD_OK);
 OA(o.items[0].state==OD_DROP&&o.items[1].state==OD_DROP&&o.items[2].state==OD_DONE);return 0;
}
static int od_check_overrun(void){
 ODSet s={{{2,4,1}},1};ODRequest r[4]={{1,0,0,8,30,0},{2,0,2,1,30,0},{3,0,4,1,30,0},{4,0,6,1,30,0}};ODResult o;
 OA(od_run(&s,r,4,2,12,OD_REJECT_NEW,4,OD_NONE,OD_NONE,&o)==OD_OK);
 OA(o.heartbeats==3&&o.items[0].state==OD_OVERRUN&&o.items[0].executed==2);
 OA(o.items[1].state==OD_DONE&&o.items[2].state==OD_DONE&&o.items[3].state==OD_DONE);
 for(unsigned t=0;t<12;t+=4){OA(o.ticks[t].heartbeat==1&&o.ticks[t].selected==OD_NONE);}return 0;
}
static int od_check_mode(void){
 ODSet s={{{3,8,0},{2,8,1}},2};ODRequest r[5]={{1,0,0,3,20,6},{2,1,0,2,20,0},{3,0,2,1,20,0},{4,1,3,1,20,0},{5,0,7,1,20,0}};ODResult o;
 OA(od_run(&s,r,5,3,10,OD_REJECT_NEW,4,2,6,&o)==OD_OK);
 OA(o.items[0].state==OD_MODE_DROP&&o.items[0].finish==2&&o.items[2].state==OD_MODE_REJECT);
 OA(o.items[1].state==OD_DONE&&o.items[3].state==OD_DONE&&o.items[4].state==OD_DONE);
 OA(o.mode_switches==1&&o.recovered==1&&o.recovery_refused==0);
 OA(od_run(&s,r,5,3,10,OD_REJECT_NEW,4,2,2,&o)==OD_OK&&o.recovery_refused==1&&o.recovered==0);return 0;
}
static int od_check_io(void){
 ODSet s={{{1,4,1}},1};ODRequest r[4]={{1,0,0,1,20,100},{2,0,1,1,20,100},{3,0,2,1,20,0},{4,0,3,1,20,0}};ODResult o;
 OA(od_run(&s,r,4,2,8,OD_REJECT_NEW,4,OD_NONE,4,&o)==OD_OK);
 OA(o.work==0&&o.heartbeats==2&&o.peak==2&&o.recovery_refused==1);
 OA(o.items[0].state==OD_PENDING&&o.items[1].state==OD_PENDING&&o.items[2].state==OD_REJECT&&o.items[3].state==OD_REJECT);
 OA(od_run(&s,r,4,2,20,OD_REJECT_NEW,4,OD_NONE,20,&o)==OD_OK);
 OA(o.items[0].state==OD_MISS&&o.items[1].state==OD_MISS&&o.recovered==1);return 0;
}
static int od_check_backpressure(void){ODDependency d;
 OA(od_backpressure(&d)==OD_OK);OA(d.producer_waiting&&d.consumer_waiting&&d.mutex_owner==0);
 OA(d.repaired_owner==1&&d.space==1&&d.producer_result==(unsigned)CMX_CLOSED);return 0;
}
static int od_check_guards(void){
 ODSet s={0};enum od_decision d=OD_ADMITTED;OA(od_admit(&s,(ODTask){2,1,0},4,&d)==OD_INVALID&&s.count==0&&d==OD_ADMITTED);
 s=(ODSet){{{1,4,1}},1};ODRequest r[2]={{7,0,0,1,8,0},{7,0,1,1,8,0}};ODResult o={0};o.peak=99;
 OA(od_run(&s,r,2,2,8,OD_REJECT_NEW,4,OD_NONE,OD_NONE,&o)==OD_INVALID&&o.peak==99);
 r[1].id=8;OA(od_run(&s,r,2,2,8,(enum od_policy)-1,4,OD_NONE,OD_NONE,&o)==OD_INVALID&&o.peak==99);
 r[0].deadline=0;OA(od_run(&s,r,2,2,8,OD_REJECT_NEW,4,OD_NONE,OD_NONE,&o)==OD_OK&&o.items[0].state==OD_MISS);
 return 0;
}
#undef OA
#endif
