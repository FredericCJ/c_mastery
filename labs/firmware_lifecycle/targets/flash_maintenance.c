#include "flash_maintenance.h"
enum fh_result fm_erase(struct fm_operation*a,struct fm_io b,enum fm_profile c,unsigned d){(void)a;(void)b;(void)c;(void)d;return FH_REJECT;}
enum fh_result fm_f439_program(struct fm_operation*a,struct fm_io b,uint32_t c,uint32_t d){(void)a;(void)b;(void)c;(void)d;return FH_REJECT;}
enum fh_result fm_poll(struct fm_operation*a){(void)a;return FH_REJECT;}
enum fh_result fm_recycle_begin(struct fm_recycle*a,struct fm_io b,uint32_t c,const struct fw_peer*d,uint32_t e,const unsigned char f[64],const uint32_t g[2],const uint32_t h[2]){(void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;return FH_REJECT;}
enum fh_result fm_recycle_poll(struct fm_recycle*a){(void)a;return FH_REJECT;}
int fm_recover(const unsigned char a[128],const unsigned char b[128]){(void)a;(void)b;return 0;}
