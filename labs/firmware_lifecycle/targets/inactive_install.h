#ifndef B06_INACTIVE_INSTALL_H
#define B06_INACTIVE_INSTALL_H
#include "recovery_transport.h"
typedef struct {FMIO io;EGRead read;void*read_ctx;FBPolicy policy;FBLayout layout;FBImage expected;FHSession program;uint32_t sector_base,control,checked;unsigned state,word;unsigned char bytes[2144];} FUInstall;
/* Fresh all-zero control object; current-policy/old selection trusted and owned.
 * Only inactive H745 bank1 sectors1..4, never bootstrap/log/active slot. */
enum FHResult fu_begin(FUInstall *,FMIO,EGRead,void *,const RTFrame *,const FBPolicy *,const BSLayouts *,const BSSelection *old);
/* At most32 erase-verification words per poll; one program-word transition.
 * Output unchanged until successful fresh signed storage readback. FH_DONE
 * means inactive image installed, not trial-health or committed activation. */
enum FHResult fu_poll(FUInstall *,FBImage *);
#endif
