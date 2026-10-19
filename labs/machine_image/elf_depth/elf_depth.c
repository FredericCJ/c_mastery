#include "elf_depth.h"
enum ed_status ed_open(const unsigned char *p,size_t n,struct ed_view *o){(void)p;(void)n;(void)o;return ED_UNSUPPORTED;}
enum ed_status ed_section(const struct ed_view *v,unsigned i,struct ed_section *o){(void)v;(void)i;(void)o;return ED_UNSUPPORTED;}
enum ed_status ed_program(const struct ed_view *v,unsigned i,struct ed_program *o){(void)v;(void)i;(void)o;return ED_UNSUPPORTED;}
enum ed_status ed_symbol(const struct ed_view *v,unsigned s,size_t i,struct ed_symbol *o){(void)v;(void)s;(void)i;(void)o;return ED_UNSUPPORTED;}
enum ed_status ed_name(const struct ed_view *v,unsigned s,uint32_t i,const char **p,size_t *n){(void)v;(void)s;(void)i;(void)p;(void)n;return ED_UNSUPPORTED;}
