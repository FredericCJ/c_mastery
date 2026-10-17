#ifndef SERVICE_DEPTH_H
#define SERVICE_DEPTH_H
#include <stddef.h>
#include <stdint.h>
enum {
  SD_OK = 0,
  SD_INVALID = 1,
  SD_FULL = 2,
  SD_STALE = 3,
  SD_DENIED = 4,
  SD_CLOSED = 5,
  SD_EMPTY = 6,
  SD_COPY = 7,
  SD_UNCERTAIN = 8,
  SD_CONFLICT = 9,
  SD_CORRUPT = 10,
  SD_UNIMPLEMENTED = 99
};
typedef struct {
  unsigned refs[16];
  unsigned char bytes[16][64];
} SDMemory;
typedef struct {
  unsigned active, count, length, frames[4], cow[4], kind[4];
} SDImage;
int sd_load(SDMemory *, const unsigned char *, size_t, int fail_after,
            SDImage *);
int sd_clone(SDMemory *, SDImage *, unsigned live_threads, SDImage *);
int sd_write(SDMemory *, SDImage *, unsigned offset, unsigned char,
             int allocation_allowed);
int sd_release(SDMemory *, SDImage *);
typedef struct {
  uint32_t epoch;
  unsigned rights;
} SDCap;
typedef struct {
  unsigned length;
  unsigned char bytes[32];
} SDMessage;
typedef struct {
  uint32_t epoch;
  unsigned closed, head, count, retired;
  SDMessage slots[2];
} SDChannel;
typedef int (*SDValidate)(
    void *, void *, size_t); /* 0 grants complete extent; no writes/reentry. */
int sd_channel_init(SDChannel *);
int sd_grant(const SDChannel *, SDCap, unsigned rights, SDCap *);
int sd_send(SDChannel *, SDCap, const unsigned char *, size_t);
int sd_receive(SDChannel *, SDCap, void *, size_t, size_t *, SDValidate,
               void *);
int sd_channel_close(SDChannel *, SDCap);
int sd_channel_reopen(SDChannel *);
typedef struct {
  const char *name;
  const unsigned char *data;
  unsigned length;
} SDEntry;
typedef struct {
  uint32_t index, generation;
} SDHandle;
typedef struct {
  unsigned active, entry, offset, refs;
} SDOpen;
typedef struct {
  unsigned active, open;
  uint32_t generation;
} SDFileHandle;
typedef struct {
  SDEntry entries[4];
  unsigned count;
  SDOpen descriptions[4];
  SDFileHandle handles[8];
} SDFiles;
int sd_files_init(SDFiles *, const SDEntry *, unsigned);
int sd_open(SDFiles *, const char *, SDHandle *);
int sd_dup(SDFiles *, SDHandle, SDHandle *);
int sd_seek(SDFiles *, SDHandle, unsigned origin, int32_t displacement);
int sd_read(SDFiles *, SDHandle, void *, size_t capacity, size_t requested,
            size_t *, SDValidate, void *);
int sd_close(SDFiles *, SDHandle);
typedef struct {
  unsigned char durable[2][32], pending[2][32];
} SDJournal;
typedef struct {
  uint32_t last;
  int32_t delta, value;
} SDReceipt;
int sd_recover(const SDJournal *, SDReceipt *);
int sd_apply(SDJournal *, uint32_t operation, int32_t delta,
             unsigned fault_stage, int32_t *);
#endif
