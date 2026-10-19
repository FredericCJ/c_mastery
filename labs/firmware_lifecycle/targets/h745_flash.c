#include "h745_flash.h"
enum FHResult fh_start(FHSession *s,FHIO io,uint32_t a,const uint32_t w[8])
{(void)s;(void)io;(void)a;(void)w;return FH_REJECT;}
enum FHResult fh_poll(FHSession *s) {(void)s;return FH_REJECT;}
int fh_record(const FWPeer *p,uint32_t n,const unsigned char h[64],const uint32_t v[2],const uint32_t ver[2],unsigned char out[128])
{(void)p;(void)n;(void)h;(void)v;(void)ver;(void)out;return 0;}
int fh_record_valid(const unsigned char in[128]) {(void)in;return 0;}
enum FHResult fh_append_begin(FHAppend *a,FHIO io,uint32_t base,const FWPeer *p,uint32_t n,const unsigned char h[64],const uint32_t v[2],const uint32_t ver[2])
{(void)a;(void)io;(void)base;(void)p;(void)n;(void)h;(void)v;(void)ver;return FH_REJECT;}
enum FHResult fh_append_poll(FHAppend *a) {(void)a;return FH_REJECT;}
