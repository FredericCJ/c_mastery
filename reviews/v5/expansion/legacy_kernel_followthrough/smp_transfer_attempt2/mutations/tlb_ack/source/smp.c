#include "cmk2.h"
#define ALIGN(n) __attribute__((aligned(n)))
#define PACKED __attribute__((packed))
extern unsigned char cmk2_ap_start[],cmk2_ap_end[];
extern uint64_t boot_pml4[];
extern void cmk2_timer(void),cmk2_ipi(void),cmk2_tlb(void),cmk2_spurious(void),cmk2_fault(void);
extern void *isr_table[];
extern void isr32(void),isr43(void),isr128(void),isr241(void);
extern void *memset(void *,int,size_t);
extern void *memcpy(void *,const void *,size_t);
static struct cmk2_topology topology;
static volatile uint32_t *lapic;
struct tss { uint32_t r0; uint64_t rsp[3],r1,ist[7],r2; uint16_t r3,iomap; } PACKED;
struct gate { uint16_t lo,selector; uint8_t ist,type; uint16_t mid; uint32_t hi,zero; } PACKED;
struct dt { uint16_t limit; uint64_t base; } PACKED;
struct cpu {
  unsigned online,done,timers,ipis,switches;
  uint64_t *idle;
  unsigned current;
  uint64_t gdt[7]; struct tss tss; struct gate idt[256];
  unsigned char boot[16384] ALIGN(16), irq[8192] ALIGN(16), tlb_stack[4096] ALIGN(16);
} ALIGN(64);
struct thread {
  unsigned process,slot,cpu,state,blocks; uint64_t *sp;
  unsigned char stack[8192] ALIGN(16);
} ALIGN(64);
struct process { unsigned id,live_threads; };
static struct cpu cpus[CMK2_MAX_CPUS];
static struct thread threads[CMK2_MAX_THREADS];
static struct process processes[CMK2_MAX_CPUS];
static struct cmk2_channel channel;
static unsigned lock_word,start,producers,received,waits,remote_wakes;
static unsigned user_online,shoot_generation,shoot_ack[CMK2_MAX_CPUS],user_start;
static void (*user_entry)(unsigned);
static unsigned char seen[CMK2_MAX_CPUS*CMK2_MESSAGES_PER_CPU];
static void outb(unsigned short port,unsigned char v){__asm__ volatile("outb %0,%1"::"a"(v),"Nd"(port));}
static unsigned char inb(unsigned short port){unsigned char v;__asm__ volatile("inb %1,%0":"=a"(v):"Nd"(port));return v;}
static void text(const char *s){while(*s){while(!(inb(0x3fd)&32))__asm__ volatile("pause");outb(0x3f8,(unsigned char)*s++);}}
static void number(unsigned n){char b[12];unsigned i=0;do{b[i++]=(char)('0'+n%10);n/=10;}while(n);while(i){char x[2]={b[--i],0};text(x);}}
static __attribute__((noreturn)) void fail(const char *s){text("FAIL SMP: ");text(s);text("\n");unsigned v=17;__asm__ volatile("outl %0,$0xf4"::"a"(v));for(;;)__asm__ volatile("cli;hlt");}
static void pause_cpu(void){__asm__ volatile("pause":::"memory");}
static void acquire(void){while(__atomic_exchange_n(&lock_word,1,__ATOMIC_ACQUIRE))pause_cpu();}
static void release(void){__atomic_store_n(&lock_word,0,__ATOMIC_RELEASE);}
unsigned cmk2_cpu_index(void){unsigned id=lapic[0x20/4]>>24;for(unsigned i=0;i<topology.count;i++)if(topology.ids[i]==id)return i;fail("unknown APIC id");}
static uint32_t rd32(const unsigned char *p){return (uint32_t)p[0]|(uint32_t)p[1]<<8|(uint32_t)p[2]<<16|(uint32_t)p[3]<<24;}
static uint64_t rd64(const unsigned char *p){return rd32(p)|(uint64_t)rd32(p+4)<<32;}
static int sum_ok(const unsigned char *p,unsigned n){unsigned s=0;for(unsigned i=0;i<n;i++)s+=p[i];return !(s&255);}
static const unsigned char *table(uint64_t address){
  if(address<0x1000 || address>UINT32_MAX-65536)fail("ACPI pointer");
  const unsigned char *p=(const unsigned char *)(uintptr_t)address;unsigned n=rd32(p+4);
  if(n<36 || n>65536 || !sum_ok(p,n))fail("ACPI table length/checksum");return p;
}
static void discover(unsigned info){
  const unsigned char *p=(const unsigned char *)(uintptr_t)info,*rsdp=0;unsigned n=rd32(p);
  if(n<16 || n>1048576)fail("multiboot length");
  for(unsigned at=8;at+8<=n;){unsigned kind=rd32(p+at),z=rd32(p+at+4);if(z<8||z>n-at)fail("tag bounds");
    if(kind==14 && z>=28 && !rsdp)rsdp=p+at+8;
    if(kind==15 && z>=44)rsdp=p+at+8;
    if(!kind)break;at=(at+z+7)&~7u;
  }
  if(!rsdp||!sum_ok(rsdp,20))fail("RSDP missing/checksum");
  unsigned stride=4;uint64_t root=rd32(rsdp+16);
  if(rsdp[15]>=2){if(rd32(rsdp+20)!=36||!sum_ok(rsdp,36))fail("extended RSDP");root=rd64(rsdp+24);stride=8;}
  const unsigned char *r=table(root);unsigned z=rd32(r+4);
  if((stride==4&&(r[0]!='R'||r[1]!='S'||r[2]!='D'||r[3]!='T')) ||
     (stride==8&&(r[0]!='X'||r[1]!='S'||r[2]!='D'||r[3]!='T')) || (z-36)%stride)fail("root ACPI type");
  for(unsigned at=36;at<z;at+=stride){const unsigned char *t=table(stride==4?rd32(r+at):rd64(r+at));
    if(t[0]=='A'&&t[1]=='P'&&t[2]=='I'&&t[3]=='C'){
      if(cmk2_madt_parse(t,rd32(t+4),&topology))fail("MADT unsupported/malformed");
      lapic=(volatile uint32_t *)(uintptr_t)topology.lapic;return;
    }
  }fail("MADT missing");
}
static void local_tables(unsigned index){
  struct cpu *c=&cpus[index];
  c->gdt[1]=UINT64_C(0x00af9a000000ffff);c->gdt[2]=UINT64_C(0x00cf92000000ffff);
  c->gdt[3]=UINT64_C(0x00affa000000ffff);c->gdt[4]=UINT64_C(0x00cff2000000ffff);
  uint64_t p=(uintptr_t)&c->tss,lim=sizeof c->tss-1;c->tss.iomap=sizeof c->tss;
  c->tss.ist[0]=(uintptr_t)(c->irq+sizeof c->irq);
  c->tss.ist[1]=(uintptr_t)(c->tlb_stack+sizeof c->tlb_stack);
  c->gdt[5]=(lim&65535)|((p&0xffffff)<<16)|(UINT64_C(0x89)<<40)|((lim&0xf0000)<<32)|((p&UINT64_C(0xff000000))<<32);c->gdt[6]=p>>32;
  struct dt gd={sizeof c->gdt-1,(uintptr_t)c->gdt};__asm__ volatile("lgdt %0"::"m"(gd));unsigned short sel=40;__asm__ volatile("ltr %0"::"r"(sel));
  for(unsigned i=0;i<256;i++){void(*f)(void)=i==240?cmk2_timer:i==241?cmk2_ipi:i==255?cmk2_spurious:cmk2_fault;uint64_t a=(uintptr_t)f;
    c->idt[i]=(struct gate){a&65535,8,1,0x8e,(a>>16)&65535,(uint32_t)(a>>32),0};}
  struct dt id={sizeof c->idt-1,(uintptr_t)c->idt};__asm__ volatile("lidt %0"::"m"(id));
  unsigned a,d;__asm__ volatile("rdmsr":"=a"(a),"=d"(d):"c"(0x1b));a|=0x800;__asm__ volatile("wrmsr"::"a"(a),"d"(d),"c"(0x1b));
  lapic[0xf0/4]=0x100|255;lapic[0x320/4]=1<<16;lapic[0x350/4]=1<<16;lapic[0x360/4]=1<<16;
}
static void ipi(unsigned id,unsigned command){
  for(unsigned i=0;lapic[0x300/4]&(1<<12);i++)if(i==10000000)fail("IPI busy timeout");
  lapic[0x310/4]=id<<24;lapic[0x300/4]=command;
  for(unsigned i=0;lapic[0x300/4]&(1<<12);i++)if(i==10000000)fail("IPI delivery timeout");
}
static void timer_on(void){lapic[0x3e0/4]=3;lapic[0x320/4]=(1<<17)|240;lapic[0x380/4]=100000;}
void cmk2_interrupt(unsigned vector){unsigned c=cmk2_cpu_index();if(vector==240)__atomic_fetch_add(&cpus[c].timers,1,__ATOMIC_RELAXED);else if(vector==241)__atomic_fetch_add(&cpus[c].ipis,1,__ATOMIC_RELAXED);else if(vector==242){uint64_t cr3;__asm__ volatile("mov %%cr3,%0":"=r"(cr3));__asm__ volatile("mov %0,%%cr3"::"r"(cr3):"memory");/* mutation: omit remote acknowledgement */}else if(vector!=255)fail("unexpected exception/interrupt");if(vector!=255)lapic[0xb0/4]=0;}
static void wake(unsigned state,unsigned own){for(unsigned i=0;i<topology.count*2;i++)if(threads[i].state==state){threads[i].state=CMK2_READY;if(threads[i].cpu!=own){remote_wakes++;ipi(topology.ids[threads[i].cpu],241);}}}
static void yield_thread(struct thread *t){unsigned c=t->cpu;cmk2_switch(&t->sp,cpus[c].idle);}
static void thread_entry(void){
  unsigned core=cmk2_cpu_index();struct thread *t=&threads[cpus[core].current];
  if(t->slot==0){
    for(unsigned i=0;i<CMK2_MESSAGES_PER_CPU;){
      acquire();struct cmk2_message m={core*CMK2_MESSAGES_PER_CPU+i,(core*CMK2_MESSAGES_PER_CPU+i)^UINT32_C(0xa53c781f)};
      int r=cmk2_channel_send(&channel,m);
      if(r==CMK2_FULL){t->state=CMK2_WAIT_SEND;t->blocks++;waits++;}
      else if(r==CMK2_OK){i++;wake(CMK2_WAIT_RECV,core);t->state=CMK2_RUNNING;}
      else fail("send contract");
      if(r==CMK2_OK && (i&15)==0)t->state=CMK2_READY;
      release();if(r==CMK2_FULL || (i&15)==0)yield_thread(t);
    }
    acquire();producers++;if(producers==topology.count)cmk2_channel_close(&channel);wake(CMK2_WAIT_RECV,core);release();
  }else{
    for(;;){acquire();struct cmk2_message m;int r=cmk2_channel_receive(&channel,&m);
      if(r==CMK2_CLOSED){release();break;}
      if(r==CMK2_EMPTY){t->state=CMK2_WAIT_RECV;t->blocks++;waits++;}
      else if(r==CMK2_OK){if(m.sequence>=topology.count*CMK2_MESSAGES_PER_CPU||m.value!=(m.sequence^UINT32_C(0xa53c781f))||seen[m.sequence])fail("message corruption/duplicate");seen[m.sequence]=1;received++;wake(CMK2_WAIT_SEND,core);t->state=CMK2_READY;}
      else fail("receive contract");release();yield_thread(t);
    }
  }
  acquire();t->state=CMK2_ZOMBIE;processes[t->process].live_threads--;release();yield_thread(t);fail("zombie resumed");
}
static void scheduler(unsigned c){
  unsigned next=0,spins=0;
  for(;;){acquire();unsigned chosen=CMK2_MAX_THREADS,live=0;
    for(unsigned j=0;j<2;j++){unsigned i=c*2+(next+j)%2;if(threads[i].state!=CMK2_ZOMBIE)live++;if(chosen==CMK2_MAX_THREADS&&threads[i].state==CMK2_READY)chosen=i;}
    if(!live){release();break;}
    if(chosen==CMK2_MAX_THREADS){release();if(++spins==100000000)fail("scheduler stalled");pause_cpu();continue;}
    spins=0;threads[chosen].state=CMK2_RUNNING;cpus[c].current=chosen;cpus[c].switches++;next=(chosen+1)%2;release();
    cmk2_switch(&cpus[c].idle,threads[chosen].sp);
  }
}
static void worker(unsigned c){
  while(!__atomic_load_n(&start,__ATOMIC_ACQUIRE))pause_cpu();
  timer_on();__asm__ volatile("sti");scheduler(c);
  /* Each CPU must service both an actual local timer interrupt and an IPI. */
  for(unsigned spins=0;!__atomic_load_n(&cpus[c].timers,__ATOMIC_RELAXED)||!__atomic_load_n(&cpus[c].ipis,__ATOMIC_RELAXED);spins++){if(spins==100000000)fail("timer/IPI not observed");pause_cpu();}
  __asm__ volatile("cli");lapic[0x320/4]=1<<16;__atomic_store_n(&cpus[c].done,1,__ATOMIC_RELEASE);
}
static void ap_entry(unsigned c){local_tables(c);timer_on();__asm__ volatile("sti");__atomic_store_n(&cpus[c].online,1,__ATOMIC_RELEASE);worker(c);for(;;)__asm__ volatile("cli;hlt");}
static void delay(void){/* PIT channel 2 one-shot, approximately 10 ms; boot only. */
  unsigned char old=inb(0x61);outb(0x61,(old&~2u)|1);outb(0x43,0xb0);outb(0x42,11932&255);outb(0x42,11932>>8);
  for(unsigned n=0;!(inb(0x61)&32);n++)if(n==10000000)fail("PIT boot delay");outb(0x61,old);
}
void cmk2_smp_run(unsigned info){
  _Static_assert(__atomic_always_lock_free(sizeof(unsigned),0),"requires native atomic unsigned");
  discover(info);unsigned bsp=cmk2_cpu_index();if(bsp){unsigned char id=topology.ids[0];topology.ids[0]=topology.ids[bsp];topology.ids[bsp]=id;}
  text("PASS smp-madt cpus=");number(topology.count);text("\n");
  outb(0x21,255);outb(0xa1,255);local_tables(0);cpus[0].online=1;
  for(unsigned c=0;c<topology.count;c++){processes[c]=(struct process){c,2};for(unsigned j=0;j<2;j++){
    struct thread *t=&threads[c*2+j];t->cpu=c;t->slot=j;t->process=c;t->state=CMK2_READY;
    *(uint64_t *)(void *)t->stack=UINT64_C(0xfacedeadbad0cafe);
    uint64_t *sp=(uint64_t *)(void *)(t->stack+sizeof t->stack-8);*--sp=(uintptr_t)thread_entry;for(unsigned k=0;k<6;k++)*--sp=0;t->sp=sp;
  }}
  size_t code=(size_t)(cmk2_ap_end-cmk2_ap_start);if(code>0xf00)fail("trampoline too large");memcpy((void *)(uintptr_t)0x8000,cmk2_ap_start,code);
  for(unsigned c=1;c<topology.count;c++){
    volatile uint64_t *m=(volatile uint64_t *)(uintptr_t)0x8f00;m[0]=(uintptr_t)boot_pml4;m[1]=(uintptr_t)(cpus[c].boot+sizeof cpus[c].boot);m[2]=(uintptr_t)ap_entry;m[3]=c;__atomic_thread_fence(__ATOMIC_SEQ_CST);
    ipi(topology.ids[c],0xc500);delay();ipi(topology.ids[c],0x8500);delay();ipi(topology.ids[c],0x608);delay();if(!__atomic_load_n(&cpus[c].online,__ATOMIC_ACQUIRE))ipi(topology.ids[c],0x608);
    for(unsigned spin=0;!__atomic_load_n(&cpus[c].online,__ATOMIC_ACQUIRE);spin++){if(spin==100000000)fail("AP startup timeout");pause_cpu();}
  }
  text("PASS smp-startup-percpu\n");
  timer_on();__asm__ volatile("sti");
  for(unsigned c=0;c<topology.count;c++)ipi(topology.ids[c],241);
  for(unsigned c=0;c<topology.count;c++)for(unsigned spin=0;!__atomic_load_n(&cpus[c].timers,__ATOMIC_RELAXED)||!__atomic_load_n(&cpus[c].ipis,__ATOMIC_RELAXED);spin++){if(spin==100000000)fail("initial APIC evidence timeout");pause_cpu();}
  text("PASS smp-apic-timer-ipi\n");
  __atomic_store_n(&start,1,__ATOMIC_RELEASE);worker(0);
  for(unsigned c=1;c<topology.count;c++)for(unsigned n=0;!__atomic_load_n(&cpus[c].done,__ATOMIC_ACQUIRE);n++){if(n==100000000)fail("AP completion timeout");pause_cpu();}
  if(received!=topology.count*CMK2_MESSAGES_PER_CPU||producers!=topology.count)fail("message totals");
  for(unsigned c=0;c<topology.count;c++){if(processes[c].live_threads||!cpus[c].switches)fail("thread lifetime");text("CPU ");number(c);text(" switches=");number(cpus[c].switches);text(" timers=");number(cpus[c].timers);text(" ipis=");number(cpus[c].ipis);text("\n");}
  for(unsigned i=0;i<topology.count*2;i++)if(*(uint64_t *)(void *)threads[i].stack!=UINT64_C(0xfacedeadbad0cafe))fail("thread stack canary");
  text("PASS smp-apic-timer-ipi\nPASS smp-cooperative-threads\nPASS smp-bounded-channel\nPASS smp-message-accounting\n");
  text("SMP messages=");number(received);text(" waits=");number(waits);text(" remote-wakes=");number(remote_wakes);text("\n");
  /* APs remain stopped. The independent v1 user-process regression continues
   * on the BSP; its success is not labelled SMP userspace execution. */
  if(!waits)fail("no blocked-wakeup observation");
  text("PASS smp-suite\n");
  lapic[0x380/4]=0;
  __asm__ volatile("sti");for(unsigned drain=0;drain<10000;drain++)pause_cpu();__asm__ volatile("cli");
  lapic[0x350/4]=0x700; /* BSP legacy PIC virtual wire for v1 regression */
}

