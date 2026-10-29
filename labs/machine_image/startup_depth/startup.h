#ifndef CM_STARTUP_DEPTH_H
#define CM_STARTUP_DEPTH_H
#include <stddef.h>
#include <stdint.h>

enum sd_status
{
    SD_OK,
    SD_ARGUMENT,
    SD_RANGE,
    SD_FORMAT,
    SD_UNSUPPORTED
};

/* Borrowed synthetic bytes, not dereferenceable process pointers. Width4/8, LE;
 * <=1MiB,4096 arguments,4096 environment words,256 aux pairs. Publish on success. */
struct sd_stack
{
    uint64_t argc;
    size_t argv, envp, auxv, extent;
    unsigned width;
};

struct sd_mapping
{
    uint64_t label;
    const unsigned char *bytes;
    size_t length;
};
enum sd_status sd_stack_decode(const unsigned char *, size_t, unsigned,
                               struct sd_stack *);
enum sd_status sd_string_lookup(uint64_t, const struct sd_mapping *, size_t,
                                const unsigned char **, size_t *);
/* Decimal spans are ASCII only;1..10 digits unsigned, optional minus for signed.
 * No output publication on invalid input/capacity. Formatting supports explicit
 * spans or NUL strings; caller owns capacity11(unsigned)/12(signed). Input,
 * output and result metadata must be disjoint except where explicitly borrowed. */
enum sd_status sd_parse_u32(const unsigned char *, size_t, uint32_t *);
enum sd_status sd_parse_i32(const unsigned char *, size_t, int32_t *);
enum sd_status sd_format_u32(uint32_t, unsigned char *, size_t, int, size_t *);
enum sd_status sd_format_u32_nodiv(uint32_t, unsigned char *, size_t, int,
                                   size_t *);
enum sd_status sd_format_i32(int32_t, unsigned char *, size_t, int, size_t *);

struct sd_result
{
    int failed;
    uint64_t value;
    unsigned error;
};
struct sd_result sd_linux_result(uint64_t);

enum sd_transfer_status
{
    SD_READY,
    SD_DONE,
    SD_BLOCKED,
    SD_YIELD,
    SD_CANCELLED,
    SD_STALLED,
    SD_EOF,
    SD_IO_ERROR,
    SD_BACKEND_BROKEN
};

struct sd_transfer
{
    unsigned char *buffer;
    size_t length, offset;
    int reading;
    enum sd_transfer_status status;
    unsigned error;
};
typedef struct sd_result (*sd_backend)(void *, unsigned char *, size_t);
/* Caller keeps state and buffer live, disjoint, and exclusively owned while a
 * backend runs. A backend accepts <=requested bytes exactly once, or error with
 * no progress. Only READY/BLOCKED/YIELD resume. Budget bounds calls, including
 * EINTR. Cancellation cannot retract the receiver's accepted prefix. */
enum sd_status sd_transfer_init(struct sd_transfer *, unsigned char *, size_t,
                                int);
enum sd_transfer_status sd_transfer_step(struct sd_transfer *, sd_backend,
                                         void *, size_t, int);
#endif
