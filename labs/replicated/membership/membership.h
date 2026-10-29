#ifndef CMASTERY_REPLICATED_MEMBERSHIP_H
#define CMASTERY_REPLICATED_MEMBERSHIP_H
#include "replicated.h"
#include <stddef.h>
#include <stdint.h>

enum
{
    RPM_OK,
    RPM_INVALID,
    RPM_FULL,
    RPM_RANGE,
    RPM_IDENTITY,
    RPM_AUTH,
    RPM_POLICY,
    RPM_CORRUPT,
    RPM_UNIMPLEMENTED
};

struct rpm_counter
{
    uint64_t uuid, self, ids[6];
    struct rp_pn blocks[2];
    unsigned count;
};

struct rpm_permit
{
    uint64_t uuid, writer;
    unsigned char key[32];
    uint32_t positive_limit, negative_limit;
    unsigned active;
};

int rpm_make(uint64_t uuid, uint64_t self, struct rpm_counter *out);
int rpm_add(const struct rpm_counter *local, int negative, uint32_t amount,
            struct rpm_counter *out);
int rpm_replace(const struct rpm_counter *local, uint64_t fresh,
                struct rpm_counter *out);
int rpm_merge(const struct rpm_counter *local,
              const struct rpm_counter *admitted, struct rpm_counter *out);
int rpm_value(const struct rpm_counter *state, int64_t *out);
int rpm_encode(const struct rpm_counter *state, unsigned char out[160]);
int rpm_decode(const unsigned char wire[160], struct rpm_counter *out);
int rpm_seal(const struct rpm_counter *writer, const unsigned char key[32],
             unsigned char out[64]);
int rpm_admit(const struct rpm_counter *local, const struct rpm_permit *permit,
              const unsigned char wire[64], struct rpm_counter *out);
#endif
