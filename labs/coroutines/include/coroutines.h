#ifndef CMASTERY_COROUTINES_H
#define CMASTERY_COROUTINES_H
#include <stddef.h>
#include <stdint.h>

enum
{
    CO_OK = 0,
    CO_INVALID = 1,
    CO_RANGE = 2,
    CO_STATE = 3,
    CO_PENDING = 4,
    CO_IO = 5,
    CO_NO_MATCH = 6,
    CO_DONE = 7,
    CO_REQUEST = 8,
    CO_UNIMPLEMENTED = 9
};

struct co_range
{
    uint32_t current, end, step;
    unsigned done;
};

struct co_flow
{
    struct co_range source;
    size_t remaining, pulls;
};

int co_next(struct co_range *, uint32_t *);
int co_flow_next(struct co_flow *, uint32_t *);
typedef int (*co_fold_fn)(void *,
                          uint32_t); /* 0 continue,1 typed break,other error */
int co_fold(struct co_flow *, co_fold_fn, void *, size_t *accepted);

struct co_row
{
    unsigned key;
    int32_t value;
};

struct co_query
{
    const struct co_row *rows;
    size_t count, index;
    unsigned closed;
};

int co_query_open(struct co_query *, const struct co_row *, size_t);
int co_query_next(struct co_query *, unsigned *key, uint32_t *sum);
int co_query_close(struct co_query *);
typedef void *(*co_alloc_fn)(void *, size_t);
typedef void (*co_free_fn)(void *, void *);

struct co_allocator
{
    void *context;
    co_alloc_fn allocate;
    co_free_fn release;
};

struct co_generator
{
    struct co_allocator allocator;
    uint32_t a, b;
    unsigned index, count;
};

int co_generator_create(struct co_allocator, unsigned, struct co_generator **);
int co_generator_next(struct co_generator *, uint32_t *);
int co_generator_destroy(struct co_generator **);

struct co_trampoline
{
    unsigned n, depth, phase, stack[12];
    uint32_t value;
};

int co_trampoline_init(struct co_trampoline *, unsigned);
int co_trampoline_step(struct co_trampoline *, unsigned budget, uint32_t *);
int co_trampoline_cancel(struct co_trampoline *);

enum
{
    CO_LITERAL = 0,
    CO_SEQUENCE = 1,
    CO_CHOICE = 2,
    CO_MANY = 3,
    CO_EMPTY = 4
};

struct co_parser
{
    unsigned kind, left, right;
    unsigned char literal;
};

int co_parse(const struct co_parser *, size_t, const unsigned char *, size_t,
             size_t, unsigned, size_t *, unsigned *);

struct co_memo
{
    unsigned state;
    size_t end;
};

int co_packrat(const unsigned char *, size_t, struct co_memo *, size_t,
               size_t *, unsigned *);

enum
{
    CO_READ_DATA = 0,
    CO_READ_AGAIN = 1,
    CO_READ_EOF = 2,
    CO_READ_ERROR = 3
};

struct co_read_result
{
    unsigned status;
    size_t count;
};
typedef struct co_read_result (*co_read_fn)(void *, unsigned char *, size_t);
typedef int (*co_arm_fn)(void *, uint32_t);
typedef void (*co_disarm_fn)(void *, uint32_t);

struct co_async_port
{
    void *context;
    co_read_fn read;
    co_arm_fn arm;
    co_disarm_fn disarm;
};

struct co_async
{
    unsigned char data[8];
    size_t length, wanted;
    unsigned state, registered, ready;
    uint32_t generation;
};

int co_async_init(struct co_async *, size_t, uint32_t);
int co_async_poll(struct co_async *, struct co_async_port, unsigned char *,
                  size_t *);
int co_async_notify(struct co_async *, uint32_t);
int co_async_cancel(struct co_async *, struct co_async_port);

struct co_effect
{
    unsigned pc;
    uint32_t generation, first;
};

struct co_continuation
{
    struct co_effect *owner;
    uint32_t generation;
    unsigned question;
};

int co_effect_start(struct co_effect *, struct co_continuation *);
int co_effect_resume(struct co_continuation *, uint32_t,
                     struct co_continuation *, uint32_t *);
int co_effect_abort(struct co_continuation *);
#endif
