#include "boot_program_mode.h"
#include "selected_startup.h"
#include "trial_health.h"
#ifndef ST_CORE
#define ST_CORE 0
#endif
#define WORD(a) (*(volatile uint32_t*)(uintptr_t)(a))
static struct bs_selection prior,selected;
static unsigned live_trial;
static _Noreturn void stop(void){for(;;)__asm__ volatile("wfi");}
static int read_slot(void*c,uint32_t a,uint32_t*v){(void)c;return eg_read_word(a,v);}
/* RFC8032 TEST1 PUBLIC teaching key. Private signing is never linked here.
 * Deployment requires its own authenticated provisioning; no real key claimed. */
static const struct fb_policy policy={.keys={{0xd7,0x5a,0x98,0x01,0x82,0xb1,0x0a,0xb7,0xd5,0x4b,0xfe,0xd3,0xc9,0x64,0x07,0x3a,0x0e,0xe1,0x72,0xf3,0xda,0xa6,0x23,0x25,0xaf,0x02,0x1a,0x68,0xf7,0x07,0x51,0x1a}},.present=2,.floor={1,1},.generation_floor=1};
static const struct bs_layouts layout={.core={{{0x08020200,0x08022000,0x20000000,0x20020000},{0x08040200,0x08042000,0x20000000,0x20020000}},{{0x08060200,0x08062000,0x30000000,0x30020000},{0x08080200,0x08082000,0x30000000,0x30020000}}}};

static enum fw_peer_result peer_step(uint32_t g,enum fw_peer_op op){enum fw_peer_result z;unsigned n=1000000;do{z=fw_h745_step(g,op);}while(z==H7_BUSY&&--n);return z;}
#if ST_CORE==0
static struct rt_frame frame;static struct fu_install install;static struct ft_writer attempt;static struct fm_recycle commit;
static void receive_pair(struct fh_io io){struct fb_image installed[2];for(unsigned core=0;core<2;++core){rt_reset(&frame);unsigned left=10000000;uint32_t status=0;int z=0;while(left--){z=rt_uart_step(io,&frame,&status);if(z<0||z==2)break;}if(z!=2||frame.bytes[5]!=core)stop();install.state=0;enum fh_result e=fu_begin(&install,fm_h745_io(),read_slot,0,&frame,&policy,&layout,&prior);left=40000000;while(e==FH_PENDING&&left--)e=fu_poll(&install,&installed[core]);if(e!=FH_DONE)stop();if(rt_uart_tx(io,5,100000)!=1)stop();}
 enum fh_result e=ft_begin(&attempt,fm_h745_io(),&prior,installed);unsigned left=40000000;while(e==FH_PENDING&&left--)e=ft_poll(&attempt);if(e!=FH_DONE||!ft_probe(read_slot,0,&policy,&layout,&prior,&selected))stop();live_trial=1;
}
#endif
_Noreturn void st_main(void){struct fh_io io=st_target_io();
#if ST_CORE==0
 if(!st_cold_h745(io))stop();
#else
 if(WORD(0xE000ED94u)||(WORD(0xE000ED14u)&0x30000u))stop();
#endif
 WORD(0xE000ED24u)|=1u<<17;__asm__ volatile("dsb sy\n\tisb sy":::"memory");const uint32_t records[2]={0x080C0000,0x080E0000};if(bs_select(read_slot,0,&policy,&layout,records,&prior)!=1)stop();
#if ST_CORE==0
 /* Primary reset enters this trusted bank2 service before any peer release.
  * One exact 'M' byte enters maintenance. Silence boots the committed pair;
  * malformed input fails closed. This local mode choice grants no signing key. */
 if(rt_uart_init(io,100000)!=1)stop();unsigned left=1000000;unsigned char byte=0;uint32_t status=0;int received=0;while(left--&&(received=rt_uart_rx(io,&byte,&status))==0){}if(received<0)stop();if(received==1){if(byte!='M')stop();receive_pair(io);}else{
  selected.sequence=prior.sequence;selected.record=prior.record;for(unsigned c=0;c<2;++c)selected.image[c]=prior.image[c];
 }
 /* No UART DMA or IRQ was enabled. Disable the polled UART before handoff. */
 WORD(0x40004800u)=0;__asm__ volatile("dsb sy":::"memory");
 WORD(0x30040034u)=0;WORD(0x30040038u)=0;__asm__ volatile("dsb sy":::"memory");if(!st_release(io,&selected))stop();left=1000000;while(WORD(0x30040034u)!=selected.image[0].generation&&--left){}if(!left)stop();__asm__ volatile("dmb sy":::"memory");WORD(0x30040030u)=selected.image[0].generation;__asm__ volatile("dsb sy":::"memory");
#else
 if(WORD(0x30040020u)==prior.sequence){selected.sequence=prior.sequence;selected.record=prior.record;for(unsigned c=0;c<2;++c)selected.image[c]=prior.image[c];}
 else{if(!ft_probe(read_slot,0,&policy,&layout,&prior,&selected))stop();live_trial=1;}
 if(!st_peer_matches(io,&selected))stop();WORD(0x30040034u)=selected.image[1].generation;__asm__ volatile("dsb sy":::"memory");unsigned left=1000000;while(WORD(0x30040030u)!=selected.image[1].generation&&--left){}if(!left)stop();__asm__ volatile("dmb sy":::"memory");
#endif
 /* Authentication acknowledgment has not set FW_HEALTHY. */
 fb_handoff(selected.image[ST_CORE].base,selected.image[ST_CORE].stack,selected.image[ST_CORE].entry);
}
_Noreturn void th_report_main(uint32_t result,uint32_t generation){
 if(!live_trial||!th_health_accept(ST_CORE,generation,selected.image[ST_CORE].generation,result))stop();
 if(peer_step(generation,FW_HEALTHY)!=H7_OK)stop();
#if ST_CORE==1
 /* Executing from bank2 now; no return to candidate bank1 instructions/data.
  * IRQs remain masked by handoff, no DMA/NMI ownership is introduced. */
 WORD(0x30040038u)=generation;__asm__ volatile("dsb sy":::"memory");stop();
#else
 unsigned left=1000000;while(WORD(0x30040038u)!=generation&&--left){}if(!left)stop();__asm__ volatile("dmb sy":::"memory");if(peer_step(generation,FW_CONFIRM)!=H7_OK)stop();
 struct fw_peer p;p.generation=WORD(0x30040000u);p.health=WORD(0x30040004u);p.failed=WORD(0x30040008u);p.outstanding[0]=WORD(0x3004000Cu);p.outstanding[1]=WORD(0x30040010u);p.confirmed=WORD(0x30040014u);p.reserved[0]=WORD(0x30040018u);p.reserved[1]=WORD(0x3004001Cu);
 /* Application handoff made flash RO; the authenticated privileged bank2 service
  * now takes exclusive programming ownership and leaves that MPU map. */
 fb_program_mode();
 enum fh_result e=th_commit_begin(&commit,fm_h745_io(),&prior,&selected,&p,WORD(0x30040038u));left=80000000;while(e==FH_PENDING&&left--)e=fm_recycle_poll(&commit);if(e!=FH_DONE)stop();
 __asm__ volatile("dsb sy":::"memory");WORD(0xE000ED0Cu)=0x05FA0004u;__asm__ volatile("dsb sy\n\tisb sy":::"memory");stop();
#endif
}
