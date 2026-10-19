#ifndef B06_H745_FLASH_H
#define B06_H745_FLASH_H
#include <stdint.h>
#include "h745_cache.h"
typedef uint32_t (*FHRead)(void *,uint32_t);
typedef void (*FHWrite)(void *,uint32_t,uint32_t);
typedef void (*FHSync)(void *);
typedef struct {void *ctx;FHRead read;FHWrite write;FHSync sync;} FHIO;
typedef struct {FHIO io;uint32_t address,control,expected[8];unsigned active;} FHSession;
enum FHResult {FH_DONE,FH_PENDING,FH_REJECT,FH_ERROR};
/* Initialized inactive session; exclusive bank1 ownership; proven virgin word. */
enum FHResult fh_start(FHSession *,FHIO,uint32_t,const uint32_t words[8]);
enum FHResult fh_poll(FHSession *);
FHIO fh_h745_io(void);
/* Output128bytes:96byte body and separately programmed32byte commit word. */
int fh_record(const FWPeer *,uint32_t sequence,const unsigned char hashes[64],
              const uint32_t vectors[2],const uint32_t versions[2],unsigned char out[128]);
int fh_record_valid(const unsigned char in[128]);
typedef struct {FHSession session;FHIO io;uint32_t base;unsigned word,state;unsigned char record[128];} FHAppend;
enum FHResult fh_append_begin(FHAppend *,FHIO,uint32_t,const FWPeer *,uint32_t,
                              const unsigned char[64],const uint32_t[2],const uint32_t[2]);
enum FHResult fh_append_poll(FHAppend *);
#endif
