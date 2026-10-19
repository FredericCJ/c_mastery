typedef unsigned U;
extern U stack_top, test_low, test_top, test_guard;
extern U ts_probe(U requested,U *observed);
static void semi(U op,const void*p){register U r0 __asm__("r0")=op;register const void*r1 __asm__("r1")=p;__asm__ volatile("bkpt 0xab":"+r"(r0):"r"(r1):"memory");}
static void stop(U n){U a[2]={0x20026,n};semi(4,n?"B06_THUMB_STACK_FAIL\n":"B06_THUMB_STACK_PASS\n");semi(0x20,a);for(;;){}}
static void fault(void){stop(3);}
void reset(void){
 __asm__ volatile("cpsid i":::"memory");
 volatile unsigned char *guard=(volatile unsigned char*)&test_guard;
 for(U i=0;i<16;i++)guard[i]=0xa5;
 for(U n=0;n<=128;n++){
  U result=99,old_sp,new_sp;__asm__ volatile("mov %0,sp":"=r"(old_sp));
  if(ts_probe(n,&result)!=1)stop(1);
  __asm__ volatile("mov %0,sp":"=r"(new_sp));
  U want=((U)&test_top-n)&~7u;
  if(result!=want||result<(U)&test_low||(result&7)||old_sp!=new_sp)stop(1);
  for(U i=0;i<16;i++)if(guard[i]!=0xa5)stop(1);
 }
 U invalid[3]={129,136,~0u};
 for(U i=0;i<3;i++){U result=99;if(ts_probe(invalid[i],&result)!=0||result!=99)stop(1);}
 stop(0);
}
__attribute__((section(".vectors"),used)) const void*v[]={&stack_top,reset,fault,fault,fault,fault,fault,0,0,0,0,fault,fault,0,fault,fault};
