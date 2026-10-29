#ifndef SERIAL_SYNC_FOLLOWTHROUGH_H
#define SERIAL_SYNC_FOLLOWTHROUGH_H
#include "uart.h"
#include "serial_depth.h"

enum
{
    SU_UNSUPPORTED = 20
};

struct mc_su
{
    struct pu_driver byte;
    struct pu_rx receive;
    uint32_t token, deadline, last;
    unsigned configured, cpol, cpha, lbcl, active, count, position, received,
        byte_received, stopping, result;
    const uint8_t *tx;
    struct pu_byte *rx;
};

/* Clock/gate/reset initialization is the existing pu_open operation. */
int su_configure(struct mc_su *, struct pu_io, unsigned cpol, unsigned cpha,
                 unsigned lbcl);
int su_enable(struct mc_su *, struct pu_io, unsigned transmit,
              unsigned receive);
int su_begin(struct mc_su *, struct pu_io, uint32_t generation, uint32_t token,
             const uint8_t *, unsigned count, struct pu_byte *,
             unsigned capacity, uint32_t now, uint32_t budget);
int su_step(struct mc_su *, struct pu_io, uint32_t token, uint32_t now);
int su_cancel(struct mc_su *, struct pu_io, uint32_t token);
/* Diagnose a synthetic complete trace against an independently declared mode. */
int su_trace(unsigned cpol, unsigned cpha, unsigned lbcl,
             const struct sd_sync *, unsigned *first_bad_edge);
/* F439-only CK pin, PD10/AF7; exclusive port ownership and board ledger required. */
void su_f439_ck(void);
#endif
