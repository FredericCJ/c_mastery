#include "boot_storage.h"
#include "monocypher-ed25519.h"
extern const unsigned char app_start[],app_end[];
static int read_flash(void*unused,uint32_t a,uint32_t*out){(void)unused;return eg_read_word(a,out);}
static void put(unsigned char*p,uint32_t n){for(unsigned i=0;i<4;++i)p[i]=(unsigned char)(n>>(8*i));}
static void emit(const char*s){register unsigned r0 __asm__("r0")=4;register const char*r1 __asm__("r1")=s;__asm__ volatile("bkpt 0xab":"+r"(r0):"r"(r1):"memory");}
static _Noreturn void bad(void){emit("BS_GUEST_FAIL\n");for(;;)__asm__ volatile("wfi");}
void fb_guest_main(void){
 /* Original MPS2 RAM-backed teaching slots, not STM32 flash programming. */
 *(volatile uint32_t*)(uintptr_t)0xE000ED24u|=1u<<17;
 uint32_t sentinel=0xA5A5A5A5u;if(eg_read_word(0xFFFFFFF0u,&sentinel)!=0||sentinel!=0xA5A5A5A5u)bad();
 struct bs_layouts choices={.core={{{0x40000,0x42000,0x20000000,0x20020000},{0x50000,0x52000,0x20000000,0x20020000}},{{0x60000,0x62000,0x20000000,0x20020000},{0x70000,0x72000,0x20000000,0x20020000}}}};
 struct fb_policy policy={0};unsigned char seed[32]={0},secret[64];crypto_ed25519_key_pair(secret,policy.keys[0],seed);policy.present=2;policy.floor[0]=policy.floor[1]=1;policy.generation_floor=1;struct fb_image images[2];
 for(unsigned core=0;core<2;++core){uint32_t base=choices.core[core][0].base;unsigned char*header=(unsigned char*)(uintptr_t)(base-32);for(unsigned i=0;i<32;++i)header[i]=0;uint32_t extent=core?576u:(uint32_t)((uintptr_t)app_end-(uintptr_t)app_start);if(core){for(unsigned i=0;i<576;++i)header[32+i]=0;put(header+32,0x2001F000);put(header+36,base+513);}put(header,0x31423646u);header[4]=1;header[5]=(unsigned char)core;put(header+8,1);put(header+12,1);put(header+16,base);put(header+20,extent);crypto_ed25519_sign((unsigned char*)(uintptr_t)(base+2048),secret,header,32+extent);if(!bs_read_image(read_flash,0,&policy,choices.core[core][0],core,&images[core]))bad();}
 struct fw_peer peer={.generation=1};if(fw_peer_step(&peer,0,1,FW_HEALTHY)!=H7_OK||fw_peer_step(&peer,1,1,FW_HEALTHY)!=H7_OK||fw_peer_step(&peer,0,1,FW_CONFIRM)!=H7_OK)bad();
 const uint32_t record_bases[2]={0x80000,0xA0000};if(!fb_joint_record(images,&peer,1,(unsigned char*)(uintptr_t)record_bases[0]))bad();for(unsigned i=0;i<128;++i)((unsigned char*)(uintptr_t)record_bases[1])[i]=0;
 struct bs_selection selected;unsigned char*changed=(unsigned char*)(uintptr_t)(0x60000+513);*changed^=1;if(bs_select(read_flash,0,&policy,&choices,record_bases,&selected)!=0)bad();*changed^=1;
 if(bs_select(read_flash,0,&policy,&choices,record_bases,&selected)!=1||selected.sequence!=1||selected.record!=0)bad();emit("BS_GUEST_PASS:guarded physical reads,signature policy,paired journal,then handoff\n");fb_handoff(selected.image[0].base,selected.image[0].stack,selected.image[0].entry);
}
