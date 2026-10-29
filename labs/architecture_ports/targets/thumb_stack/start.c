typedef unsigned po_u;
extern po_u stack_top, test_low, test_top, test_guard;
extern po_u ts_probe(po_u requested,po_u *observed);
static void semi(po_u op,const void*p){register po_u r0 __asm__("r0")=op;register const void*r1 __asm__("r1")=p;__asm__ volatile("bkpt 0xab":"+r"(r0):"r"(r1):"memory");}
static void stop(po_u n){po_u a[2]={0x20026,n};semi(4,n?"B06_THUMB_STACK_FAIL\n":"B06_THUMB_STACK_PASS\n");semi(0x20,a);for(;;){}}
static void fault(void){stop(3);}
void reset(void){
 __asm__ volatile("cpsid i":::"memory");
 volatile unsigned char *guard=(volatile unsigned char*)&test_guard;
 for(po_u i=0;i<16;i++)guard[i]=0xa5;
 for(po_u n=0;n<=128;n++){
  po_u result=99,old_sp,new_sp;__asm__ volatile("mov %0,sp":"=r"(old_sp));
  if(ts_probe(n,&result)!=1)stop(1);
  __asm__ volatile("mov %0,sp":"=r"(new_sp));
  po_u want=((po_u)&test_top-n)&~7u;
  if(result!=want||result<(po_u)&test_low||(result&7)||old_sp!=new_sp)stop(1);
  for(po_u i=0;i<16;i++)if(guard[i]!=0xa5)stop(1);
 }
 po_u invalid[3]={129,136,~0u};
 for(po_u i=0;i<3;i++){po_u result=99;if(ts_probe(invalid[i],&result)!=0||result!=99)stop(1);}
 stop(0);
}
__attribute__((section(".vectors"),used)) const void*v[]={&stack_top,reset,fault,fault,fault,fault,fault,0,0,0,0,fault,fault,0,fault,fault};
