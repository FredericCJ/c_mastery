#ifndef HF_FLOW_H
#define HF_FLOW_H
#include "com_http.h"
#include <stddef.h>
#include <stdint.h>

struct hf_slot
{
    int fd;
    uint32_t generation;
    unsigned live, pending, retired;
};

struct hf_pool
{
    struct hf_slot slot[4];
};

struct hf_handle
{
    unsigned index;
    uint32_t generation;
};

/* A queued callback pins its slot storage. Closing invalidates dispatch but
   storage cannot be recycled until every successful pin is completed. */
int hf_admit(struct hf_pool *, int fd, unsigned flags, struct hf_handle *);
int hf_pin(struct hf_pool *, struct hf_handle);
int hf_callback(struct hf_pool *, struct hf_handle, int *dispatch_fd);
int hf_close(struct hf_pool *, struct hf_handle, int *close_fd);

struct hf_accept
{
    int (*accept)(void *);
    int (*flags)(void *, int);
    void (*close)(void *, int);
    void *context;
};

/* accept callback: descriptor>=0; -1 interrupted; -2 would-block; -3 fatal.
   At most four attempts. Flags and admission precede publication. */
int hf_accept_batch(struct hf_pool *, struct hf_accept,
                    struct hf_handle handles[4], unsigned *count);
/* Date NULL means unavailable and omits the field. Error representation NULL
   selects reason+LF. Explicit bytes permit a UTF-8 changed representation. */
int hf_response(const struct http_request *, const char *date, int close,
                const void *error, size_t error_length, void *, size_t,
                size_t *);

struct hf_output
{
    unsigned char bytes[1600];
    size_t length, offset;
    uint64_t started;
    unsigned request, interim;
};

struct hf_flow
{
    struct http_parser parser;
    struct hf_output output[2];
    unsigned capacity, head, count, requests, closing, draining, aborted;
    uint64_t started, progress, drain_started;
};

int hf_init(struct hf_flow *, unsigned capacity, const char *, uint64_t);
int hf_feed(struct hf_flow *, const void *, size_t, size_t *, uint64_t,
            const char *, const char *date);
int hf_eof(struct hf_flow *, uint64_t, const char *date);
/* recv result adapter: -1 EINTR/-2 EAGAIN preserve state;0 is realEOF;
   other negative values abort. Positive bytes are separately retained/fed. */
int hf_read_event(struct hf_flow *, int result, uint64_t, const char *date);
int hf_write(struct hf_flow *, http_write_fn, void *, uint64_t);
/* 0 wait;1 begin write shutdown;2 close after drain;3 output abort;
   request expiry enqueues408 and returns0. Monotonic regression aborts. */
int hf_tick(struct hf_flow *, uint64_t, const char *date);
#endif
