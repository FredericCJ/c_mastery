#include "context.h"
void lc_utf_init(struct lc_utf*s){*s=(struct lc_utf){0,0,0,0,0};}
int lc_utf_feed(struct lc_utf*s,const unsigned char*p,size_t n){(void)s;(void)p;(void)n;return LC_INVALID;}
int lc_utf_finish(const struct lc_utf*s,size_t*out){(void)s;(void)out;return LC_INVALID;}
void lc_locale_init(struct lc_locale*s){*s=(struct lc_locale){0};}
int lc_locale_one(struct lc_locale*s,const char*p,size_t n,wchar_t*out,size_t*used){(void)s;(void)p;(void)n;(void)out;(void)used;return LC_INVALID;}
