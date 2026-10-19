#ifndef B06_TRIAL_INTENT_H
#define B06_TRIAL_INTENT_H
#include "inactive_install.h"
/* Intent128 bytes plus separately programmed attempt32 bytes. Neither makes
 * the image a committed cold-boot choice. Cold reset always uses bs_select. */
int ft_intent(const BSSelection *old,const FBImage installed[2],unsigned char out[128]);
int ft_attempt(const unsigned char intent[128],unsigned char out[32]);
/* Live released peer only: read immutable intent+completed attempt, verify
 * installed bytes/current policy and parent sequence, then publish selection.
 * Call st_peer_matches afterward. Never use this to choose a cold-reset trial. */
int ft_probe(EGRead,void *,const FBPolicy *,const BSLayouts *,const BSSelection *old,BSSelection *);
/* Zero initialized, exclusive sector5/bank1 owner executing from bank2.
 * Installed images are current authenticated results, retained immutable.
 * Only DONE permits a live trial launch; cold reset ignores this record. */
typedef struct {FMIO io;FHSession program;uint32_t control,checked;unsigned state,word;unsigned char bytes[160];} FTWriter;
enum FHResult ft_begin(FTWriter *,FMIO,const BSSelection *,const FBImage[2]);
enum FHResult ft_poll(FTWriter *);
#endif
