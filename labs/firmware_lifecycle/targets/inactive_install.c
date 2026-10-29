#include "inactive_install.h"
enum fh_result fu_begin(struct fu_install*f,struct fm_io io,eg_read_fn r,void*c,const struct rt_frame*t,const struct fb_policy*p,const struct bs_layouts*l,const struct bs_selection*o){(void)f;(void)io;(void)r;(void)c;(void)t;(void)p;(void)l;(void)o;return FH_REJECT;}
enum fh_result fu_poll(struct fu_install*f,struct fb_image*out){(void)f;(void)out;return FH_REJECT;}
