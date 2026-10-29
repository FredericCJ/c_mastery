#ifndef CMASTERY_MPL_MACROS_H
#define CMASTERY_MPL_MACROS_H
#include "logging.h"
/* Compiling unsolved support: replace the aggregate forwarding with the
   required bounded selector and typed 0/1/2/3/4 wrappers. */
#define MPL_LOG(s,...) mpl_write((s),NULL,(struct mpl_field[]){__VA_ARGS__},sizeof((struct mpl_field[]){__VA_ARGS__})/sizeof(struct mpl_field))
#define MPL_LOG0(s) mpl_write((s),NULL,NULL,0)
#define MPL_LOG_AT(s,...) MPL_LOG((s),__VA_ARGS__)
#define MPL_LOG_AT0(s) MPL_LOG0(s)
#endif