unsigned cmk2_cpu_count(void){return topology.count;}
void cmk2_eoi(void){lapic[0xb0/4]=0;}
void cmk2_wake_cpu(unsigned c){if(c<topology.count)ipi(topology.ids[c],241);}
void cmk2_flush_remote(void){
 unsigned me=cmk2_cpu_index(),mask=__atomic_load_n(&user_online,__ATOMIC_ACQUIRE);
 unsigned gen=__atomic_add_fetch(&shoot_generation,1,__ATOMIC_RELEASE);
 uint64_t cr3;__asm__ volatile("mov %%cr3,%0":"=r"(cr3));__asm__ volatile("mov %0,%%cr3"::"r"(cr3):"memory");
 for(unsigned c=0;c<topology.count;c++)if(c!=me&&(mask&(1u<<c)))ipi(topology.ids[c],242);
 for(unsigned c=0;c<topology.count;c++)if(c!=me&&(mask&(1u<<c)))for(unsigned n=0;__atomic_load_n(&shoot_ack[c],__ATOMIC_ACQUIRE)!=gen;n++){if(n==100000000)fail("TLB shootdown deadline");pause_cpu();}
}
static void user_tables(unsigned c){
 local_tables(c);struct cpu *p=&cpus[c];p->tss.rsp[0]=(uintptr_t)(p->boot+sizeof p->boot);
 for(unsigned i=0;i<256;i++){
  void(*f)(void)=i<32?(void(*)(void))isr_table[i]:i==32?isr32:i==43?isr43:i==128?isr128:i==241?isr241:i==242?cmk2_tlb:cmk2_spurious;
  uint64_t a=(uintptr_t)f;p->idt[i]=(struct gate){a&65535,8,(uint8_t)(i==8?1:i==242?2:0),(uint8_t)(i==128?0xee:0x8e),(a>>16)&65535,(uint32_t)(a>>32),0};
 }
 lapic[0x3e0/4]=3;lapic[0x320/4]=(1<<17)|32;lapic[0x380/4]=625000;
 __atomic_fetch_or(&user_online,1u<<c,__ATOMIC_RELEASE);
}
static void user_ap_entry(unsigned c){user_tables(c);__atomic_store_n(&cpus[c].online,1,__ATOMIC_RELEASE);while(!__atomic_load_n(&user_start,__ATOMIC_ACQUIRE))pause_cpu();user_entry(c);fail("user AP returned");}
void cmk2_start_user_aps(void (*entry)(unsigned)){
 user_entry=entry;user_tables(0);outb(0x21,255);outb(0xa1,255);
 for(unsigned c=1;c<topology.count;c++){
  __atomic_store_n(&cpus[c].online,0,__ATOMIC_RELEASE);
  volatile uint64_t *m=(volatile uint64_t *)(uintptr_t)0x8f00;m[0]=(uintptr_t)boot_pml4;m[1]=(uintptr_t)(cpus[c].boot+sizeof cpus[c].boot);m[2]=(uintptr_t)user_ap_entry;m[3]=c;__atomic_thread_fence(__ATOMIC_SEQ_CST);
  ipi(topology.ids[c],0xc500);delay();ipi(topology.ids[c],0x8500);delay();ipi(topology.ids[c],0x608);delay();if(!__atomic_load_n(&cpus[c].online,__ATOMIC_ACQUIRE))ipi(topology.ids[c],0x608);
  for(unsigned spin=0;!__atomic_load_n(&cpus[c].online,__ATOMIC_ACQUIRE);spin++){if(spin==100000000)fail("user AP startup timeout");pause_cpu();}
 }
 __atomic_store_n(&user_start,1,__ATOMIC_RELEASE);
}
