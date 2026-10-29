#include "membership.h"

int rpm_make(uint64_t uuid, uint64_t self, struct rpm_counter *out)
{
    (void)uuid;
    (void)self;
    (void)out;
    return RPM_UNIMPLEMENTED;
}

int rpm_add(const struct rpm_counter *local, int negative, uint32_t amount,
            struct rpm_counter *out)
{
    (void)local;
    (void)negative;
    (void)amount;
    (void)out;
    return RPM_UNIMPLEMENTED;
}

int rpm_replace(const struct rpm_counter *local, uint64_t fresh,
                struct rpm_counter *out)
{
    (void)local;
    (void)fresh;
    (void)out;
    return RPM_UNIMPLEMENTED;
}

int rpm_merge(const struct rpm_counter *local,
              const struct rpm_counter *admitted, struct rpm_counter *out)
{
    (void)local;
    (void)admitted;
    (void)out;
    return RPM_UNIMPLEMENTED;
}

int rpm_value(const struct rpm_counter *state, int64_t *out)
{
    (void)state;
    (void)out;
    return RPM_UNIMPLEMENTED;
}

int rpm_encode(const struct rpm_counter *state, unsigned char out[160])
{
    (void)state;
    (void)out;
    return RPM_UNIMPLEMENTED;
}

int rpm_decode(const unsigned char wire[160], struct rpm_counter *out)
{
    (void)wire;
    (void)out;
    return RPM_UNIMPLEMENTED;
}

int rpm_seal(const struct rpm_counter *writer, const unsigned char key[32],
             unsigned char out[64])
{
    (void)writer;
    (void)key;
    (void)out;
    return RPM_UNIMPLEMENTED;
}

int rpm_admit(const struct rpm_counter *local, const struct rpm_permit *permit,
              const unsigned char wire[64], struct rpm_counter *out)
{
    (void)local;
    (void)permit;
    (void)wire;
    (void)out;
    return RPM_UNIMPLEMENTED;
}
