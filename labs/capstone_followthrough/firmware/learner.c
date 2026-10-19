#include "firmware.h"
int cf_init(CfFirmware *f){(void)f;return 0;}
CfStatus cf_boot(CfFirmware*f,int(*r)(void*),void*p,unsigned n){(void)f;(void)r;(void)p;(void)n;return CF_INVALID;}
CfStatus cf_acquire(CfFirmware*f,unsigned s,const CPFSample*p){(void)f;(void)s;(void)p;return CF_INVALID;}
CfStatus cf_foreground(CfFirmware*f,int a){(void)f;(void)a;return CF_INVALID;}
CfStatus cf_command(CfFirmware*f,const CPMessage*m,int a){(void)f;(void)m;(void)a;return CF_INVALID;}
CfStatus cf_dma_begin(CfFirmware*f,PdIo io,uint32_t a,uint32_t b,uint32_t t){(void)f;(void)io;(void)a;(void)b;(void)t;return CF_INVALID;}
CfStatus cf_dma_cancel(CfFirmware*f,PdIo io,uint32_t t){(void)f;(void)io;(void)t;return CF_INVALID;}
CfStatus cf_dma_finish(CfFirmware*f,PdIo io,uint32_t t){(void)f;(void)io;(void)t;return CF_INVALID;}
unsigned cf_watchdog(CfFirmware*f,unsigned m){(void)f;(void)m;return 0;}
void cf_fault(CfFirmware*f,uint32_t c){(void)f;(void)c;}
CfStatus cf_recover(CfFirmware*f){(void)f;return CF_INVALID;}
int cf_budget(uint32_t l,uint32_t e,uint32_t*t,uint32_t*r){(void)l;(void)e;(void)t;(void)r;return 0;}
void cf_candidate(CfSlot*s,uint16_t g,unsigned c,unsigned a,unsigned t,unsigned p){(void)s;(void)g;(void)c;(void)a;(void)t;(void)p;}
int cf_select(const CfSlot*a,const CfSlot*b,unsigned*o){(void)a;(void)b;(void)o;return 0;}
void cf_crash(CfCrash*r,uint32_t s,uint32_t c,unsigned cut){(void)r;(void)s;(void)c;(void)cut;}
int cf_crash_select(const CfCrash*a,const CfCrash*b,unsigned*o){(void)a;(void)b;(void)o;return 0;}
int cf_copy_request(const unsigned char*b,size_t n,size_t o,size_t c,unsigned char out[8]){(void)b;(void)n;(void)o;(void)c;(void)out;return 0;}
int cf_elapsed(uint16_t a,uint16_t b,unsigned w,uint32_t*o){(void)a;(void)b;(void)w;(void)o;return 0;}
unsigned cf_status_read(CfRegister*r){(void)r;return 0;}
void cf_status_write(CfRegister*r,unsigned v){(void)r;(void)v;}
int cf_status_ack(CfRegister*r,unsigned m){(void)r;(void)m;return 0;}
