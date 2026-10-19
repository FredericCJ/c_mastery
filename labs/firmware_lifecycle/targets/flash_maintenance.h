#ifndef B06_FLASH_MAINTENANCE_H
#define B06_FLASH_MAINTENANCE_H
#include "h745_flash.h"
/* Exact selected profiles: F439 2MiB unswapped, H745xI 2MiB unswapped.
 * Fresh must make subsequent reads observe flash, not pre-operation cache.
 * It must not fail, reenter, or access a bank while an operation is pending. */
typedef struct {FHIO bus;void (*fresh)(void *,uint32_t,uint32_t);} FMIO;
enum FMProfile {FM_F439=4,FM_H745=7};
typedef struct {FMIO io;uint32_t address,span,checked,control,value;unsigned profile,kind,state;} FMOperation;
enum FHResult fm_erase(FMOperation *,FMIO,enum FMProfile,unsigned sector);
enum FHResult fm_f439_program(FMOperation *,FMIO,uint32_t address,uint32_t value);
enum FHResult fm_poll(FMOperation *);
FMIO fm_f439_io(void);
FMIO fm_h745_io(void);
/* Copy validated new joint record to inactive sector6/7, validate it, then erase
 * old sector. Caller has authenticated images and owns both sectors/peers. */
typedef struct {FMOperation erase;FHAppend append;FMIO io;uint32_t old_base,new_base;unsigned state;unsigned char record[128];} FMRecycle;
enum FHResult fm_recycle_begin(FMRecycle *,FMIO,uint32_t old_base,const FWPeer *,uint32_t sequence,
                              const unsigned char[64],const uint32_t[2],const uint32_t[2]);
enum FHResult fm_recycle_poll(FMRecycle *);
/* Readable, fault-contained snapshots only. Highest sequence wins; unequal
 * records with equal sequences are corruption, never an arbitrary tie-break. */
int fm_recover(const unsigned char a[128],const unsigned char b[128]);
#endif
