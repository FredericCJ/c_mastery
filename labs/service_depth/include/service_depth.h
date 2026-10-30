#ifndef SERVICE_DEPTH_H
#define SERVICE_DEPTH_H
#include <stddef.h>
#include <stdint.h>

/* Status values. Every call returns one of these. A failure leaves outputs
 * and logical ownership unchanged, except the documented SD_UNCERTAIN journal
 * progress and the terminal channel retirement (see CONTRACTS.md). All calls
 * are serialized by one owner; no function allocates from the host heap. */
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

/* Physical memory model: 16 frames of 64 bytes with a reference count each. */
struct sd_memory
{
    unsigned refs[16];
    unsigned char bytes[16][64];
};

/* One address space: up to 4 distinct frames (length 1..256 bytes). cow and
 * kind are per-page flags; kind 0 is private ordinary memory, anything else
 * (shared, device) cannot be cloned. */
struct sd_image
{
    unsigned active, count, length, frames[4], cow[4], kind[4];
};

/* Copy `length` bytes (1..256) into freshly taken frames and publish the
 * image in `out`. `fail_after` -1 injects nothing; k >= 0 makes allocation
 * number k (zero based) fail. Returns SD_INVALID for bad arguments, SD_FULL on
 * exhaustion or injection: frames taken by this call are released and `out`
 * is untouched. `out` must not own an image already. */
int sd_load(struct sd_memory *memory, const unsigned char *data, size_t length,
            int fail_after, struct sd_image *out);
/* Copy-on-write clone for a process with exactly one live thread. Returns
 * SD_INVALID (thread count or image shape), SD_DENIED (a page with nonzero
 * kind), SD_FULL (invalid frame or saturated reference count); the parent is
 * unchanged on every failure. On success both images are marked COW. */
int sd_clone(struct sd_memory *memory, struct sd_image *parent,
             unsigned live_threads, struct sd_image *out);
/* Store one byte. A shared frame is copied first, which needs
 * `allocation_allowed` (0 or 1): SD_FULL otherwise or when no frame is free.
 * A sole owner writes in place. */
int sd_write(struct sd_memory *memory, struct sd_image *image, unsigned offset,
             unsigned char value, int allocation_allowed);
/* Drop the image's references; a frame whose count reaches zero is cleared.
 * The whole ownership list is validated first (SD_INVALID, nothing released),
 * so a repeated release is rejected. */
int sd_release(struct sd_memory *memory, struct sd_image *image);

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

/* Copy-out check, called as check(context, destination, length) before any
 * state changes. Returning 0 promises the whole extent can be written without
 * fault; it must not write, re-enter the service or keep the pointer. Nonzero
 * makes the operation return SD_COPY with everything unchanged. */
typedef int (*sd_validate_fn)(void *context, void *destination, size_t length);
/* Rights: send 1, receive 2, close 4. */
int sd_channel_init(struct sd_channel *channel);
/* Attenuate: `out` gets a capability for a nonempty subset `rights` of what
 * `cap` already holds. SD_STALE for a retired channel or old epoch (checked
 * before rights), SD_DENIED for a right `cap` lacks, SD_INVALID otherwise. */
int sd_grant(const struct sd_channel *channel, struct sd_cap cap,
             unsigned rights, struct sd_cap *out);
/* Queue a copy of `length` (<= 32) bytes; `data` may be NULL when length is 0.
 * Needs the send right. SD_CLOSED after close, SD_FULL with two queued. */
int sd_send(struct sd_channel *channel, struct sd_cap cap,
            const unsigned char *data, size_t length);
/* Dequeue the oldest message into `out` (`capacity` bytes) and report its size
 * in `received`. Needs the receive right. SD_EMPTY (or SD_CLOSED once closed),
 * SD_FULL for too small a destination, SD_COPY when `check` refuses; in each
 * case the message stays queued and `received` is untouched. */
int sd_receive(struct sd_channel *channel, struct sd_cap cap, void *out,
               size_t capacity, size_t *received, sd_validate_fn check,
               void *context);
/* Needs the close right; idempotent, queued messages stay receivable. */
int sd_channel_close(struct sd_channel *channel, struct sd_cap cap);
/* Trusted lifecycle step for a closed, empty channel: a new epoch makes every
 * old capability stale. At the last epoch value the channel is retired and
 * SD_FULL is returned; later calls return SD_STALE. */
int sd_channel_reopen(struct sd_channel *channel);

/* A read-only file; name and data are borrowed and must outlive the table. */
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

/* Validate and install 1..4 entries; the table is unchanged on SD_INVALID. */
int sd_files_init(struct sd_files *files, const struct sd_entry *entries,
                  unsigned count);
/* New handle with its own description (offset 0). SD_INVALID for an unknown
 * or malformed name, SD_FULL when no handle or description is free. */
int sd_open(struct sd_files *files, const char *name, struct sd_handle *out);
/* New handle sharing the description (and offset) of `handle`. SD_STALE if
 * `handle` is not live, SD_FULL when no handle is free. */
int sd_dup(struct sd_files *files, struct sd_handle handle,
           struct sd_handle *out);
/* origin 0 start, 1 current, 2 end. Offsets outside [0, length] are
 * SD_INVALID and leave the offset alone. */
int sd_seek(struct sd_files *files, struct sd_handle handle, unsigned origin,
            int32_t displacement);
/* Read min(requested, remaining) bytes at the shared offset into `out`
 * (`capacity` bytes, requested <= capacity). `check` runs once, even for zero
 * bytes; if it refuses, SD_COPY is returned and neither the offset nor
 * `received` changes. */
int sd_read(struct sd_files *files, struct sd_handle handle, void *out,
            size_t capacity, size_t requested, size_t *received,
            sd_validate_fn check, void *context);
/* Release one handle; SD_STALE for a closed or never-issued handle. */
int sd_close(struct sd_files *files, struct sd_handle handle);

/* Two alternating 32-byte durable slots plus the volatile buffers that are
 * prepared and then flushed into them. Zero initialization is valid. */
struct sd_journal
{
    unsigned char durable[2][32], pending[2][32];
};

struct sd_receipt
{
    uint32_t last;
    int32_t delta, value;
};

/* Find the latest committed receipt (all zero if none). SD_CORRUPT for a
 * committed slot with a bad magic, zero id, checksum mismatch or duplicate
 * id; `out` is then untouched. */
int sd_recover(const struct sd_journal *journal, struct sd_receipt *out);
/* Apply `delta` exactly once under operation id `operation` (must equal the
 * last id + 1). A retry of the latest id returns its recorded value, or
 * SD_CONFLICT with a different delta; older ids are SD_STALE; overflow of the
 * running value is SD_FULL. `fault_stage` 1..4 injects an interruption after
 * prepare, first flush, commit and second flush: SD_UNCERTAIN with `out`
 * untouched, durable progress possible (only stage 4 commits). */
int sd_apply(struct sd_journal *journal, uint32_t operation, int32_t delta,
             unsigned fault_stage, int32_t *out);
#endif
