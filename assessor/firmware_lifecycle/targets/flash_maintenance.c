#include "flash_maintenance.h"
#ifndef FM_MUTANT
#define FM_MUTANT 0
#endif
#define HCR 0x5200200Cu
#define HSR 0x52002010u
#define HCCR 0x52002014u
#define HKEY 0x52002004u
#define FCR 0x40023C10u
#define FSR 0x40023C0Cu
#define FKEY 0x40023C04u
#define HERR 0x07EE0000u
#define FERR 0x1F2u
static int valid_io(FMIO io){return io.bus.read&&io.bus.write&&io.bus.sync&&io.fresh;}
static uint32_t read4(const unsigned char*p){return (uint32_t)p[0]|(uint32_t)p[1]<<8|(uint32_t)p[2]<<16|(uint32_t)p[3]<<24;}
static void snapshot(FMIO io,uint32_t base,unsigned char out[128]){for(unsigned i=0;i<32;++i){uint32_t v=io.bus.read(io.bus.ctx,base+4*i);for(unsigned b=0;b<4;++b)out[4*i+b]=(unsigned char)(v>>(8*b));}}
static enum FHResult begin(FMOperation*o,FMIO io,unsigned profile,unsigned kind,uint32_t address,uint32_t span,uint32_t value,unsigned sector){
 if(!o||o->state||!valid_io(io))return FH_REJECT;
 unsigned h=profile==FM_H745;uint32_t craddr=h?HCR:FCR,sraddr=h?HSR:FSR,lock=h?1u:0x80000000u;
 uint32_t sr=io.bus.read(io.bus.ctx,sraddr),cr=io.bus.read(io.bus.ctx,craddr);
 if((sr&(h?15u:0x10000u))||(cr&~(h?0x31u:0x80000300u)))return FH_REJECT;
 o->io=io;o->address=address;o->span=span;o->checked=0;o->control=cr;
 o->value=value;o->profile=profile;o->kind=kind;o->state=1;
 if(cr&lock){io.bus.write(io.bus.ctx,h?HKEY:FKEY,0x45670123u);io.bus.write(io.bus.ctx,h?HKEY:FKEY,0xCDEF89ABu);if(io.bus.read(io.bus.ctx,craddr)&lock){o->state=4;return FH_ERROR;}}
 io.bus.write(io.bus.ctx,h?HCCR:FSR,h?(HERR|0x10000u):(FERR|1u));
 uint32_t active=h?(cr&0x30u):0x200u; /* F439 x32 at qualified 2.7..3.6V. */
 if(kind==1){active|=h?(4u|(sector<<8)):(2u|(sector<<3));io.bus.write(io.bus.ctx,craddr,active);io.bus.write(io.bus.ctx,craddr,active|(h?0x80u:0x10000u));}
 else {io.bus.write(io.bus.ctx,craddr,active|1u);io.bus.write(io.bus.ctx,address,value);}
 io.bus.sync(io.bus.ctx);return FH_PENDING;
}
enum FHResult fm_erase(FMOperation*o,FMIO io,enum FMProfile p,unsigned s){
 if(p==FM_H745){if(s<6||s>7)return FH_REJECT;return begin(o,io,p,1,0x08000000u+s*0x20000u,0x20000u,UINT32_MAX,s);}
 if(p==FM_F439){if(s<10||s>11)return FH_REJECT;return begin(o,io,p,1,0x080C0000u+(s-10)*0x20000u,0x20000u,UINT32_MAX,s);}
 return FH_REJECT;
}
enum FHResult fm_f439_program(FMOperation*o,FMIO io,uint32_t a,uint32_t v){
 if((a&3u)||a<0x080C0000u||a>0x080FFFFCu)return FH_REJECT;
 return begin(o,io,FM_F439,2,a,4,v,0);
}
enum FHResult fm_poll(FMOperation*o){
 if(!o||(o->state!=1&&o->state!=2))return FH_REJECT;
 FHIO io=o->io.bus;unsigned h=o->profile==FM_H745;uint32_t sraddr=h?HSR:FSR,craddr=h?HCR:FCR,err=h?HERR:FERR;
 if(o->state==1){uint32_t sr=io.read(io.ctx,sraddr);if(FM_MUTANT!=1&&(sr&(h?7u:0x10000u)))return FH_PENDING;
  if(h&&!(sr&(HERR|0x10000u)))return FH_PENDING;
  io.sync(io.ctx);io.write(io.ctx,craddr,o->control&~(h?1u:0x80000000u));if(o->control&(h?1u:0x80000000u))io.write(io.ctx,craddr,o->control);io.sync(io.ctx);
  if(sr&err){o->state=4;return FH_ERROR;}o->io.fresh(io.ctx,o->address,o->span);o->state=2;
 }
 /* Bounded progress: at most32 physical reads per call, no heap or helpers. */
 for(unsigned n=0;n<32&&o->checked<o->span;n++,o->checked+=4){if(FM_MUTANT!=2&&io.read(io.ctx,o->address+o->checked)!=o->value){o->state=4;return FH_ERROR;}}
 if(io.read(io.ctx,sraddr)&err){o->state=4;return FH_ERROR;}
 if(o->checked==o->span){o->state=3;return FH_DONE;}return FH_PENDING;
}
int fm_recover(const unsigned char a[128],const unsigned char b[128]){
 int va=fh_record_valid(a),vb=fh_record_valid(b);if(!va)return vb?2:0;if(!vb)return 1;
 uint32_t sa=read4(a+4),sb=read4(b+4);if(sa!=sb)return sa>sb?1:2;
 for(unsigned i=0;i<128;++i)if(a[i]!=b[i])return -1;return 1;
}
enum FHResult fm_recycle_begin(FMRecycle*r,FMIO io,uint32_t old,const FWPeer*p,uint32_t n,const unsigned char h[64],const uint32_t v[2],const uint32_t ver[2]){
 if(!r||r->state||!valid_io(io)||(old!=0x080C0000u&&old!=0x080E0000u))return FH_REJECT;
 unsigned char prior[128],record[128];io.fresh(io.bus.ctx,old,128);snapshot(io,old,prior);
 if(!fh_record_valid(prior)||n<=read4(prior+4)||!fh_record(p,n,h,v,ver,record))return FH_REJECT;
 r->io=io;r->old_base=old;r->new_base=old^0x20000u;r->state=1;r->erase.state=0;
 for(unsigned i=0;i<128;++i)r->record[i]=record[i];
 enum FHResult e=fm_erase(&r->erase,io,FM_H745,(r->new_base-0x08000000u)/0x20000u);if(e!=FH_PENDING)r->state=5;return e;
}
enum FHResult fm_recycle_poll(FMRecycle*r){
 if(!r||r->state<1||r->state>3)return FH_REJECT;
 enum FHResult e;
 if(r->state==1){e=fm_poll(&r->erase);if(e==FH_PENDING)return e;if(e!=FH_DONE){r->state=5;return e;}
  /* The already validated record is copied exactly. Program each virgin word
   * once; append publishes its separately programmed commit word last. */
  r->append.io=r->io.bus;r->append.base=r->new_base;r->append.state=1;r->append.word=0;r->append.session.active=0;
  for(unsigned i=0;i<128;++i)r->append.record[i]=r->record[i];
  uint32_t w[8];for(unsigned i=0;i<8;++i)w[i]=read4(r->record+4*i);e=fh_start(&r->append.session,r->io.bus,r->new_base,w);if(e!=FH_PENDING){r->state=5;return e;}r->state=2;return FH_PENDING;
 }
 if(r->state==2){e=fh_append_poll(&r->append);if(e==FH_PENDING)return e;if(e!=FH_DONE){r->state=5;return e;}
  unsigned char check[128];r->io.fresh(r->io.bus.ctx,r->new_base,128);snapshot(r->io,r->new_base,check);
  if(FM_MUTANT!=3){for(unsigned i=0;i<128;++i)if(check[i]!=r->record[i]){r->state=5;return FH_ERROR;}if(!fh_record_valid(check)){r->state=5;return FH_ERROR;}}
  r->erase.state=0;e=fm_erase(&r->erase,r->io,FM_H745,(r->old_base-0x08000000u)/0x20000u);if(e!=FH_PENDING){r->state=5;return e;}r->state=3;return FH_PENDING;
 }
 e=fm_poll(&r->erase);if(e==FH_DONE)r->state=4;else if(e!=FH_PENDING)r->state=5;return e;
}
