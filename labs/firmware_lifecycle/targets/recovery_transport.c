#include "recovery_transport.h"
void rt_reset(struct rt_frame*f){if(f){f->used=0;f->need=12;f->state=0;}}
enum rt_result rt_byte(struct rt_frame*f,unsigned char c){(void)f;(void)c;return RT_BAD;}
int rt_admit(const struct rt_frame*f,const struct fb_policy*p,const struct bs_layouts*l,struct fb_image*out){(void)f;(void)p;(void)l;(void)out;return 0;}
int rt_uart_init(struct fh_io i,unsigned b){(void)i;(void)b;return -1;}
int rt_uart_rx(struct fh_io i,unsigned char*b,uint32_t*s){(void)i;(void)b;(void)s;return -1;}
int rt_uart_tx(struct fh_io i,unsigned char b,unsigned n){(void)i;(void)b;(void)n;return -1;}
int rt_uart_step(struct fh_io i,struct rt_frame*f,uint32_t*s){(void)i;(void)f;(void)s;return -1;}
