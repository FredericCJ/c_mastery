#include "firmware.h"
int cf_init(struct cf_firmware *f){(void)f;return 0;}
enum cf_status cf_boot(struct cf_firmware*f,int(*r)(void*),void*p,unsigned n){(void)f;(void)r;(void)p;(void)n;return CF_INVALID;}
enum cf_status cf_acquire(struct cf_firmware*f,unsigned s,const struct cpf_sample*p){(void)f;(void)s;(void)p;return CF_INVALID;}
enum cf_status cf_foreground(struct cf_firmware*f,int a){(void)f;(void)a;return CF_INVALID;}
enum cf_status cf_command(struct cf_firmware*f,const struct cp_message*m,int a){(void)f;(void)m;(void)a;return CF_INVALID;}
enum cf_status cf_dma_begin(struct cf_firmware*f,struct pd_io io,uint32_t a,uint32_t b,uint32_t t){(void)f;(void)io;(void)a;(void)b;(void)t;return CF_INVALID;}
enum cf_status cf_dma_cancel(struct cf_firmware*f,struct pd_io io,uint32_t t){(void)f;(void)io;(void)t;return CF_INVALID;}
enum cf_status cf_dma_finish(struct cf_firmware*f,struct pd_io io,uint32_t t){(void)f;(void)io;(void)t;return CF_INVALID;}
unsigned cf_watchdog(struct cf_firmware*f,unsigned m){(void)f;(void)m;return 0;}
void cf_fault(struct cf_firmware*f,uint32_t c){(void)f;(void)c;}
enum cf_status cf_recover(struct cf_firmware*f){(void)f;return CF_INVALID;}
int cf_budget(uint32_t l,uint32_t e,uint32_t*t,uint32_t*r){(void)l;(void)e;(void)t;(void)r;return 0;}
void cf_candidate(struct cf_slot*s,uint16_t g,unsigned c,unsigned a,unsigned t,unsigned p){(void)s;(void)g;(void)c;(void)a;(void)t;(void)p;}
int cf_select(const struct cf_slot*a,const struct cf_slot*b,unsigned*o){(void)a;(void)b;(void)o;return 0;}
void cf_crash(struct cf_crash*r,uint32_t s,uint32_t c,unsigned cut){(void)r;(void)s;(void)c;(void)cut;}
int cf_crash_select(const struct cf_crash*a,const struct cf_crash*b,unsigned*o){(void)a;(void)b;(void)o;return 0;}
int cf_copy_request(const unsigned char*b,size_t n,size_t o,size_t c,unsigned char out[8]){(void)b;(void)n;(void)o;(void)c;(void)out;return 0;}
int cf_elapsed(uint16_t a,uint16_t b,unsigned w,uint32_t*o){(void)a;(void)b;(void)w;(void)o;return 0;}
unsigned cf_status_read(struct cf_register*r){(void)r;return 0;}
void cf_status_write(struct cf_register*r,unsigned v){(void)r;(void)v;}
int cf_status_ack(struct cf_register*r,unsigned m){(void)r;(void)m;return 0;}
