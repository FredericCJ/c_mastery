#ifndef CHECKPOINT_WRITER_H
#define CHECKPOINT_WRITER_H
#include <stddef.h>
#include <stdint.h>

enum cpw_kind
{
    CPW_PROGRESS,
    CPW_RETRY,
    CPW_ERROR
};

struct cpw_result
{
    enum cpw_kind kind;
    size_t accepted;
    int cause;
};

enum cpw_status
{
    CPW_PENDING,
    CPW_DONE,
    CPW_FAILED,
    CPW_INVALID
};

struct cpw_error
{
    unsigned operation;
    size_t accepted;
    int primary, cleanup;
};

struct cpw_transfer
{
    const unsigned char *data;
    size_t length, accepted;
    enum cpw_status status;
    struct cpw_error error;
};
typedef struct cpw_result (*cpw_write_fn)(void *, const unsigned char *,
                                          size_t);
struct cpw_result cpw_raw(long);
struct cpw_result cpw_libc(long, int);
struct cpw_result cpw_script(long);
enum cpw_status cpw_begin(struct cpw_transfer *, const unsigned char *, size_t,
                          unsigned);
enum cpw_status cpw_step(struct cpw_transfer *, cpw_write_fn, void *);
enum cpw_status cpw_recover(struct cpw_transfer *);
void cpw_cleanup(struct cpw_transfer *, int);
#endif
