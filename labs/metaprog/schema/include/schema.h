#ifndef CMASTERY_MPS_SCHEMA_H
#define CMASTERY_MPS_SCHEMA_H
#include <stddef.h>
#include <stdint.h>
#include "schema_fields.h"
#include "schema_stamp.h"

enum
{
    MPS_OK,
    MPS_INVALID,
    MPS_RANGE,
    MPS_FULL,
    MPS_STOP
};

enum
{
    MPS_U16,
    MPS_U32,
    MPS_S32
};

#define MPS_DECLARE(i, name, type, tag, width, required, kind, lo, hi, since) \
    type name;

struct mps_record
{
    uint32_t present;
    MPS_FIELDS(MPS_DECLARE)
};

#undef MPS_DECLARE
#define MPS_COUNT(i, name, type, tag, width, required, kind, lo, hi, since) +1

enum
{
    MPS_FIELD_COUNT = 0 MPS_FIELDS(MPS_COUNT),
    MPS_WIRE_CAPACITY = 64
};

#undef MPS_COUNT

struct mps_field
{
    const char *name;
    size_t offset, native_size;
    unsigned index, tag, width, required, kind, since;
    int64_t minimum, maximum;
};

typedef int (*mps_visitor_fn)(void *, const struct mps_field *, int64_t);
void MPS_LINK_GUARD(void);
size_t mps_fields(const struct mps_field **);
int mps_validate(const struct mps_record *);
int mps_equal(const struct mps_record *, const struct mps_record *, unsigned *);
int mps_encode(unsigned, const struct mps_record *, unsigned char *, size_t,
               size_t *);
int mps_decode(const unsigned char *, size_t, struct mps_record *);
int mps_visit(const struct mps_record *, mps_visitor_fn, void *, size_t *);
#endif
