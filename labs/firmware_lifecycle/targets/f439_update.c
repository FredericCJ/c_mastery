#include "f439_update.h"
int f4_record(const struct f4_selection*s,unsigned char b[80]){(void)s;(void)b;return 0;}
int f4_select(eg_read_fn r,void*c,const struct fb_policy*p,const struct fb_layout l[2],struct f4_selection*s){(void)r;(void)c;(void)p;(void)l;(void)s;return 0;}
enum fh_result f4_install_begin(struct f4_writer*w,struct fm_io io,const struct f4_selection*o,const struct rt_frame*f,const struct fb_policy*p,const struct fb_layout l[2]){(void)w;(void)io;(void)o;(void)f;(void)p;(void)l;return FH_REJECT;}
int f4_install_result(const struct f4_writer*w,eg_read_fn r,void*c,const struct fb_policy*p,const struct fb_layout l[2],struct fb_image*out){(void)w;(void)r;(void)c;(void)p;(void)l;(void)out;return 0;}
enum fh_result f4_trial_begin(struct f4_writer*w,struct fm_io io,const struct f4_selection*o,const struct fb_image*i){(void)w;(void)io;(void)o;(void)i;return FH_REJECT;}
int f4_trial_probe(eg_read_fn r,void*c,const struct fb_policy*p,const struct fb_layout l[2],const struct f4_selection*o,struct f4_selection*s){(void)r;(void)c;(void)p;(void)l;(void)o;(void)s;return 0;}
enum fh_result f4_commit_begin(struct f4_writer*w,struct fm_io io,const struct f4_selection*o,const struct f4_selection*t,uint32_t g,uint32_t h){(void)w;(void)io;(void)o;(void)t;(void)g;(void)h;return FH_REJECT;}
enum fh_result f4_poll(struct f4_writer*w){(void)w;return FH_REJECT;}
