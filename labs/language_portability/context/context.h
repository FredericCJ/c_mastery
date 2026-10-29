#ifndef LP_CONTEXT_H
#define LP_CONTEXT_H
#include <stddef.h>
#include <stdint.h>
#include <wchar.h>
enum {LC_OK,LC_INVALID,LC_ENCODING,LC_INCOMPLETE,LC_RANGE,LC_POISONED};
struct lc_utf {uint32_t scalar,minimum;unsigned remaining;size_t bytes,scalars;};
struct lc_locale {mbstate_t state;unsigned poisoned;};
/* Live exclusive controls; reset before changing the process locale. */
void lc_utf_init(struct lc_utf *);
int lc_utf_feed(struct lc_utf *,const unsigned char *,size_t);
int lc_utf_finish(const struct lc_utf *,size_t *);
void lc_locale_init(struct lc_locale *);
int lc_locale_one(struct lc_locale *,const char *,size_t,wchar_t *,size_t *);
#endif
