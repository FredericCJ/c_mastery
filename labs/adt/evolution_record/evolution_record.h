#ifndef ADT_EVOLUTION_RECORD_H
#define ADT_EVOLUTION_RECORD_H
#include "adt.h"

enum
{
    ADE_OK,
    ADE_INVALID,
    ADE_CAPACITY,
    ADE_AUTH,
    ADE_POLICY,
    ADE_RANGE
};

enum
{
    ADE_FIELDS = 8,
    ADE_PAYLOAD = 16,
    ADE_BYTES = 145
};

struct ade_field
{
    unsigned tag, length;
    unsigned char bytes[ADE_PAYLOAD];
};

struct ade_record
{
    unsigned count;
    struct ade_field fields[ADE_FIELDS];
};

struct ade_service
{
    uint32_t total, privileged;
};

/* Record wire=count:u8 followed by original ADT TLVs. Known tags1/2 are
 * singular; unknown advisory duplicates retain occurrence order. Owned copy. */
int ade_read(const unsigned char *, size_t, struct ade_record *);
int ade_write(const struct ade_record *, int canonical, unsigned char *, size_t,
              size_t *);
/* Canonical transcript includes every field and unknown duplicate. key32 is
 * synthetic teaching material; actual first-principles RphHmac is imported. */
int ade_tag(const struct ade_record *, const unsigned char key[32],
            unsigned char tag[32]);
int ade_check(const struct ade_record *, const unsigned char key[32],
              const unsigned char tag[32]);
/* Synchronous service policy: exactly one tag1 amount; optional tag2;
 * every unknown is refused even when authenticated. State changes last. */
int ade_apply(const struct ade_record *, const unsigned char key[32],
              const unsigned char tag[32], struct ade_service *);
#endif
