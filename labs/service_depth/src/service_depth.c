#include "service_depth.h"

/* Learner scaffold: every function compiles, ignores its inputs and returns
 * SD_UNIMPLEMENTED, so the supplied tests fail until you implement the
 * contracts in service_depth.h and CONTRACTS.md. */

int sd_load(struct sd_memory *memory, const unsigned char *data, size_t length,
            int fail_after, struct sd_image *out)
{
    (void)memory;
    (void)data;
    (void)length;
    (void)fail_after;
    (void)out;
    return SD_UNIMPLEMENTED;
}

int sd_clone(struct sd_memory *memory, struct sd_image *parent,
             unsigned live_threads, struct sd_image *out)
{
    (void)memory;
    (void)parent;
    (void)live_threads;
    (void)out;
    return SD_UNIMPLEMENTED;
}

int sd_write(struct sd_memory *memory, struct sd_image *image, unsigned offset,
             unsigned char value, int allocation_allowed)
{
    (void)memory;
    (void)image;
    (void)offset;
    (void)value;
    (void)allocation_allowed;
    return SD_UNIMPLEMENTED;
}

int sd_release(struct sd_memory *memory, struct sd_image *image)
{
    (void)memory;
    (void)image;
    return SD_UNIMPLEMENTED;
}

int sd_channel_init(struct sd_channel *channel)
{
    (void)channel;
    return SD_UNIMPLEMENTED;
}

int sd_grant(const struct sd_channel *channel, struct sd_cap cap,
             unsigned rights, struct sd_cap *out)
{
    (void)channel;
    (void)cap;
    (void)rights;
    (void)out;
    return SD_UNIMPLEMENTED;
}

int sd_send(struct sd_channel *channel, struct sd_cap cap,
            const unsigned char *data, size_t length)
{
    (void)channel;
    (void)cap;
    (void)data;
    (void)length;
    return SD_UNIMPLEMENTED;
}

int sd_receive(struct sd_channel *channel, struct sd_cap cap, void *out,
               size_t capacity, size_t *received, sd_validate_fn check,
               void *context)
{
    (void)channel;
    (void)cap;
    (void)out;
    (void)capacity;
    (void)received;
    (void)check;
    (void)context;
    return SD_UNIMPLEMENTED;
}

int sd_channel_close(struct sd_channel *channel, struct sd_cap cap)
{
    (void)channel;
    (void)cap;
    return SD_UNIMPLEMENTED;
}

int sd_channel_reopen(struct sd_channel *channel)
{
    (void)channel;
    return SD_UNIMPLEMENTED;
}

int sd_files_init(struct sd_files *files, const struct sd_entry *entries,
                  unsigned count)
{
    (void)files;
    (void)entries;
    (void)count;
    return SD_UNIMPLEMENTED;
}

int sd_open(struct sd_files *files, const char *name, struct sd_handle *out)
{
    (void)files;
    (void)name;
    (void)out;
    return SD_UNIMPLEMENTED;
}

int sd_dup(struct sd_files *files, struct sd_handle handle,
           struct sd_handle *out)
{
    (void)files;
    (void)handle;
    (void)out;
    return SD_UNIMPLEMENTED;
}

int sd_seek(struct sd_files *files, struct sd_handle handle, unsigned origin,
            int32_t displacement)
{
    (void)files;
    (void)handle;
    (void)origin;
    (void)displacement;
    return SD_UNIMPLEMENTED;
}

int sd_read(struct sd_files *files, struct sd_handle handle, void *out,
            size_t capacity, size_t requested, size_t *received,
            sd_validate_fn check, void *context)
{
    (void)files;
    (void)handle;
    (void)out;
    (void)capacity;
    (void)requested;
    (void)received;
    (void)check;
    (void)context;
    return SD_UNIMPLEMENTED;
}

int sd_close(struct sd_files *files, struct sd_handle handle)
{
    (void)files;
    (void)handle;
    return SD_UNIMPLEMENTED;
}

int sd_recover(const struct sd_journal *journal, struct sd_receipt *out)
{
    (void)journal;
    (void)out;
    return SD_UNIMPLEMENTED;
}

int sd_apply(struct sd_journal *journal, uint32_t operation, int32_t delta,
             unsigned fault_stage, int32_t *out)
{
    (void)journal;
    (void)operation;
    (void)delta;
    (void)fault_stage;
    (void)out;
    return SD_UNIMPLEMENTED;
}
