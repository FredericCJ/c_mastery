#include "flash_maintenance.h"
enum FHResult fm_erase(FMOperation*a,FMIO b,enum FMProfile c,unsigned d){(void)a;(void)b;(void)c;(void)d;return FH_REJECT;}
enum FHResult fm_f439_program(FMOperation*a,FMIO b,uint32_t c,uint32_t d){(void)a;(void)b;(void)c;(void)d;return FH_REJECT;}
enum FHResult fm_poll(FMOperation*a){(void)a;return FH_REJECT;}
enum FHResult fm_recycle_begin(FMRecycle*a,FMIO b,uint32_t c,const FWPeer*d,uint32_t e,const unsigned char f[64],const uint32_t g[2],const uint32_t h[2]){(void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;return FH_REJECT;}
enum FHResult fm_recycle_poll(FMRecycle*a){(void)a;return FH_REJECT;}
int fm_recover(const unsigned char a[128],const unsigned char b[128]){(void)a;(void)b;return 0;}
