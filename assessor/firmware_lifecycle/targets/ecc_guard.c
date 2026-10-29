#include "ecc_guard.h"
#include "flash_maintenance.h"
#ifndef EG_MUTANT
#define EG_MUTANT 0
#endif
int eg_frame(uint32_t f[8],uint32_t ret,uint32_t status,uint32_t address,uint32_t load,uint32_t abort,uint32_t admitted){
 if(!f||ret!=0xFFFFFFF9u||status!=0x8200u||!(f[7]&0x01000000u)||(f[7]&511u)||(abort&1u)||(load&1u)||(admitted&3u))return 0;
 if(EG_MUTANT!=1&&(f[6]!=load||address!=admitted))return 0;
 f[0]=0;f[6]=abort;return 1;
}
int eg_snapshot(eg_read_fn read,void*ctx,uint32_t base,unsigned char out[128]){
 if(!read||!out||(base&31u)||base>UINT32_MAX-127u)return 0;
 unsigned char bytes[128];for(unsigned i=0;i<32;++i){uint32_t word;if(read(ctx,base+4*i,&word)!=1)return 0;for(unsigned b=0;b<4;++b){bytes[4*i+b]=(unsigned char)(word>>(8*b));if(EG_MUTANT==2)out[4*i+b]=bytes[4*i+b];}}
 for(unsigned i=0;i<128;++i)out[i]=bytes[i];return 1;
}
int eg_scan_pair(eg_read_fn read,void*ctx,uint32_t first,uint32_t second,unsigned char out[128]){
 if(!out||first==second)return 0;unsigned char a[128],b[128];int va=eg_snapshot(read,ctx,first,a),vb=eg_snapshot(read,ctx,second,b);
 if(!va)for(unsigned i=0;i<128;++i)a[i]=0;if(!vb)for(unsigned i=0;i<128;++i)b[i]=0;
 int selected=fm_recover(a,b);if(selected==1||selected==2){const unsigned char*p=selected==1?a:b;for(unsigned i=0;i<128;++i)out[i]=p[i];}return selected;
}
