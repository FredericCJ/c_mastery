#ifndef SERVICE_DEPTH_H
#define SERVICE_DEPTH_H
#include <stddef.h>
#include <stdint.h>

enum
{
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

struct sd_memory
{
    unsigned refs[16];
    unsigned char bytes[16][64];
};

struct sd_image
{
    unsigned active, count, length, frames[4], cow[4], kind[4];
};

int sd_load(struct sd_memory *, const unsigned char *, size_t, int fail_after,
            struct sd_image *);
int sd_clone(struct sd_memory *, struct sd_image *, unsigned live_threads,
             struct sd_image *);
int sd_write(struct sd_memory *, struct sd_image *, unsigned offset,
             unsigned char, int allocation_allowed);
int sd_release(struct sd_memory *, struct sd_image *);

struct sd_cap
{
    uint32_t epoch;
    unsigned rights;
};

struct sd_message
{
    unsigned length;
    unsigned char bytes[32];
};

struct sd_channel
{
    uint32_t epoch;
    unsigned closed, head, count, retired;
    struct sd_message slots[2];
};

typedef int (*sd_validate_fn)(
    void *, void *, size_t); /* 0 grants complete extent; no writes/reentry. */
int sd_channel_init(struct sd_channel *);
int sd_grant(const struct sd_channel *, struct sd_cap, unsigned rights,
             struct sd_cap *);
int sd_send(struct sd_channel *, struct sd_cap, const unsigned char *, size_t);
int sd_receive(struct sd_channel *, struct sd_cap, void *, size_t, size_t *,
               sd_validate_fn, void *);
int sd_channel_close(struct sd_channel *, struct sd_cap);
int sd_channel_reopen(struct sd_channel *);

struct sd_entry
{
    const char *name;
    const unsigned char *data;
    unsigned length;
};

struct sd_handle
{
    uint32_t index, generation;
};

struct sd_open
{
    unsigned active, entry, offset, refs;
};

struct sd_file_handle
{
    unsigned active, open;
    uint32_t generation;
};

struct sd_files
{
    struct sd_entry entries[4];
    unsigned count;
    struct sd_open descriptions[4];
    struct sd_file_handle handles[8];
};

int sd_files_init(struct sd_files *, const struct sd_entry *, unsigned);
int sd_open(struct sd_files *, const char *, struct sd_handle *);
int sd_dup(struct sd_files *, struct sd_handle, struct sd_handle *);
int sd_seek(struct sd_files *, struct sd_handle, unsigned origin,
            int32_t displacement);
int sd_read(struct sd_files *, struct sd_handle, void *, size_t capacity,
            size_t requested, size_t *, sd_validate_fn, void *);
int sd_close(struct sd_files *, struct sd_handle);

struct sd_journal
{
    unsigned char durable[2][32], pending[2][32];
};

struct sd_receipt
{
    uint32_t last;
    int32_t delta, value;
};

int sd_recover(const struct sd_journal *, struct sd_receipt *);
int sd_apply(struct sd_journal *, uint32_t operation, int32_t delta,
             unsigned fault_stage, int32_t *);
#endif
