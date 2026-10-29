#ifndef B06_H745_FLASH_H
#define B06_H745_FLASH_H
#include <stdint.h>
#include "h745_cache.h"
typedef uint32_t (*fh_read_fn)(void *,uint32_t);
typedef void (*fh_write_fn)(void *,uint32_t,uint32_t);
typedef void (*fh_sync_fn)(void *);
struct fh_io {void *ctx;fh_read_fn read;fh_write_fn write;fh_sync_fn sync;};
struct fh_session {struct fh_io io;uint32_t address,control,expected[8];unsigned active;};
enum fh_result {FH_DONE,FH_PENDING,FH_REJECT,FH_ERROR};
/* Initialized inactive session; exclusive bank1 ownership; proven virgin word. */
enum fh_result fh_start(struct fh_session *,struct fh_io,uint32_t,const uint32_t words[8]);
enum fh_result fh_poll(struct fh_session *);
struct fh_io fh_h745_io(void);
/* Output128bytes:96byte body and separately programmed32byte commit word. */
int fh_record(const struct fw_peer *,uint32_t sequence,const unsigned char hashes[64],
              const uint32_t vectors[2],const uint32_t versions[2],unsigned char out[128]);
int fh_record_valid(const unsigned char in[128]);
struct fh_append {struct fh_session session;struct fh_io io;uint32_t base;unsigned word,state;unsigned char record[128];};
enum fh_result fh_append_begin(struct fh_append *,struct fh_io,uint32_t,const struct fw_peer *,uint32_t,
                              const unsigned char[64],const uint32_t[2],const uint32_t[2]);
enum fh_result fh_append_poll(struct fh_append *);
#endif
