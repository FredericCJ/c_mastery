#include "i2c.h"

/* Intentionally unfinished learner scaffold. */
struct pi_io pi_f439_io(void)
{
    struct pi_io io = {0, 0, 0};
    return io;
}

void pi_f439_pins(void)
{
}

int pi_f439_step(struct pi_i2c *s, uint32_t token, uint32_t now)
{
    (void)s;
    (void)token;
    (void)now;
    return PI_UNIMPLEMENTED;
}

int pi_f439_open(struct pi_i2c *s, const struct pu_clock *clock)
{
    (void)s;
    (void)clock;
    return PI_UNIMPLEMENTED;
}

int pi_f439_begin(struct pi_i2c *s, uint32_t generation, uint32_t token,
                  unsigned address, unsigned command, unsigned count,
                  uint8_t *out, uint32_t now, unsigned budget)
{
    (void)s;
    (void)generation;
    (void)token;
    (void)address;
    (void)command;
    (void)count;
    (void)out;
    (void)now;
    (void)budget;
    return PI_UNIMPLEMENTED;
}

int pi_f439_cancel(struct pi_i2c *s, uint32_t token)
{
    (void)s;
    (void)token;
    return PI_UNIMPLEMENTED;
}

struct pi_lines pi_f439_lines(void)
{
    struct pi_lines io = {0, 0, 0};
    return io;
}

int pi_f439_recovery_prepare(struct pi_i2c *s, unsigned exclusive,
                             unsigned peers)
{
    (void)s;
    (void)exclusive;
    (void)peers;
    return PI_UNIMPLEMENTED;
}
