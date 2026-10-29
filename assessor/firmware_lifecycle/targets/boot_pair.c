#include "boot_pair.h"
#include "monocypher.h"
#include "monocypher-ed25519.h"
#ifndef FB_MUTANT
#define FB_MUTANT 0
#endif
static uint32_t get(const unsigned char*p){return (uint32_t)p[0]|(uint32_t)p[1]<<8|(uint32_t)p[2]<<16|(uint32_t)p[3]<<24;}
int fb_admit(const struct fb_policy*p,struct fb_layout l,unsigned core,const unsigned char*b,size_t n,const unsigned char sig[64],struct fb_image*out){
 if(!p||!b||!sig||!out||core>1||n<32+514||n>32+2048||get(b)!=0x31423646u||b[5]!=core||b[6]||b[7])return 0;
 for(unsigned i=24;i<32;++i)if(b[i])return 0;
 unsigned key=b[4];uint32_t version=get(b+8),generation=get(b+12),base=get(b+16),extent=get(b+20),stack=get(b+32),entry=get(b+36);
 if(key<1||key>2||!(p->present&(1u<<key))||(p->revoked&(1u<<key))||version<p->floor[core]||!generation||generation<p->generation_floor)return 0;
 if(base!=l.base||(base&511u)||l.limit<=base||extent!=n-32||extent>l.limit-base||l.stack_low>=l.stack_high||(stack&7u)||stack<=l.stack_low||stack>l.stack_high)return 0;
 if(FB_MUTANT!=2&&(!(entry&1u)||(entry&~1u)<base+512u||(entry&~1u)-base>=extent))return 0;
 /* Provider returns0 for authentic, not the existing OpenSSL adapter's1. */
 if(FB_MUTANT!=1&&crypto_ed25519_check(sig,p->keys[key-1],b,n)!=0)return 0;
 struct fb_image staged;staged.version=version;staged.generation=generation;staged.base=base;staged.stack=stack;staged.entry=entry;staged.extent=extent;staged.key=key;staged.core=core;
 crypto_blake2b(staged.digest,32,b,n);*out=staged;return 1;
}
int fb_joint_record(const struct fb_image image[2],const struct fw_peer*p,uint32_t sequence,unsigned char out[128]){
 if(!image||!p||!out||image[0].core!=0||image[1].core!=1||image[0].generation!=image[1].generation||p->generation!=image[0].generation)return 0;
 unsigned char digest[64];uint32_t vectors[2],versions[2];for(unsigned c=0;c<2;++c){for(unsigned i=0;i<32;++i)digest[32*c+i]=image[c].digest[i];vectors[c]=image[c].base;versions[c]=image[c].version;}
 if(FB_MUTANT==3){struct fw_peer forged=*p;forged.health=3;forged.confirmed=1;forged.failed=0;forged.outstanding[0]=forged.outstanding[1]=0;forged.reserved[0]=forged.reserved[1]=0;return fh_record(&forged,sequence,digest,vectors,versions,out);}
 return fh_record(p,sequence,digest,vectors,versions,out);
}
int fb_record_matches(const unsigned char r[128],const struct fb_image image[2]){
 if(!r||!image||!fh_record_valid(r)||image[0].core!=0||image[1].core!=1||image[0].generation!=image[1].generation||get(r+8)!=image[0].generation)return 0;
 for(unsigned c=0;c<2;++c){if(get(r+80+4*c)!=image[c].base||get(r+88+4*c)!=image[c].version)return 0;for(unsigned i=0;i<32;++i)if(r[16+32*c+i]!=image[c].digest[i])return 0;}return 1;
}
