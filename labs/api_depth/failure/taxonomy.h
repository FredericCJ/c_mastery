#ifndef CMASTERY_FAILURE_TAXONOMY_H
#define CMASTERY_FAILURE_TAXONOMY_H
#include "generic.h"
#include "public_error.h"

enum ae_outcome
{
    AE_RECOVERABLE,
    AE_RETRYABLE,
    AE_TERMINAL,
    AE_DEGRADED
};

struct ae_secondary
{
    unsigned stage;
    int cause;
};

struct ae_error
{
    struct ae_public result;
    unsigned stage, backend;
    int first;
    struct ae_secondary secondary[2];
    unsigned stored, dropped, saturated;
};
enum ae_code ae_map(unsigned, int);
enum ae_outcome ae_outcome(enum ae_code);
enum ae_code ae_first(struct ae_error *, enum ae_code, unsigned operation,
                      unsigned stage, unsigned backend, int cause);
enum ae_code ae_secondary(struct ae_error *, unsigned stage, int cause);
enum ae_code ae_boundary(const unsigned char *, size_t, unsigned internal_count,
                         unsigned allocation_failed, unsigned device_failed,
                         unsigned *out);

struct ae_digit
{
    unsigned value, admitted;
};
enum ae_code ae_admit_digit(const unsigned char *, size_t, struct ae_digit *);
enum ae_code ae_trusted_digit(struct ae_digit, unsigned *);

struct ae_entry
{
    unsigned id, delta;
};

struct ae_counter
{
    struct ae_entry history[2];
    unsigned count, next, value, effects;
};

enum ae_transport_mode
{
    AE_SEND,
    AE_BUSY,
    AE_FAIL_BEFORE,
    AE_LOSE_ACK
};
enum ae_code ae_submit(struct ae_counter *, unsigned id, unsigned delta,
                       enum ae_transport_mode);

struct ae_write_result
{
    enum ae_code code;
    size_t accepted;
};
enum ae_code ae_write_result(size_t offered, size_t accepted, enum ae_code,
                             struct ae_write_result *);

struct ae_lookup
{
    unsigned found, value;
};
struct ae_lookup ae_lookup(unsigned key);

struct ae_field_error
{
    unsigned field;
    size_t offset;
    int cause;
};

struct ae_fields
{
    unsigned count;
    int values[4];
    struct ae_field_error errors[4];
};
enum ae_code ae_fields(const struct av_text *, size_t, struct ae_fields *);
#endif
