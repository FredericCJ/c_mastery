#ifndef LP_CONTEXT_H
#define LP_CONTEXT_H
#include <stddef.h>
#include <stdint.h>
#include <wchar.h>
enum {LC_OK,LC_INVALID,LC_ENCODING,LC_INCOMPLETE,LC_RANGE,LC_POISONED};
typedef struct {uint32_t scalar,minimum;unsigned remaining;size_t bytes,scalars;} LcUtf;
typedef struct {mbstate_t state;unsigned poisoned;} LcLocale;
/* Live exclusive controls; reset before changing the process locale. */
void lc_utf_init(LcUtf *);
int lc_utf_feed(LcUtf *,const unsigned char *,size_t);
int lc_utf_finish(const LcUtf *,size_t *);
void lc_locale_init(LcLocale *);
int lc_locale_one(LcLocale *,const char *,size_t,wchar_t *,size_t *);
#endif
