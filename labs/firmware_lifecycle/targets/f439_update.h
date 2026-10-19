#ifndef B06_F439_UPDATE_H
#define B06_F439_UPDATE_H
#include "recovery_transport.h"
typedef struct {uint32_t sequence;unsigned record;FBImage image;} F4Selection;
typedef struct {FMIO io;uint32_t base,sector_base,control,checked;unsigned length,word,state,mode;unsigned char bytes[2144];} F4Writer;
/* Selected two-MiB unswapped F439, x32 at qualified2.7..3.6V, bank2 code.
 * All buffers distinct, callbacks live/nonreentrant, exclusive flash ownership.
 * Cold selector never consults trial sector7. No invented second core. */
int f4_record(const F4Selection *,unsigned char out[80]);
int f4_select(EGRead,void *,const FBPolicy *,const FBLayout choices[2],F4Selection *);
enum FHResult f4_install_begin(F4Writer *,FMIO,const F4Selection *,const RTFrame *,const FBPolicy *,const FBLayout choices[2]);
int f4_install_result(const F4Writer *,EGRead,void *,const FBPolicy *,const FBLayout choices[2],FBImage *);
enum FHResult f4_trial_begin(F4Writer *,FMIO,const F4Selection *,const FBImage *);
int f4_trial_probe(EGRead,void *,const FBPolicy *,const FBLayout choices[2],const F4Selection *,F4Selection *);
enum FHResult f4_commit_begin(F4Writer *,FMIO,const F4Selection *,const F4Selection *,uint32_t application_generation,uint32_t application_result);
enum FHResult f4_poll(F4Writer *);
#endif
