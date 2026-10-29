#ifndef CMASTERY_ERRORS_H
#define CMASTERY_ERRORS_H
#include <stddef.h>
#include <stdint.h>
#include <setjmp.h>

typedef enum
{
    ER_OK,
    ER_ABSENT,
    ER_INVALID,
    ER_RANGE,
    ER_AGAIN,
    ER_IO,
    ER_THROWN,
    ER_CONTRACT,
    ER_NOT_IMPLEMENTED
} er_code;

typedef struct
{
    er_code tag;

    union
    {
        uint32_t value;
        size_t offset;
    } as;
} er_result;

typedef struct
{
    size_t next;
    er_code code;
} er_progress;

typedef er_progress (*er_sink)(void *, const unsigned char *, size_t);

typedef struct
{
    int age, quota, denominator;
} er_config;

typedef struct
{
    size_t total, stored;
    int truncated;
} er_validation;

typedef void (*er_cleanup)(void *);

typedef struct
{
    jmp_buf jump;
    er_code thrown;
    int active, cleaning;
    size_t count;

    struct
    {
        er_cleanup fn;
        void *context;
    } cleanup[4];
} er_frame;

typedef enum
{
    ER_CHECKED,
    ER_WRAPPING,
    ER_SATURATING
} er_policy;

typedef enum
{
    ER_OFF,
    ER_OBSERVE,
    ER_ENFORCE
} er_mode;

typedef struct
{
    uint32_t numerator, denominator;
} er_fraction;

er_result er_ok(uint32_t value);
er_result er_error(er_code code, size_t offset);
er_code er_optional_pack(uint32_t value, int present, uint32_t *encoded);
er_result er_optional_unpack(uint32_t encoded);
er_result er_parse_pair(const unsigned char *bytes, size_t length,
                        uint32_t *sum);
er_progress er_transfer(const unsigned char *bytes, size_t length, size_t next,
                        er_sink sink, void *context);
er_result er_map(er_result input, uint32_t (*fn)(uint32_t, void *),
                 void *context);
er_result er_then(er_result input, er_result (*fn)(uint32_t, void *),
                  void *context);
er_validation er_validate(er_config input, er_config *output, unsigned *errors,
                          size_t capacity);
er_code er_defer(er_frame *frame, er_cleanup fn, void *context);
er_code er_raise(er_frame *frame, er_code code);
er_result er_run(er_frame *frame, er_result (*body)(er_frame *, void *),
                 void *context);
er_code er_add16(uint32_t a, uint32_t b, er_policy policy, uint32_t *output);
er_code er_serial16(uint32_t a, uint32_t b, int *order);
er_code er_contract_counter(er_mode mode, int amount, int *counter,
                            size_t *reports);
er_code er_refine(uint32_t numerator, uint32_t denominator,
                  er_fraction *output);
#endif
